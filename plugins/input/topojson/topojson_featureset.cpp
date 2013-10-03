/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
// stl
#include <string>
#include <vector>
#include <fstream>
// boost
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include "topojson_featureset.hpp"

namespace mapnik { namespace topojson {

template <typename Context>
struct feature_generator : public boost::static_visitor<mapnik::feature_ptr>
{
    feature_generator(Context & ctx, topology const& topo, std::size_t feature_id)
        : ctx_(ctx),
          topo_(topo),
          feature_id_(feature_id) {}

    feature_ptr operator() (point const& pt) const
    {
        return feature_ptr();
    }

    feature_ptr operator() (linestring const& line) const
    {
        return feature_ptr();
    }

    feature_ptr operator() (polygon const& poly) const
    {
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));
        std::unique_ptr<geometry_type> poly_ptr(new geometry_type(geometry_type::types::Polygon));

        for (auto const& ring : poly.rings)
        {
            bool first = true;
            for (auto const& index : ring)
            {
                double px = 0, py = 0;
                bool reversed = index < 0;
                index_type arc_index = reversed ? std::abs(index) - 1 : index;
                auto const& coords = topo_.arcs[arc_index].coordinates;
                std::deque<mapnik::topojson::coordinate> processed_coords;
                for (auto const& pt : coords )
                {
                    double x = pt.x;
                    double y = pt.y;

                    if (topo_.tr)
                    {
                        x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                        y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                    }
                    if (reversed)
                        processed_coords.emplace_front(coordinate{x,y});
                    else
                        processed_coords.emplace_back(coordinate{x,y});
                }

                for (auto const& c : processed_coords)
                {
                    if (first)
                    {
                        first = false;
                        poly_ptr->move_to(c.x,c.y);
                    }
                    else poly_ptr->line_to(c.x,c.y);
                }
            }
            poly_ptr->close_path();
        }
        feature->paths().push_back(poly_ptr.release());
        return feature;
    }

    template<typename T>
    feature_ptr operator() (T const& ) const
    {
        return feature_ptr();
    }

    Context & ctx_;
    topology const& topo_;
    std::size_t feature_id_;
};

}}

topojson_featureset::topojson_featureset(mapnik::topojson::topology const& topo,
                                       std::deque<std::size_t>::const_iterator index_itr,
                                       std::deque<std::size_t>::const_iterator index_end)
    : ctx_(std::make_shared<mapnik::context_type>()),
      topo_(topo),
      index_itr_(index_itr),
      index_end_(index_end),
      feature_id_ (0) {}

topojson_featureset::~topojson_featureset() {}

mapnik::feature_ptr topojson_featureset::next()
{
    if (index_itr_ != index_end_)
    {
        std::size_t index = *index_itr_++;
        if ( index < topo_.geometries.size())
        {
            mapnik::topojson::geometry const& geom = topo_.geometries[index];
            mapnik::feature_ptr feature = boost::apply_visitor(
                mapnik::topojson::feature_generator<mapnik::context_ptr>(ctx_, topo_, feature_id_++),
                geom);
            return feature;
        }
    }

    return mapnik::feature_ptr();
}
