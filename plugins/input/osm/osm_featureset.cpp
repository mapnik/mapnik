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

// mapnik
#include <mapnik/make_unique.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/unicode.hpp>

// boost

#include "osm_featureset.hpp"

using mapnik::feature_ptr;
using mapnik::geometry_type;
using mapnik::feature_factory;

template <typename filterT>
osm_featureset<filterT>::osm_featureset(const filterT& filter,
                                        osm_dataset* dataset,
                                        const std::set<std::string>&
                                        attribute_names,
                                        std::string const& encoding)
    : filter_(filter),
      query_ext_(),
      tr_(new transcoder(encoding)),
      dataset_ (dataset),
      attribute_names_ (attribute_names),
      ctx_(std::make_shared<mapnik::context_type>())
{
    dataset_->rewind();
}

template <typename filterT>
feature_ptr osm_featureset<filterT>::next()
{
    feature_ptr feature;

    osm_item* cur_item = dataset_->next_item();
    if (!cur_item) return feature_ptr();
    if (dataset_->current_item_is_node())
    {
        feature = feature_factory::create(ctx_, cur_item->id);
        double lat = static_cast<osm_node*>(cur_item)->lat;
        double lon = static_cast<osm_node*>(cur_item)->lon;
        std::unique_ptr<geometry_type> point = std::make_unique<geometry_type>(mapnik::geometry_type::types::Point);
        point->move_to(lon, lat);
        feature->add_geometry(point.release());
    }
    else if (dataset_->current_item_is_way())
    {
        // Loop until we find a feature which passes the filter
        while (cur_item)
        {
            bounds b = static_cast<osm_way*>(cur_item)->get_bounds();
            if (filter_.pass(box2d<double>(b.w, b.s, b.e, b.n))
                    &&
                static_cast<osm_way*>(cur_item)->nodes.size()) break;
            cur_item = dataset_->next_item();
        }

        if (!cur_item) return feature_ptr();
        feature = feature_factory::create(ctx_, cur_item->id);
        mapnik::geometry_type::types geom_type = mapnik::geometry_type::types::LineString;
        if (static_cast<osm_way*>(cur_item)->is_polygon())
        {
            geom_type = mapnik::geometry_type::types::Polygon;
        }
        std::unique_ptr<geometry_type> geom = std::make_unique<geometry_type>(geom_type);

        geom->move_to(static_cast<osm_way*>(cur_item)->nodes[0]->lon,
                      static_cast<osm_way*>(cur_item)->nodes[0]->lat);

        for (unsigned int count = 1;
             count < static_cast<osm_way*>(cur_item)->nodes.size();
             count++)
        {
            geom->line_to(static_cast<osm_way*>(cur_item)->nodes[count]->lon,
                          static_cast<osm_way*>(cur_item)->nodes[count]->lat);
        }
        feature->add_geometry(geom.release());
    }
    else
    {
        MAPNIK_LOG_ERROR(osm_featureset) << "Current item is neither node nor way.\n";
    }

    std::set<std::string>::const_iterator itr = attribute_names_.begin();
    std::set<std::string>::const_iterator end = attribute_names_.end();
    std::map<std::string,std::string>::iterator end_keyvals = cur_item->keyvals.end();
    for (; itr != end; itr++)
    {
        std::map<std::string,std::string>::iterator i = cur_item->keyvals.find(*itr);
        if (i != end_keyvals)
        {
            feature->put_new(i->first, tr_->transcode(i->second.c_str()));
        }
    }
    return feature;
}

template <typename filterT>
osm_featureset<filterT>::~osm_featureset() {}

template class osm_featureset<mapnik::filter_in_box>;
template class osm_featureset<mapnik::filter_at_point>;
