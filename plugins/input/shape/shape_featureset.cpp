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

// stl
#include <iostream>
#include <cassert>
// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/unicode.hpp>

// boost

#include "shape_featureset.hpp"
#include "shape_utils.hpp"

using mapnik::context_ptr;
using mapnik::feature_factory;

template<typename filterT>
shape_featureset<filterT>::shape_featureset(filterT const& filter,
                                            std::string const& shape_name,
                                            std::set<std::string> const& attribute_names,
                                            std::string const& encoding,
                                            int row_limit)
    : filter_(filter)
    , shape_(shape_name, false)
    , query_ext_()
    , feature_bbox_()
    , tr_(new transcoder(encoding))
    , shx_file_length_(0)
    , row_limit_(row_limit)
    , count_(0)
    , ctx_(std::make_shared<mapnik::context_type>())
{
    if (!shape_.shx().is_open())
    {
        throw mapnik::datasource_exception("Shape Plugin: can't open '" + shape_name + ".shx' file");
    }
    shape_file::record_type shx_header(100);
    shape_.shx().read_record(shx_header);
    shx_header.skip(6 * 4);
    shx_file_length_ = shx_header.read_xdr_integer();
    setup_attributes(ctx_, attribute_names, shape_name, shape_, attr_ids_);
}

template<typename filterT>
feature_ptr shape_featureset<filterT>::next()
{
    if (row_limit_ && count_ >= row_limit_)
    {
        return feature_ptr();
    }

    std::streampos position_limit = 2 * shx_file_length_ - 2 * sizeof(int);
    while (shape_.shx().is_good() && shape_.shx().pos() <= position_limit)
    {
        int offset = shape_.shx().read_xdr_integer();
        int record_length = shape_.shx().read_xdr_integer();
        shape_.move_to(2 * offset);
        mapnik::value_integer feature_id = shape_.id();
        assert(record_length == shape_.reclength_);
        shape_file::record_type record(record_length * 2);
        shape_.shp().read_record(record);
        int type = record.read_ndr_integer();

        // skip null shapes
        if (type == shape_io::shape_null)
            continue;

        feature_ptr feature(feature_factory::create(ctx_, feature_id));
        switch (type)
        {
            case shape_io::shape_point:
            case shape_io::shape_pointm:
            case shape_io::shape_pointz: {
                double x = record.read_double();
                double y = record.read_double();
                if (!filter_.pass(mapnik::box2d<double>(x, y, x, y)))
                    continue;
                feature->set_geometry(mapnik::geometry::point<double>(x, y));
                break;
            }
            case shape_io::shape_multipoint:
            case shape_io::shape_multipointm:
            case shape_io::shape_multipointz: {
                shape_io::read_bbox(record, feature_bbox_);
                if (!filter_.pass(feature_bbox_))
                    continue;
                int num_points = record.read_ndr_integer();
                mapnik::geometry::multi_point<double> multi_point;
                for (int i = 0; i < num_points; ++i)
                {
                    double x = record.read_double();
                    double y = record.read_double();
                    multi_point.emplace_back(mapnik::geometry::point<double>(x, y));
                }
                feature->set_geometry(std::move(multi_point));
                break;
            }

            case shape_io::shape_polyline:
            case shape_io::shape_polylinem:
            case shape_io::shape_polylinez: {
                shape_io::read_bbox(record, feature_bbox_);
                if (!filter_.pass(feature_bbox_))
                    continue;
                feature->set_geometry(shape_io::read_polyline(record));
                break;
            }
            case shape_io::shape_polygon:
            case shape_io::shape_polygonm:
            case shape_io::shape_polygonz: {
                shape_io::read_bbox(record, feature_bbox_);
                if (!filter_.pass(feature_bbox_))
                    continue;
                feature->set_geometry(shape_io::read_polygon(record));
                break;
            }
            default:
                MAPNIK_LOG_DEBUG(shape) << "shape_featureset: Unsupported type" << type;
                return feature_ptr();
        }

        if (attr_ids_.size())
        {
            shape_.dbf().move_to(shape_.id_);
            try
            {
                for (auto id : attr_ids_)
                {
                    shape_.dbf().add_attribute(id, *tr_, *feature); // TODO optimize!!!
                }
            } catch (...)
            {
                MAPNIK_LOG_ERROR(shape) << "Shape Plugin: error processing attributes";
            }
        }
        ++count_;
        return feature;
    }

    MAPNIK_LOG_DEBUG(shape) << "shape_featureset: Total shapes read=" << count_;
    return feature_ptr();
}

template<typename filterT>
shape_featureset<filterT>::~shape_featureset()
{}

template class shape_featureset<mapnik::filter_in_box>;
template class shape_featureset<mapnik::filter_at_point>;
