/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

// mapnik
#include <mapnik/geom_util.hpp>

// boost
#include <boost/version.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/make_shared.hpp>

// stl
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "shape_datasource.hpp"
#include "shape_featureset.hpp"
#include "shape_index_featureset.hpp"

DATASOURCE_PLUGIN(shape_datasource)

using mapnik::String;
using mapnik::Double;
using mapnik::Integer;
using mapnik::datasource_exception;
using mapnik::filter_in_box;
using mapnik::filter_at_point;
using mapnik::attribute_descriptor;

shape_datasource::shape_datasource(const parameters &params, bool bind)
    : datasource (params),
      type_(datasource::Vector),
      file_length_(0),
      indexed_(false),
      row_limit_(*params_.get<int>("row_limit",0)),
      desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8"))
{
    boost::optional<std::string> file = params.get<std::string>("file");
    if (!file) throw datasource_exception("Shape Plugin: missing <file> parameter");
      
    boost::optional<std::string> base = params.get<std::string>("base");
    if (base)
        shape_name_ = *base + "/" + *file;
    else
        shape_name_ = *file;

    boost::algorithm::ireplace_last(shape_name_,".shp","");
    
    if (bind)
    {
        this->bind();
    }
}

void shape_datasource::bind() const
{
    if (is_bound_) return;
    
    if (!boost::filesystem::exists(shape_name_ + ".shp"))
    {
        throw datasource_exception("Shape Plugin: shapefile '" + shape_name_ + ".shp' does not exist");
    }

    if (boost::filesystem::is_directory(shape_name_ + ".shp"))
    {
        throw datasource_exception("Shape Plugin: shapefile '" + shape_name_ + ".shp' appears to be a directory not a file");
    }

    if (!boost::filesystem::exists(shape_name_ + ".dbf"))
    {
        throw datasource_exception("Shape Plugin: shapefile '" + shape_name_ + ".dbf' does not exist");
    }


    try
    {  
        boost::shared_ptr<shape_io> shape_ref = boost::make_shared<shape_io>(shape_name_);
        init(*shape_ref);
        for (int i=0;i<shape_ref->dbf().num_fields();++i)
        {
            field_descriptor const& fd=shape_ref->dbf().descriptor(i);
            std::string fld_name=fd.name_;
            switch (fd.type_)
            {
            case 'C': // character
            case 'D': // Date
            case 'M': // Memo, a string
            case 'L': // logical
            case '@': // timestamp
                desc_.add_descriptor(attribute_descriptor(fld_name, String));
                break;
            case 'N':
            case 'O': // double
            case 'F': // float
            {
                if (fd.dec_>0)
                {   
                    desc_.add_descriptor(attribute_descriptor(fld_name,Double,false,8));
                }
                else
                {
                    desc_.add_descriptor(attribute_descriptor(fld_name,Integer,false,4));
                }
                break;
            }
            default:
#ifdef MAPNIK_DEBUG
                // I - long
                // G - ole
                // + - autoincrement
                std::clog << "Shape Plugin: unknown type " << fd.type_ << std::endl;
#endif 
                break;
            }
        }
        // for indexed shapefiles we keep open the file descriptor for fast reads
        if (indexed_) {
            shape_ = shape_ref;
        }

    }
    catch (const datasource_exception& ex)
    {
        std::clog << "Shape Plugin: error processing field attributes, " << ex.what() << std::endl;
        throw;
    }
    catch (const std::exception& ex)
    {
        std::clog << "Shape Plugin: error processing field attributes, " << ex.what() << std::endl;
        throw;
    }
    catch (...) // exception: pipe_select_interrupter: Too many open files
    {
        std::clog << "Shape Plugin: error processing field attributes" << std::endl;
        throw;
    }
    
    is_bound_ = true;
}

shape_datasource::~shape_datasource() {}

void  shape_datasource::init(shape_io& shape) const
{
    //first read header from *.shp
    int file_code=shape.shp().read_xdr_integer();
    if (file_code!=9994)
    {
        //invalid file code
        throw datasource_exception("Shape Plugin: " + (boost::format("wrong file code : %d") % file_code).str());
    }
    
    shape.shp().skip(5*4);
    file_length_=shape.shp().read_xdr_integer();
    int version=shape.shp().read_ndr_integer();
   
    if (version!=1000)
    {
        //invalid version number
        throw datasource_exception("Shape Plugin: " + (boost::format("invalid version number: %d") % version).str());
    }
   
    int shape_type = shape.shp().read_ndr_integer();
    if (shape_type == shape_io::shape_multipatch)
        throw datasource_exception("Shape Plugin: shapefile multipatch type is not supported");
   
    shape.shp().read_envelope(extent_);
   
#ifdef MAPNIK_DEBUG
    double zmin = shape.shp().read_double();
    double zmax = shape.shp().read_double();
    double mmin = shape.shp().read_double();
    double mmax = shape.shp().read_double();

    std::clog << "Shape Plugin: Z min/max " << zmin << "," << zmax << std::endl;
    std::clog << "Shape Plugin: M min/max " << mmin << "," << mmax << "\n";
#else
    shape.shp().skip(4*8);
#endif

    // check if we have an index file around
    
    indexed_ = shape.has_index();
    
    //std::string index_name(shape_name_+".index");
    //std::ifstream file(index_name.c_str(),std::ios::in | std::ios::binary);
    //if (file)
    //{
    //    indexed_=true;
    //    file.close();
    //}
    //else
    //{
    //    std::clog << "### Notice: no .index file found for " + shape_name_ + ".shp, use the 'shapeindex' program to build an index for faster rendering\n";
    //}
    
#ifdef MAPNIK_DEBUG
    std::clog << "Shape Plugin: extent=" << extent_ << std::endl;
    std::clog << "Shape Plugin: file_length=" << file_length_ << std::endl;
    std::clog << "Shape Plugin: shape_type=" << shape_type << std::endl;
#endif

}

std::string shape_datasource::name()
{
    return "shape";
}

int shape_datasource::type() const
{
    return type_;
}

layer_descriptor shape_datasource::get_descriptor() const
{
    if (!is_bound_) bind();
    return desc_;
}

featureset_ptr shape_datasource::features(const query& q) const
{
    if (!is_bound_) bind();
    
    filter_in_box filter(q.get_bbox());
    if (indexed_)
    {
        shape_->shp().seek(0);
        // TODO - use boost::make_shared - #760
        return featureset_ptr
            (new shape_index_featureset<filter_in_box>(filter,
                                                       *shape_,
                                                       q.property_names(),
                                                       desc_.get_encoding(),
                                                       shape_name_,
                                                       row_limit_));
    }
    else
    {
        return boost::make_shared<shape_featureset<filter_in_box> >(filter,
                                                 shape_name_,
                                                 q.property_names(),
                                                 desc_.get_encoding(),
                                                 file_length_,
                                                 row_limit_);
    }
}

featureset_ptr shape_datasource::features_at_point(coord2d const& pt) const
{
    if (!is_bound_) bind();

    filter_at_point filter(pt);
    // collect all attribute names
    std::vector<attribute_descriptor> const& desc_vector = desc_.get_descriptors();
    std::vector<attribute_descriptor>::const_iterator itr = desc_vector.begin();
    std::vector<attribute_descriptor>::const_iterator end = desc_vector.end();
    std::set<std::string> names;
    
    while (itr != end)
    {    
        names.insert(itr->get_name());
        ++itr;
    }
    
    if (indexed_)
    {
        shape_->shp().seek(0);
        // TODO - use boost::make_shared - #760
        return featureset_ptr
            (new shape_index_featureset<filter_at_point>(filter,
                                                         *shape_,
                                                         names,
                                                         desc_.get_encoding(),
                                                         shape_name_,
                                                         row_limit_));
    }
    else
    {
        return boost::make_shared<shape_featureset<filter_at_point> >(filter,
                                                   shape_name_,
                                                   names,
                                                   desc_.get_encoding(),
                                                   file_length_,
                                                   row_limit_);
    }
}

box2d<double> shape_datasource::envelope() const
{
    if (!is_bound_) bind();
    
    return extent_;
}
