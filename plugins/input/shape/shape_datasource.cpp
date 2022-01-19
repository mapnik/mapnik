/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#include "shape_datasource.hpp"
#include "shape_featureset.hpp"
#include "shape_index_featureset.hpp"

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/version.hpp>
#include <boost/algorithm/string.hpp>
MAPNIK_DISABLE_WARNING_POP

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/global.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/timer.hpp>
#include <mapnik/value/types.hpp>

// stl
#include <fstream>
#include <sstream>
#include <stdexcept>

DATASOURCE_PLUGIN(shape_datasource)

using mapnik::String;
using mapnik::Double;
using mapnik::Integer;
using mapnik::Boolean;
using mapnik::datasource_exception;
using mapnik::filter_in_box;
using mapnik::filter_at_point;
using mapnik::attribute_descriptor;

shape_datasource::shape_datasource(parameters const& params)
    : datasource (params),
      type_(datasource::Vector),
      file_length_(0),
      indexed_(false),
      row_limit_(*params.get<mapnik::value_integer>("row_limit",0)),
      desc_(shape_datasource::name(), *params.get<std::string>("encoding","utf-8"))
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "shape_datasource::init");
#endif
    boost::optional<std::string> file = params.get<std::string>("file");
    if (!file) throw datasource_exception("Shape Plugin: missing <file> parameter");

    boost::optional<std::string> base = params.get<std::string>("base");
    if (base)
        shape_name_ = *base + "/" + *file;
    else
        shape_name_ = *file;

    boost::algorithm::ireplace_last(shape_name_,".shp","");
    if (!mapnik::util::exists(shape_name_ + ".shp"))
    {
        throw datasource_exception("Shape Plugin: shapefile '" + shape_name_ + ".shp' does not exist");
    }
    if (mapnik::util::is_directory(shape_name_ + ".shp"))
    {
        throw datasource_exception("Shape Plugin: shapefile '" + shape_name_ + ".shp' appears to be a directory not a file");
    }
    if (!mapnik::util::exists(shape_name_ + ".dbf"))
    {
        throw datasource_exception("Shape Plugin: shapefile '" + shape_name_ + ".dbf' does not exist");
    }

    try
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats2__(std::clog, "shape_datasource::init(get_column_description)");
#endif

        shape_io shape(shape_name_);
        init(shape);
        for (int i = 0; i < shape.dbf().num_fields(); ++i)
        {
            field_descriptor const& fd = shape.dbf().descriptor(i);
            std::string fld_name=fd.name_;
            switch (fd.type_)
            {
            case 'C': // character
            case 'D': // date
                desc_.add_descriptor(attribute_descriptor(fld_name, String));
                break;
            case 'L': // logical
                desc_.add_descriptor(attribute_descriptor(fld_name, Boolean));
                break;
            case 'N': // numeric
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
                // I - long
                // G - ole
                // + - autoincrement
                // @ - timestamp
                // B - binary
                // l - long
                // M - memo
                MAPNIK_LOG_ERROR(shape) << "shape_datasource: Unknown type=" << fd.type_;
                break;
            }
        }
    }
    catch (datasource_exception const& ex)
    {
        MAPNIK_LOG_ERROR(shape) << "Shape Plugin: error processing field attributes, " << ex.what();
        throw;
    }
    catch (const std::exception& ex)
    {
        MAPNIK_LOG_ERROR(shape) << "Shape Plugin: error processing field attributes, " << ex.what();
        throw;
    }
    catch (...) // exception: pipe_select_interrupter: Too many open files
    {
        MAPNIK_LOG_ERROR(shape) << "Shape Plugin: error processing field attributes";
        throw;
    }

}

void shape_datasource::init(shape_io& shape)
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "shape_datasource::init");
#endif

    //first read header from *.shp
    shape_file::record_type header(100);
    shape.shp().read_record(header);

    int file_code = header.read_xdr_integer();
    if (file_code != 9994)
    {
        std::ostringstream s;
        s << "Shape Plugin: wrong file code " << file_code;
        throw datasource_exception(s.str());
    }
    header.skip(5 * 4);
    file_length_ = header.read_xdr_integer();
    int version = header.read_ndr_integer();

    if (version != 1000)
    {
        std::ostringstream s;
        s << "Shape Plugin: nvalid version number " << version;
        throw datasource_exception(s.str());
    }

    shape_type_ = static_cast<shape_io::shapeType>(header.read_ndr_integer());
    if (shape_type_ == shape_io::shape_multipatch)
        throw datasource_exception("Shape Plugin: shapefile multipatch type is not supported");

    const double lox = header.read_double();
    const double loy = header.read_double();
    const double hix = header.read_double();
    const double hiy = header.read_double();
    extent_.init(lox, loy, hix, hiy);

#ifdef MAPNIK_LOG
    const double zmin = header.read_double();
    const double zmax = header.read_double();
    const double mmin = header.read_double();
    const double mmax = header.read_double();

    MAPNIK_LOG_DEBUG(shape) << "shape_datasource: Z min/max=" << zmin << "," << zmax;
    MAPNIK_LOG_DEBUG(shape) << "shape_datasource: M min/max=" << mmin << "," << mmax;
#endif

    // check if we have an index file around
    indexed_ = shape.has_index();
    MAPNIK_LOG_DEBUG(shape) << "shape_datasource: Extent=" << extent_;
    MAPNIK_LOG_DEBUG(shape) << "shape_datasource: File length=" << file_length_;
    MAPNIK_LOG_DEBUG(shape) << "shape_datasource: Shape type=" << shape_type_;
}

shape_datasource::~shape_datasource() {}

const char * shape_datasource::name()
{
    return "shape";
}

datasource::datasource_t shape_datasource::type() const
{
    return type_;
}

layer_descriptor shape_datasource::get_descriptor() const
{
    return desc_;
}

featureset_ptr shape_datasource::features(query const& q) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "shape_datasource::features");
#endif

    auto const& query_box = q.get_bbox();

    if (indexed_)
    {
        std::unique_ptr<shape_io> shape_ptr = std::make_unique<shape_io>(shape_name_);
        mapnik::bounding_box_filter<float> filter(mapnik::box2d<float>(query_box.minx(), query_box.miny(), query_box.maxx(), query_box.maxy()));
        return featureset_ptr
            (new shape_index_featureset<mapnik::bounding_box_filter<float>>(filter,
                                                                            std::move(shape_ptr),
                                                                            q.property_names(),
                                                                            desc_.get_encoding(),
                                                                            shape_name_,
                                                                            row_limit_));
    }
    else
    {
        mapnik::bounding_box_filter<double> filter(q.get_bbox());
        return std::make_shared<shape_featureset< mapnik::bounding_box_filter<double>>>(filter,
                                                                  shape_name_,
                                                                  q.property_names(),
                                                                  desc_.get_encoding(),
                                                                  row_limit_);
    }
}

featureset_ptr shape_datasource::features_at_point(coord2d const& pt, double tol) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "shape_datasource::features_at_point");
#endif


    // collect all attribute names
    auto const& desc = desc_.get_descriptors();
    std::set<std::string> names;

    for (auto const& attr_info : desc)
    {
        names.insert(attr_info.get_name());
    }

    if (indexed_)
    {
        std::unique_ptr<shape_io> shape_ptr = std::make_unique<shape_io>(shape_name_);
        mapnik::at_point_filter<float> filter(mapnik::coord2f(pt.x, pt.y));
        return featureset_ptr
            (new shape_index_featureset<mapnik::at_point_filter<float>>(filter,
                                                                        std::move(shape_ptr),
                                                                        names,
                                                                        desc_.get_encoding(),
                                                                        shape_name_,
                                                                        row_limit_));
    }
    else
    {
        filter_at_point filter(pt,tol);
        return std::make_shared<shape_featureset<filter_at_point> >(filter,
                                                                    shape_name_,
                                                                    names,
                                                                    desc_.get_encoding(),
                                                                    row_limit_);
    }
}

box2d<double> shape_datasource::envelope() const
{
    return extent_;
}

boost::optional<mapnik::datasource_geometry_t> shape_datasource::get_geometry_type() const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "shape_datasource::get_geometry_type");
#endif

    boost::optional<mapnik::datasource_geometry_t> result;
    switch (shape_type_)
    {
    case shape_io::shape_point:
    case shape_io::shape_pointm:
    case shape_io::shape_pointz:
    case shape_io::shape_multipoint:
    case shape_io::shape_multipointm:
    case shape_io::shape_multipointz:
    {
        result.reset(mapnik::datasource_geometry_t::Point);
        break;
    }
    case shape_io::shape_polyline:
    case shape_io::shape_polylinem:
    case shape_io::shape_polylinez:
    {
        result.reset(mapnik::datasource_geometry_t::LineString);
        break;
    }
    case shape_io::shape_polygon:
    case shape_io::shape_polygonm:
    case shape_io::shape_polygonz:
    {
        result.reset(mapnik::datasource_geometry_t::Polygon);
        break;
    }
    default:
        break;
    }
    return result;
}
