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

#ifndef MAPNIK_TOPOJSON_UTILS_HPP
#define MAPNIK_TOPOJSON_UTILS_HPP

// mapnik
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/json/topology.hpp>
#include <mapnik/json/attribute_value_visitor.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry/boost_adapters.hpp>
#include <mapnik/geometry/correct.hpp>

namespace mapnik { namespace topojson {

struct bounding_box_visitor
{
    bounding_box_visitor(topology const& topo)
        : topo_(topo),
          num_arcs_(topo_.arcs.size()) {}

    box2d<double> operator() (mapnik::topojson::empty const&) const
    {
        return box2d<double>();
    }

    box2d<double> operator() (mapnik::topojson::point const& pt) const
    {
        double x = pt.coord.x;
        double y = pt.coord.y;
        if (topo_.tr)
        {
            x =  x * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
            y =  y * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
        }
        return box2d<double>(x, y, x, y);
    }

    box2d<double> operator() (mapnik::topojson::multi_point const& multi_pt) const
    {
        box2d<double> bbox;
        bool first = true;
        double px = 0, py = 0;
        for (auto const& pt : multi_pt.points)
        {
            double x = pt.x;
            double y = pt.y;
            if (topo_.tr)
            {
                x = (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                y = (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
            }
            if (first)
            {
                first = false;
                bbox.init(x,y,x,y);
            }
            else
            {
                bbox.expand_to_include(x,y);
            }
        }
        return bbox;
    }

    box2d<double> operator() (mapnik::topojson::linestring const& line) const
    {
        box2d<double> bbox;
        bool first = true;
        if (num_arcs_ > 0)
        {
            for (auto index : line.rings)
            {
                index_type arc_index = index < 0 ? std::abs(index) - 1 : index;
                if (arc_index >= 0 && arc_index < static_cast<int>(num_arcs_))
                {

                    double px = 0, py = 0;
                    auto const& arcs = topo_.arcs[arc_index];
                    for (auto pt : arcs.coordinates)
                    {
                        double x = pt.x;
                        double y = pt.y;
                        if (topo_.tr)
                        {
                            x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                            y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                        }
                        if (first)
                        {
                            first = false;
                            bbox.init(x, y, x, y);
                        }
                        else
                        {
                            bbox.expand_to_include(x, y);
                        }
                    }
                }
            }
        }
        return bbox;
    }

    box2d<double> operator() (mapnik::topojson::multi_linestring const& multi_line) const
    {
        box2d<double> bbox;
        if (num_arcs_ > 0)
        {
            bool first = true;
            for (auto const& line : multi_line.lines)
            {
                for (auto index : line)
                {
                    index_type arc_index = index < 0 ? std::abs(index) - 1 : index;
                    if (arc_index >= 0 && arc_index < static_cast<int>(num_arcs_))
                    {
                        double px = 0, py = 0;
                        auto const& arcs = topo_.arcs[arc_index];
                        for (auto pt : arcs.coordinates)
                        {
                            double x = pt.x;
                            double y = pt.y;
                            if (topo_.tr)
                            {
                                x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                                y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                            }
                            if (first)
                            {
                                first = false;
                                bbox.init(x, y, x, y);
                            }
                            else
                            {
                                bbox.expand_to_include(x, y);
                            }
                        }
                    }
                }
            }
        }
        return bbox;
    }

    box2d<double> operator() (mapnik::topojson::polygon const& poly) const
    {
        box2d<double> bbox;
        if (num_arcs_ > 0)
        {
            bool first = true;
            for (auto const& ring : poly.rings)
            {
                for (auto index : ring)
                {
                    index_type arc_index = index < 0 ? std::abs(index) - 1 : index;
                    if (arc_index >= 0 && arc_index < static_cast<int>(num_arcs_))
                    {
                        double px = 0, py = 0;
                        auto const& arcs = topo_.arcs[arc_index];
                        for (auto const& pt : arcs.coordinates)
                        {
                            double x = pt.x;
                            double y = pt.y;

                            if (topo_.tr)
                            {
                                x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                                y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                            }

                            if (first)
                            {
                                first = false;
                                bbox.init( x, y, x, y);
                            }
                            else
                            {
                                bbox.expand_to_include(x, y);
                            }
                        }
                    }
                }
            }
        }
        return bbox;
    }

    box2d<double> operator() (mapnik::topojson::multi_polygon const& multi_poly) const
    {
        box2d<double> bbox;
        if (num_arcs_ > 0)
        {
            bool first = true;
            for (auto const& poly : multi_poly.polygons)
            {
                for (auto const& ring : poly)
                {
                    for (auto index : ring)
                    {
                        index_type arc_index = index < 0 ? std::abs(index) - 1 : index;
                        if (arc_index >= 0 && arc_index < static_cast<int>(num_arcs_))
                        {
                            double px = 0, py = 0;
                            auto const& arcs = topo_.arcs[arc_index];
                            for (auto const& pt : arcs.coordinates)
                            {
                                double x = pt.x;
                                double y = pt.y;

                                if (topo_.tr)
                                {
                                    x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                                    y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                                }

                                if (first)
                                {
                                    first = false;
                                    bbox.init( x, y, x, y);
                                }
                                else
                                {
                                    bbox.expand_to_include(x, y);
                                }
                            }
                        }
                    }
                }
            }
        }
        return bbox;
    }
private:
    topology const& topo_;
    std::size_t num_arcs_;
};

namespace {

template <typename T>
void assign_properties(mapnik::feature_impl & feature, T const& geom, mapnik::transcoder const& tr)
{
    if ( geom.props)
    {
        for (auto const& p : *geom.props)
        {
            feature.put_new(std::get<0>(p), mapnik::util::apply_visitor(mapnik::json::attribute_value_visitor(tr),std::get<1>(p)));
        }
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
            multi_point.emplace_back(x, y);
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
            mapnik::geometry::line_string<double> line_string;

            for (auto index : line.rings)
            {
                index_type arc_index = index < 0 ? std::abs(index) - 1 : index;
                if (arc_index >= 0 && arc_index < static_cast<int>(num_arcs_))
                {
                    auto const& arcs = topo_.arcs[arc_index];
                    double px = 0, py = 0;
                    line_string.reserve(line_string.size() + arcs.coordinates.size());
                    for (auto pt : arcs.coordinates)
                    {
                        double x = pt.x;
                        double y = pt.y;
                        if (topo_.tr)
                        {
                            x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                            y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                        }
                        line_string.emplace_back(x,y);
                    }
                }
            }
            feature->set_geometry(std::move(line_string));
            assign_properties(*feature, line, tr_);
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
            for (auto const& line : multi_line.lines)
            {
                multi_line_string.reserve(multi_line_string.size() + line.size());
                mapnik::geometry::line_string<double> line_string;
                for (auto index : line)
                {
                    index_type arc_index = index < 0 ? std::abs(index) - 1 : index;
                    if (arc_index >= 0 && arc_index < static_cast<int>(num_arcs_))
                    {
                        hit = true;
                        double px = 0, py = 0;
                        auto const& arcs = topo_.arcs[arc_index];
                        line_string.reserve(line_string.size() + arcs.coordinates.size());
                        for (auto pt : arcs.coordinates)
                        {
                            double x = pt.x;
                            double y = pt.y;
                            if (topo_.tr)
                            {
                                x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                                y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                            }
                            line_string.emplace_back(x, y);
                        }

                    }
                }
                multi_line_string.push_back(std::move(line_string));
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
            polygon.reserve(poly.rings.size());
            bool hit = false;
            for (auto const& ring : poly.rings)
            {
                mapnik::geometry::linear_ring<double> linear_ring;
                for (auto const& index : ring)
                {
                    double px = 0, py = 0;
                    bool reverse = index < 0;
                    index_type arc_index = reverse ? std::abs(index) - 1 : index;
                    if (arc_index >= 0 && arc_index < static_cast<int>(num_arcs_))
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
                        linear_ring.reserve(linear_ring.size() + processed_coords.size());
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
                polygon.push_back(std::move(linear_ring));
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
                mapnik::geometry::polygon<double> polygon;
                polygon.reserve(poly.size());

                for (auto const& ring : poly)
                {
                    mapnik::geometry::linear_ring<double> linear_ring;
                    for (auto const& index : ring)
                    {
                        double px = 0, py = 0;
                        bool reverse = index < 0;
                        index_type arc_index = reverse ? std::abs(index) - 1 : index;
                        if (arc_index >= 0 && arc_index < static_cast<int>(num_arcs_))
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

                            using namespace boost::adaptors;
                            linear_ring.reserve(linear_ring.size() + processed_coords.size());
                            if (reverse)
                            {
                                for (auto const& c : (processed_coords | reversed))
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
                    polygon.push_back(std::move(linear_ring));
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

#endif //MAPNIK_TOPOJSON_UTILS_HPP
