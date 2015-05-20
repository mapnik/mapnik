/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/json/topology.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/geometry_adapters.hpp>
#include <mapnik/geometry_correct.hpp>

// stl
#include <string>
#include <vector>
#include <fstream>

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/range/adaptor/reversed.hpp>
#pragma GCC diagnostic pop

#include "topojson_featureset.hpp"

namespace mapnik { namespace topojson {

struct attribute_value_visitor

{
public:
    attribute_value_visitor(mapnik::transcoder const& tr)
        : tr_(tr) {}

    mapnik::value operator()(std::string const& val) const
    {
        return mapnik::value(tr_.transcode(val.c_str()));
    }

    template <typename T>
    mapnik::value operator()(T const& val) const
    {
        return mapnik::value(val);
    }

    mapnik::transcoder const& tr_;
};

template <typename T>
void assign_properties(mapnik::feature_impl & feature, T const& geom, mapnik::transcoder const& tr)
{
    if ( geom.props)
    {
        for (auto const& p : *geom.props)
        {
            feature.put_new(std::get<0>(p), mapnik::util::apply_visitor(attribute_value_visitor(tr),std::get<1>(p)));
        }
    }
}

template <typename Context>
struct feature_generator
{
    feature_generator(Context & ctx,  mapnik::transcoder const& tr, topology const& topo, std::size_t feature_id)
        : ctx_(ctx),
          tr_(tr),
          topo_(topo),
          num_arcs_(topo.arcs.size()),
          feature_id_(feature_id) {}

    feature_ptr operator() (point const& pt) const
    {
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));
        double x = pt.coord.x;
        double y = pt.coord.y;
        if (topo_.tr)
        {
            x =  x * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
            y =  y * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
        }
        mapnik::geometry::point<double> point(x, y);
        feature->set_geometry(std::move(point));
        assign_properties(*feature, pt, tr_);
        return feature;
    }

    feature_ptr operator() (multi_point const& multi_pt) const
    {
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));
        mapnik::geometry::multi_point<double> multi_point;
        multi_point.reserve(multi_pt.points.size());
        for (auto const& pt : multi_pt.points)
        {
            double x = pt.x;
            double y = pt.y;
            if (topo_.tr)
            {
                x =  x * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                y =  y * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
            }
            multi_point.add_coord(x, y);
        }
        feature->set_geometry(std::move(multi_point));
        assign_properties(*feature, multi_pt, tr_);
        return feature;
    }

    feature_ptr operator() (linestring const& line) const
    {
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));
        if (num_arcs_ > 0)
        {
            index_type index = line.ring;
            index_type arc_index = index < 0 ? std::abs(index) - 1 : index;
            if (arc_index < num_arcs_)
            {            
                auto const& arcs = topo_.arcs[arc_index];
                double px = 0, py = 0;
                mapnik::geometry::line_string<double> line_string;
                line_string.reserve(arcs.coordinates.size());

                for (auto pt : arcs.coordinates)
                {
                    double x = pt.x;
                    double y = pt.y;
                    if (topo_.tr)
                    {
                        x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                        y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                    }
                    line_string.add_coord(x,y);
                }
                feature->set_geometry(std::move(line_string));
                assign_properties(*feature, line, tr_);
            }
        }
        return feature;
    }

    feature_ptr operator() (multi_linestring const& multi_line) const
    {
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));
        if (num_arcs_ > 0)
        {
            mapnik::geometry::multi_line_string<double> multi_line_string;
            bool hit = false;
            multi_line_string.reserve(multi_line.rings.size());
            for (auto const& index : multi_line.rings)
            {
                index_type arc_index = index < 0 ? std::abs(index) - 1 : index;
                if (arc_index < num_arcs_)
                {
                    hit = true;
                    double px = 0, py = 0;
                    mapnik::geometry::line_string<double> line_string;
                    auto const& arcs = topo_.arcs[arc_index];
                    line_string.reserve(arcs.coordinates.size());
                    for (auto pt : arcs.coordinates)
                    {
                        double x = pt.x;
                        double y = pt.y;
                        if (topo_.tr)
                        {
                            x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                            y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                        }
                        line_string.add_coord(x, y);
                    }
                    multi_line_string.push_back(std::move(line_string));
                }
            }
            if (hit)
            {
                feature->set_geometry(std::move(multi_line_string));
                assign_properties(*feature, multi_line, tr_);                
            }
        }
        return feature;
    }

    feature_ptr operator() (polygon const& poly) const
    {
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));
        if (num_arcs_ > 0)
        {
            std::vector<mapnik::topojson::coordinate> processed_coords;
            mapnik::geometry::polygon<double> polygon;
            if (poly.rings.size() > 1) polygon.interior_rings.reserve(poly.rings.size() - 1);
            bool first = true;
            bool hit = false;
            for (auto const& ring : poly.rings)
            {
                mapnik::geometry::linear_ring<double> linear_ring;
                for (auto const& index : ring)
                {
                    double px = 0, py = 0;
                    bool reverse = index < 0;
                    index_type arc_index = reverse ? std::abs(index) - 1 : index;
                    if (arc_index < num_arcs_)
                    {
                        hit = true;
                        auto const& arcs = topo_.arcs[arc_index];
                        auto const& coords = arcs.coordinates;
                        processed_coords.clear();
                        processed_coords.reserve(coords.size());
                        for (auto const& pt : coords )
                        {
                            double x = pt.x;
                            double y = pt.y;

                            if (topo_.tr)
                            {
                                transform const& tr = *topo_.tr;
                                x =  (px += x) * tr.scale_x + tr.translate_x;
                                y =  (py += y) * tr.scale_y + tr.translate_y;
                            }
                            processed_coords.emplace_back(coordinate{x,y});
                        }

                        if (reverse)
                        {
                            for (auto const& c : processed_coords | boost::adaptors::reversed)
                            {
                                linear_ring.emplace_back(c.x, c.y);
                            }
                        }
                        else
                        {
                            for (auto const& c : processed_coords)
                            {
                                linear_ring.emplace_back(c.x, c.y);
                            }
                        }
                    }
                }
                if (first)
                {
                    first = false;
                    polygon.set_exterior_ring(std::move(linear_ring));
                }
                else
                {
                    polygon.add_hole(std::move(linear_ring));
                }
            }
            if (hit)
            {
                mapnik::geometry::correct(polygon);
                feature->set_geometry(std::move(polygon));
                assign_properties(*feature, poly, tr_);                
            }
        }
        return feature;
    }

    feature_ptr operator() (multi_polygon const& multi_poly) const
    {
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));
        if (num_arcs_ > 0)
        {
            std::vector<mapnik::topojson::coordinate> processed_coords;
            mapnik::geometry::multi_polygon<double> multi_polygon;
            multi_polygon.reserve(multi_poly.polygons.size());
            bool hit = false;
            for (auto const& poly : multi_poly.polygons)
            {
                bool first = true;
                mapnik::geometry::polygon<double> polygon;
                if (poly.size() > 1) polygon.interior_rings.reserve(poly.size() - 1);

                for (auto const& ring : poly)
                {
                    mapnik::geometry::linear_ring<double> linear_ring;
                    for (auto const& index : ring)
                    {
                        double px = 0, py = 0;
                        bool reverse = index < 0;
                        index_type arc_index = reverse ? std::abs(index) - 1 : index;
                        if (arc_index < num_arcs_)
                        {
                            hit = true;
                            auto const& arcs = topo_.arcs[arc_index];
                            auto const& coords = arcs.coordinates;
                            processed_coords.clear();
                            processed_coords.reserve(coords.size());
                            for (auto const& pt : coords )
                            {
                                double x = pt.x;
                                double y = pt.y;

                                if (topo_.tr)
                                {
                                    x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                                    y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                                }
                                processed_coords.emplace_back(coordinate{x,y});
                            }

                            using namespace boost::adaptors;

                            if (reverse)
                            {
                                for (auto const& c : (processed_coords | reversed))
                                {
                                    linear_ring.add_coord(c.x, c.y);
                                }
                            }
                            else
                            {
                                for (auto const& c : processed_coords)
                                {
                                    linear_ring.add_coord(c.x, c.y);
                                }
                            }
                        }
                    }
                    if (first)
                    {
                        first = false;
                        polygon.set_exterior_ring(std::move(linear_ring));
                    }
                    else
                    {
                        polygon.add_hole(std::move(linear_ring));
                    }
                }
                multi_polygon.push_back(std::move(polygon));
            }
            if (hit)
            {
                mapnik::geometry::correct(multi_polygon);
                feature->set_geometry(std::move(multi_polygon));
                assign_properties(*feature, multi_poly, tr_);                
            }
        }
        return feature;
    }

    template<typename T>
    feature_ptr operator() (T const& ) const
    {
        return feature_ptr();
    }

    Context & ctx_;
    mapnik::transcoder const& tr_;
    topology const& topo_;
    std::size_t num_arcs_;
    std::size_t feature_id_;
};

}}

topojson_featureset::topojson_featureset(mapnik::topojson::topology const& topo,
                                         mapnik::transcoder const& tr,
                                         array_type && index_array)
    : ctx_(std::make_shared<mapnik::context_type>()),
      topo_(topo),
      tr_(tr),
      index_array_(std::move(index_array)),
      index_itr_(index_array_.begin()),
      index_end_(index_array_.end()),
      feature_id_(1) {}

topojson_featureset::~topojson_featureset() {}

mapnik::feature_ptr topojson_featureset::next()
{
    if (index_itr_ != index_end_)
    {
        topojson_datasource::item_type const& item = *index_itr_++;
        std::size_t index = item.second;
        if ( index < topo_.geometries.size())
        {
            mapnik::topojson::geometry const& geom = topo_.geometries[index];
            mapnik::feature_ptr feature = mapnik::util::apply_visitor(
                mapnik::topojson::feature_generator<mapnik::context_ptr>(ctx_, tr_, topo_, feature_id_++),
                geom);
            return feature;
        }
    }

    return mapnik::feature_ptr();
}
