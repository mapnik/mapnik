/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <fstream>

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/feature_factory.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#include <boost/interprocess/streams/bufferstream.hpp>
#endif
#pragma GCC diagnostic pop

#include "shape_index_featureset.hpp"
#include "shape_utils.hpp"
#include <mapnik/util/spatial_index.hpp>

using mapnik::feature_factory;

template <typename filterT>
shape_index_featureset<filterT>::shape_index_featureset(filterT const& filter,
                                                        std::unique_ptr<shape_io> && shape_ptr,
                                                        std::set<std::string> const& attribute_names,
                                                        std::string const& encoding,
                                                        std::string const& shape_name,
                                                        int row_limit)
    : filter_(filter),
      ctx_(std::make_shared<mapnik::context_type>()),
      shape_ptr_(std::move(shape_ptr)),
      tr_(new mapnik::transcoder(encoding)),
      offsets_(),
      itr_(),
      attr_ids_(),
      row_limit_(row_limit),
      count_(0),
      feature_bbox_()
{
    shape_ptr_->shp().skip(100);
    setup_attributes(ctx_, attribute_names, shape_name, *shape_ptr_,attr_ids_);

    auto index = shape_ptr_->index();
    if (index)
    {
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
        mapnik::util::spatial_index<mapnik::detail::node, filterT,boost::interprocess::ibufferstream>::query(filter, index->file(), offsets_);
#else
        mapnik::util::spatial_index<mapnik::detail::node, filterT, std::ifstream>::query(filter, index->file(), offsets_);
#endif
    }
    std::sort(offsets_.begin(), offsets_.end(), [](mapnik::detail::node const& n0, mapnik::detail::node const& n1)
              {return n0.offset != n1.offset ? n0.offset < n1.offset : n0.start < n1.start;});
    MAPNIK_LOG_DEBUG(shape) << "shape_index_featureset: Query size=" << offsets_.size();
    itr_ = offsets_.begin();
}

template <typename filterT>
feature_ptr shape_index_featureset<filterT>::next()
{
    if (row_limit_ && count_ >= row_limit_)
    {
        return feature_ptr();
    }

    while ( itr_ != offsets_.end())
    {
        std::uint64_t offset = itr_->offset;
        shape_ptr_->move_to(offset);
        std::vector<std::pair<int,int>> parts;
        while (itr_ != offsets_.end() && itr_->offset == offset)
        {
            if (itr_->start!= -1) parts.emplace_back(itr_->start, itr_->end);
            ++itr_;
        }
        mapnik::value_integer feature_id = shape_ptr_->id();
        shape_file::record_type record(shape_ptr_->reclength_ * 2);
        shape_ptr_->shp().read_record(record);
        int type = record.read_ndr_integer();
        feature_ptr feature(feature_factory::create(ctx_, feature_id));

        switch (type)
        {
        case shape_io::shape_point:
        case shape_io::shape_pointm:
        case shape_io::shape_pointz:
        {
            double x = record.read_double();
            double y = record.read_double();
            feature->set_geometry(mapnik::geometry::point<double>(x,y));
            break;
        }
        case shape_io::shape_multipoint:
        case shape_io::shape_multipointm:
        case shape_io::shape_multipointz:
        {
            shape_io::read_bbox(record, feature_bbox_);
            if (!filter_.pass(feature_bbox_)) continue;
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
        case shape_io::shape_polylinez:
        {
            shape_io::read_bbox(record, feature_bbox_);
            if (!filter_.pass(feature_bbox_)) continue;
            if (parts.size() < 2) feature->set_geometry(shape_io::read_polyline(record));
            else feature->set_geometry(shape_io::read_polyline_parts(record, parts));
            break;
        }
        case shape_io::shape_polygon:
        case shape_io::shape_polygonm:
        case shape_io::shape_polygonz:
        {
            shape_io::read_bbox(record, feature_bbox_);
            if (!filter_.pass(feature_bbox_)) continue;
            if (parts.size() < 2) feature->set_geometry(shape_io::read_polygon(record));
            else feature->set_geometry(shape_io::read_polygon_parts(record, parts));
            break;
        }
        default :
            MAPNIK_LOG_DEBUG(shape) << "shape_index_featureset: Unsupported type" << type;
            return feature_ptr();
        }

        if (attr_ids_.size())
        {
            shape_ptr_->dbf().move_to(shape_ptr_->id_);
            try
            {
                for (auto id : attr_ids_)
                {
                    shape_ptr_->dbf().add_attribute(id, *tr_, *feature);
                }
            }
            catch (...)
            {
                MAPNIK_LOG_ERROR(shape) << "Shape Plugin: error processing attributes";
            }
        }
        ++count_;
        return feature;
    }

    MAPNIK_LOG_DEBUG(shape) << "shape_index_featureset: " << count_ << " features";
    return feature_ptr();
}


template <typename filterT>
shape_index_featureset<filterT>::~shape_index_featureset() {}

template class shape_index_featureset<mapnik::filter_in_box>;
template class shape_index_featureset<mapnik::filter_at_point>;
