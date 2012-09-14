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
#include <fstream>

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/feature_factory.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>

#include "shape_index_featureset.hpp"
#include "shape_utils.hpp"

using mapnik::feature_factory;
using mapnik::geometry_type;

template <typename filterT>
shape_index_featureset<filterT>::shape_index_featureset(filterT const& filter,
                                                        shape_io& shape,
                                                        std::set<std::string> const& attribute_names,
                                                        std::string const& encoding,
                                                        std::string const& shape_name,
                                                        int row_limit)
    : filter_(filter),
      ctx_(boost::make_shared<mapnik::context_type>()),
      shape_(shape),
      tr_(new transcoder(encoding)),
      row_limit_(row_limit),
      count_(0)
{
    shape_.shp().skip(100);
    setup_attributes(ctx_, attribute_names, shape_name, shape_,attr_ids_);

    boost::shared_ptr<shape_file> index = shape_.index();
    if (index)
    {
#ifdef SHAPE_MEMORY_MAPPED_FILE
        //shp_index<filterT,stream<mapped_file_source> >::query(filter, index->file(), ids_);
        shp_index<filterT,boost::interprocess::ibufferstream>::query(filter, index->file(), ids_);
#else
        shp_index<filterT,std::ifstream>::query(filter, index->file(), ids_);
#endif
    }

    std::sort(ids_.begin(), ids_.end());

    MAPNIK_LOG_DEBUG(shape) << "shape_index_featureset: Query size=" << ids_.size();

    itr_ = ids_.begin();
}

template <typename filterT>
feature_ptr shape_index_featureset<filterT>::next()
{
    if (row_limit_ && count_ > row_limit_)
    {
        return feature_ptr();
    }

    if (itr_ != ids_.end())
    {
        int pos = *itr_++;
        shape_.move_to(pos);

        int type = shape_.type();
        feature_ptr feature(feature_factory::create(ctx_,shape_.id_));
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
            while(! filter_.pass(shape_.current_extent()) &&
                  itr_ != ids_.end())
            {
                if (shape_.type() != shape_io::shape_null)
                {
                    pos = *itr_++;
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
            case shape_io::shape_multipointm:
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
                // ignore m and z for now
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
        // FIXME
        feature->set_id(shape_.id_);
        if (attr_ids_.size())
        {
            shape_.dbf().move_to(shape_.id_);
            std::vector<int>::const_iterator itr = attr_ids_.begin();
            std::vector<int>::const_iterator end = attr_ids_.end();
            try
            {
                for (; itr!=end; ++itr)
                {
                    shape_.dbf().add_attribute(*itr, *tr_, *feature);
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
        MAPNIK_LOG_DEBUG(shape) << "shape_index_featureset: " << count_ << " features";

        return feature_ptr();
    }
}


template <typename filterT>
shape_index_featureset<filterT>::~shape_index_featureset() {}

template class shape_index_featureset<mapnik::filter_in_box>;
template class shape_index_featureset<mapnik::filter_at_point>;
