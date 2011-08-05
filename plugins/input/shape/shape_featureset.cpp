
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
#include <mapnik/feature_factory.hpp>

// boost
#include <boost/algorithm/string.hpp>

// stl
#include <iostream>

#include "shape_featureset.hpp"

using mapnik::geometry_type;
using mapnik::feature_factory;

template <typename filterT>
shape_featureset<filterT>::shape_featureset(const filterT& filter, 
                                            const std::string& shape_name,
                                            const std::set<std::string>& attribute_names,
                                            std::string const& encoding,
                                            long file_length )
    : filter_(filter),
      //shape_type_(shape_io::shape_null),
      shape_(shape_name, false),
      query_ext_(),
      tr_(new transcoder(encoding)),
      file_length_(file_length),
      count_(0)
{
    shape_.shp().skip(100);
    //attributes
    typename std::set<std::string>::const_iterator pos=attribute_names.begin();
    while (pos!=attribute_names.end())
    {
        bool found_name = false;
        for (int i=0;i<shape_.dbf().num_fields();++i)
        {
            if (shape_.dbf().descriptor(i).name_ == *pos)
            {
                attr_ids_.push_back(i);
                found_name = true;
                break;
            }
        }
        if (!found_name)
        {
            std::ostringstream s;

            s << "no attribute '" << *pos << "' in '"
              << shape_name << "'. Valid attributes are: ";
            std::vector<std::string> list;
            for (int i=0;i<shape_.dbf().num_fields();++i)
            {
                list.push_back(shape_.dbf().descriptor(i).name_);
            }
            s << boost::algorithm::join(list, ",") << ".";
            
            throw mapnik::datasource_exception( "Shape Plugin: " + s.str() );
        }
        ++pos;
    }
}


template <typename filterT>
feature_ptr shape_featureset<filterT>::next()
{
    std::streampos pos=shape_.shp().pos();
    
    if (pos < std::streampos(file_length_ * 2))
    {
        shape_.move_to(pos);
        int type=shape_.type();
        feature_ptr feature(feature_factory::create(shape_.id_));
        if (type == shape_io::shape_point)
        {
            double x=shape_.shp().read_double();
            double y=shape_.shp().read_double();
            geometry_type * point = new geometry_type(mapnik::Point);
            point->move_to(x,y);
            feature->add_geometry(point);
            ++count_;
        }
        else if (type == shape_io::shape_pointm)
        {
            double x=shape_.shp().read_double();
            double y=shape_.shp().read_double();
            shape_.shp().skip(8); //m
            geometry_type * point = new geometry_type(mapnik::Point);
            point->move_to(x,y);
            feature->add_geometry(point);
            ++count_;
        }
        else if (type == shape_io::shape_pointz)
        {
            double x=shape_.shp().read_double();
            double y=shape_.shp().read_double();
            // skip z
            shape_.shp().skip(8);

            //skip m if exists
            if ( shape_.reclength_ == 8 + 36) 
            {
                shape_.shp().skip(8);
            }
            geometry_type * point = new geometry_type(mapnik::Point);
            point->move_to(x,y);
            feature->add_geometry(point);
            ++count_;
        }
        else
        {
            while (!filter_.pass(shape_.current_extent()))
            {        
                if (!shape_.shp().is_eof())
                {
                    std::streampos pos = shape_.shp().pos();
                    if (shape_.type() != shape_io::shape_null)
                    {
                       pos += std::streampos(2 * shape_.reclength_ - 36);
                    }
                    shape_.move_to(pos);                    
                }
                else
                {
                    return feature_ptr();
                }
            }
            
            switch (type)
            {
                case shape_io::shape_multipoint:
                {
                    int num_points = shape_.shp().read_ndr_integer();
                    for (int i=0; i< num_points;++i)
                    { 
                        double x=shape_.shp().read_double();
                        double y=shape_.shp().read_double();
                        geometry_type * point = new geometry_type(mapnik::Point);
                        point->move_to(x,y);
                        feature->add_geometry(point);
                    }
                    ++count_;
                    break;
                }
                case shape_io::shape_multipointm:
                {
                    int num_points = shape_.shp().read_ndr_integer();
                    for (int i=0; i< num_points;++i)
                    { 
                        double x=shape_.shp().read_double();
                        double y=shape_.shp().read_double();
                        geometry_type * point = new geometry_type(mapnik::Point);
                        point->move_to(x,y);
                        feature->add_geometry(point);
                    }
        
                    // skip m 
                    shape_.shp().skip(2*8 + 8*num_points);
                    ++count_;
                    break;
                }
                case shape_io::shape_multipointz:
                {
                    unsigned num_points = shape_.shp().read_ndr_integer();
                    for (unsigned i=0; i< num_points;++i)
                    { 
                        double x=shape_.shp().read_double();
                        double y=shape_.shp().read_double();
                        geometry_type * point = new geometry_type(mapnik::Point);
                        point->move_to(x,y);
                        feature->add_geometry(point);
                    }
        
                    // skip z
                    shape_.shp().skip(2*8 + 8*num_points);
                    
                    // check if we have measure data 
        
                    if ( shape_.reclength_ == num_points * 16 + 36)   
                    {
                        // skip m 
                        shape_.shp().skip(2*8 + 8*num_points);
                    }
                    ++count_;
                    break;
                }
                case shape_io::shape_polyline:
                {
                    geometry_type * line = shape_.read_polyline();
                    feature->add_geometry(line);
                    ++count_;
                    break;
                }
                case shape_io::shape_polylinem:
                {
                    geometry_type * line = shape_.read_polylinem();
                    feature->add_geometry(line);
                    ++count_;
                    break;
                }
                case shape_io::shape_polylinez:
                {
                    geometry_type * line = shape_.read_polylinez();
                    feature->add_geometry(line);
                    ++count_;
                    break;
                }
                case shape_io::shape_polygon:
                {         
                    geometry_type * poly = shape_.read_polygon();
                    feature->add_geometry(poly);
                    ++count_;
                    break;
                }
                case shape_io::shape_polygonm:
                {         
                    geometry_type * poly = shape_.read_polygonm();
                    feature->add_geometry(poly);
                    ++count_;
                    break;
                }
                case shape_io::shape_polygonz:
                {
                    geometry_type * poly = shape_.read_polygonz();
                    feature->add_geometry(poly);
                    ++count_;
                    break;
                }
            }
        }
        
        feature->set_id(shape_.id_);
        if (attr_ids_.size())
        {
            shape_.dbf().move_to(shape_.id_);
            std::vector<int>::const_iterator pos=attr_ids_.begin();
            std::vector<int>::const_iterator end=attr_ids_.end();
             
            while (pos!=end)
            {
                try 
                {
                    shape_.dbf().add_attribute(*pos,*tr_,*feature);//TODO optimize!!!
                }
                catch (...)
                {
                    std::clog << "Shape Plugin: error processing attributes " << std::endl;
                }
                ++pos;
            }
        }
        return feature;
    }
    else
    {
#ifdef MAPNIK_DEBUG
        std::clog << "Shape Plugin: total shapes read=" << count_ << std::endl;
#endif
        return feature_ptr();
    }
}

template <typename filterT>
shape_featureset<filterT>::~shape_featureset() {}

template class shape_featureset<mapnik::filter_in_box>;
template class shape_featureset<mapnik::filter_at_point>;

         
