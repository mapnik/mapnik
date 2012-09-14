/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

// stl
#include <iostream>

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/feature_factory.hpp>

#include "shape_featureset.hpp"
#include "shape_utils.hpp"

using mapnik::geometry_type;
using mapnik::feature_factory;
using mapnik::context_ptr;

template <typename filterT>
shape_featureset<filterT>::shape_featureset(filterT const& filter,
                                            std::string const& shape_name,
                                            std::set<std::string> const& attribute_names,
                                            std::string const& encoding,
                                            long file_length,
                                            int row_limit)
    : filter_(filter),
      shape_(shape_name, false),
      query_ext_(),
      tr_(new transcoder(encoding)),
      file_length_(file_length),
      row_limit_(row_limit),
      count_(0),
      ctx_(boost::make_shared<mapnik::context_type>())
{
    shape_.shp().skip(100);
    setup_attributes(ctx_, attribute_names, shape_name, shape_,attr_ids_);
}

template <typename filterT>
feature_ptr shape_featureset<filterT>::next()
{
    if (row_limit_ && count_ > row_limit_)
    {
        return feature_ptr();
    }

    std::streampos pos = shape_.shp().pos();

    // skip null shapes
    while (pos > 0 && pos < std::streampos(file_length_ * 2))
    {
        shape_.move_to(pos);
        if (shape_.type() == shape_io::shape_null)
        {
            pos += std::streampos(12);
        }
        else
        {
            break;
        }
    }

    if (pos < std::streampos(file_length_ * 2))
    {
        int type = shape_.type();
        feature_ptr feature(feature_factory::create(ctx_, shape_.id_));

        if (type == shape_io::shape_point)
        {
            double x = shape_.shp().read_double();
            double y = shape_.shp().read_double();
            geometry_type* point = new geometry_type(mapnik::Point);
            point->move_to(x, y);
            feature->add_geometry(point);
            ++count_;
        }
        else if (type == shape_io::shape_pointm)
        {
            double x = shape_.shp().read_double();
            double y = shape_.shp().read_double();
            // skip m
            shape_.shp().skip(8);
            geometry_type* point = new geometry_type(mapnik::Point);
            point->move_to(x, y);
            feature->add_geometry(point);
            ++count_;
        }
        else if (type == shape_io::shape_pointz)
        {
            double x = shape_.shp().read_double();
            double y = shape_.shp().read_double();
            // skip z
            shape_.shp().skip(8);
            // skip m if exists
            if (shape_.reclength_ == 8 + 36)
            {
                shape_.shp().skip(8);
            }
            geometry_type* point = new geometry_type(mapnik::Point);
            point->move_to(x, y);
            feature->add_geometry(point);
            ++count_;
        }
        else
        {
            // skip shapes
            for (;;)
            {
                std::streampos pos = shape_.shp().pos();
                if (shape_.type() == shape_io::shape_null)
                {
                    pos += std::streampos(12);

                    // TODO handle the shapes
                    MAPNIK_LOG_WARN(shape) << "shape_featureset: NULL SHAPE len=" << shape_.reclength_;
                }
                else if (filter_.pass(shape_.current_extent()))
                {
                    break;
                }
                else
                {
                    pos += std::streampos(2 * shape_.reclength_ - 36);
                }

                if (pos > 0 && pos < std::streampos(file_length_ * 2))
                {
                    shape_.move_to(pos);
                }
                else
                {
                    MAPNIK_LOG_DEBUG(shape) << "shape_featureset: Total shapes read=" << count_;

                    return feature_ptr();
                }
            }

            switch (type)
            {
            case shape_io::shape_multipoint:
            {
                int num_points = shape_.shp().read_ndr_integer();
                for (int i = 0; i < num_points; ++i)
                {
                    double x = shape_.shp().read_double();
                    double y = shape_.shp().read_double();
                    geometry_type* point = new geometry_type(mapnik::Point);
                    point->move_to(x, y);
                    feature->add_geometry(point);
                }
                ++count_;
                break;
            }

            case shape_io::shape_multipointm:
            {
                int num_points = shape_.shp().read_ndr_integer();
                for (int i = 0; i < num_points; ++i)
                {
                    double x = shape_.shp().read_double();
                    double y = shape_.shp().read_double();
                    geometry_type* point = new geometry_type(mapnik::Point);
                    point->move_to(x, y);
                    feature->add_geometry(point);
                }

                // skip m
                shape_.shp().skip(2 * 8 + 8 * num_points);
                ++count_;
                break;
            }

            case shape_io::shape_multipointz:
            {
                int num_points = shape_.shp().read_ndr_integer();
                for (int i = 0; i < num_points; ++i)
                {
                    double x = shape_.shp().read_double();
                    double y = shape_.shp().read_double();
                    geometry_type* point = new geometry_type(mapnik::Point);
                    point->move_to(x, y);
                    feature->add_geometry(point);
                }

                // skip z
                shape_.shp().skip(2 * 8 + 8 * num_points);

                // check if we have measure data
                if (shape_.reclength_ == (unsigned)(num_points * 16 + 36))
                {
                    // skip m
                    shape_.shp().skip(2 * 8 + 8 * num_points);
                }
                ++count_;
                break;
            }

            case shape_io::shape_polyline:
            case shape_io::shape_polylinem:
            case shape_io::shape_polylinez:
            {
                shape_.read_polyline(feature->paths());
                ++count_;
                break;
            }

            case shape_io::shape_polygon:
            case shape_io::shape_polygonm:
            case shape_io::shape_polygonz:
            {
                shape_.read_polygon(feature->paths());
                ++count_;
                break;
            }
            }
        }
        // FIXME: https://github.com/mapnik/mapnik/issues/1020
        feature->set_id(shape_.id_);
        if (attr_ids_.size())
        {
            shape_.dbf().move_to(shape_.id_);
            std::vector<int>::const_iterator itr = attr_ids_.begin();
            std::vector<int>::const_iterator end = attr_ids_.end();
            try
            {
                for (; itr != end; ++itr)
                {
                    shape_.dbf().add_attribute(*itr, *tr_, *feature); //TODO optimize!!!
                }
            }
            catch (...)
            {
                MAPNIK_LOG_ERROR(shape) << "Shape Plugin: error processing attributes";
            }
        }

        return feature;
    }
    else
    {
        MAPNIK_LOG_DEBUG(shape) << "shape_featureset: Total shapes read=" << count_;

        return feature_ptr();
    }
}

template <typename filterT>
shape_featureset<filterT>::~shape_featureset() {}

template class shape_featureset<mapnik::filter_in_box>;
template class shape_featureset<mapnik::filter_at_point>;
