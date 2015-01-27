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

#ifndef MAPNIK_UTIL_GEOBUF_HPP
#define MAPNIK_UTIL_GEOBUF_HPP

#include <iostream>
#include <mapnik/value.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>

#include <cmath>
#include <cassert>
#include <vector>

#include <boost/optional.hpp>
#include "pbf.hpp"

namespace mapnik { namespace util {

enum geometry_type_e
{
    Unknown = -1,
    Point = 0,
    MultiPoint = 1,
    LineString = 2,
    MultiLineString = 3,
    Polygon = 4,
    MultiPolygon = 5,
    GeometryCollection = 6
};

namespace detail {
struct value_visitor
{
    value_visitor(mapnik::feature_impl & feature, mapnik::transcoder const& tr, std::string const& name)
        : feature_(feature),
          tr_(tr),
          name_(name) {}

    void operator() (std::string const& val) // unicode
    {
        feature_.put_new(name_, tr_.transcode(val.c_str()));
    }

    template <typename T>
    void operator() (T val)
    {
        feature_.put_new(name_, val);
    }

    mapnik::feature_impl & feature_;
    mapnik::transcoder const& tr_;
    std::string const& name_;
};
}

struct geobuf
{
    using value_type = mapnik::util::variant<bool, int, double, std::string>;
    unsigned dim = 2;
    unsigned precision = std::pow(10,6);
    bool is_topo = false;
    bool transformed = false;
    std::size_t lengths = 0;
    std::vector<std::string> keys_;
    std::vector<value_type> values_;
    mbgl::pbf pbf_;
    mapnik::context_ptr ctx_;
    const std::unique_ptr<mapnik::transcoder> tr_;
public:
    //ctor
    geobuf (unsigned char const* buf, std::size_t size)
     : pbf_(buf, size),
       ctx_(std::make_shared<mapnik::context_type>()),
       tr_(new mapnik::transcoder("utf8")) {}

    template <typename T>
    void read(T & features)
    {
        //using feature_container_type = T;
        while (pbf_.next())
        {
            switch (pbf_.tag)
            {
            case 1: // keys
            {
                keys_.push_back(pbf_.string());
                break;
            }
            case 2:
            {
                dim = pbf_.varint();
                break;
            }
            case 3:
            {
                precision = std::pow(10,pbf_.varint());
                break;
            }
            case 4:
            {
                auto feature_collection = pbf_.message();
                read_feature_collection(feature_collection, features);
                break;
            }
            case 6:
            {
                std::cerr << "FIXME" << std::endl;
                //auto geometry = pbf_.message();
                //read_geometry(geometry);
            }
            default:
                std::cerr << "FIXME tag=" << pbf_.tag << std::endl;
                break;
            }
        }
    }

private:

    template <typename T>
    void read_value(T & pbf)
    {
        while (pbf.next())
        {
            switch (pbf.tag)
            {
            case 1:
            {
                values_.emplace_back(pbf.string());
                break;
            }
            case 2:
            {
                values_.emplace_back(pbf.float64());
                break;
            }
            case 3:
            {
                values_.emplace_back(static_cast<int>(pbf.varint()));
                break;
            }
            case 4:
            {
                values_.emplace_back(-static_cast<int>(pbf.varint()));
                break;
            }
            case 5:
            {
                values_.emplace_back(pbf.boolean());
                break;
            }
            case 6:
            {
                values_.emplace_back(pbf.string()); // JSON value
                break;
            }
            default:
                break;
            }
        }
    }

    template <typename T, typename Feature>
    void read_props(T & pbf, Feature & feature)
    {
        uint8_t const* end = pbf.data + pbf.varint();
        while (pbf.data < end)
        {
            auto key_index = pbf.varint();
            auto value_index = pbf.varint();
            assert(key_index < keys_.size());
            assert(value_index < values_.size());
            std::string const& name = keys_[key_index];
            mapnik::util::apply_visitor(detail::value_visitor(feature, *tr_, name), values_[value_index]);
        }
        values_.clear();
    }

    template <typename T, typename Features>
    void read_feature (T & pbf, Features & features)
    {
        using feature_type = typename Features::value_type;
        feature_type feature(feature_factory::create(ctx_,1));
        while (pbf.next())
        {
            switch (pbf.tag)
            {
            case 1:
            {
                auto message = pbf.message();
                read_geometry(message, feature->paths());
                break;
            }
            case 11:
            {
                auto feature_id =  pbf.string();
                break;
            }
            case 12:
            {
                feature->set_id(pbf.svarint());
                break;
            }
            case 13:
            {
                auto message = pbf.message();
                read_value(message);
                break;
            }
            case 14:
            {
                // feature props
                read_props(pbf, *feature);
                break;
            }
            case 15:
            {
                // generic props
                read_props(pbf, *feature);
                break;
            }
            default:
                std::cerr << "FAIL tag=" << pbf.tag <<  std::endl;
                break;
            }
        }
        features.push_back(feature);
    }

    template <typename T, typename Features>
    void read_feature_collection(T & pbf, Features & features)
    {
        while (pbf.next())
        {
            switch (pbf.tag)
            {
            case 1:
            {
                auto message = pbf.message();
                read_feature(message, features);
                break;
            }
            case 13:
            {
                auto message = pbf.message();
                read_value(message);
                break;
            }
            case 15:
            {
                pbf.skipBytes(pbf.varint());
                break;
            }
            default:
                break;
            }
        }
    }

    template <typename T, typename GeometryContainer>
    void read_point(T & pbf, GeometryContainer & paths)
    {
        auto size = pbf.varint();
        std::unique_ptr<geometry_type> point(new geometry_type(mapnik::geometry_type::types::Point));

        uint8_t const* end = pbf.data + size;
        std::size_t count = 0;
        int x, y;
        while (pbf.data < end)
        {
            auto p = pbf.svarint();
            auto index = count % dim;
            if ( index == 0)
            {
                x = p;
            }
            else if (index == 1)
            {
                y = p;
                point->move_to(x, y);
            }
            ++count;
        }
        paths.push_back(point.release());
    }

    template <typename T, typename GeometryContainer>
    void read_coords(T & pbf, geometry_type_e type, boost::optional<std::vector<int>> const& lengths, GeometryContainer & paths)
    {
        switch (type)
        {
        case Point:
        {
            read_point(pbf, paths);
            break;
        }
        case Polygon:
        {
            read_polygon(pbf, lengths, paths);
            break;
        }
        case LineString:
        {
            read_line_string(pbf, paths);
            break;
        }
        case MultiPolygon:
        {
            read_multipolygon(pbf, lengths, paths);
            break;
        }
        default:
        {
            //std::cout << "read coord FAIL type=" << type << std::endl;
            break;
        }
        }
    }

    template <typename T>
    std::vector<int> read_lengths(T & pbf)
    {
        std::vector<int> lengths;
        auto size = pbf.varint();
        uint8_t const* end = pbf.data + size;
        while (pbf.data < end)
        {
            lengths.push_back(pbf.varint());
        }
        return lengths;
    }

    template <typename T, typename Geometry>
    void read_linear_ring(T & pbf, int len, std::size_t size, Geometry & geom, bool polygon = false)
    {
        int i = 0;
        int x = 0.0;
        int y = 0.0;
        uint8_t const* end = pbf.data + size;
        while ( (len > 0) ? i++ < len : pbf.data < end)
        {
            for (int d = 0; d < dim; ++d)
            {
                if (d == 0) x += pbf.svarint();
                else if (d == 1) y += pbf.svarint();
            }
            if (i == 1) geom.move_to(x, y);
            else geom.line_to(x, y);
        }
        if (polygon) geom.close_path();
    }

    template <typename T, typename GeometryContainer>
    void read_line_string(T & pbf, GeometryContainer & paths)
    {
        std::unique_ptr<geometry_type> line(new geometry_type(mapnik::geometry_type::types::LineString));
        auto size = pbf.varint();
        read_linear_ring(pbf, 0, size, *line);
        paths.push_back(line.release());
    }

    template <typename T, typename GeometryContainer>
    void read_polygon(T & pbf, boost::optional<std::vector<int>> const& lengths, GeometryContainer & paths)
    {
        std::unique_ptr<geometry_type> poly(new geometry_type(mapnik::geometry_type::types::Polygon));

        auto size = pbf.varint();
        if (!lengths)
        {
            read_linear_ring(pbf, 0, size, *poly, true);
        }
        else
        {
            for (auto len : *lengths)
            {
                read_linear_ring(pbf, len, size, *poly, true);
            }
        }
        paths.push_back(poly.release());
    }

    template <typename T, typename GeometryContainer>
    void read_multipolygon(T & pbf, boost::optional<std::vector<int>> const& lengths, GeometryContainer & paths)
    {
        auto size = pbf.varint();
        if (!lengths)
        {
            std::unique_ptr<geometry_type> poly(new geometry_type(mapnik::geometry_type::types::Polygon));
            read_linear_ring(pbf, 0, size, *poly, true);
            paths.push_back(poly.release());
        }
        else if ((*lengths).size() > 0)
        {
            int j = 1;
            for (int i = 0; i < (*lengths)[0]; ++i)
            {
                std::unique_ptr<geometry_type> poly(new geometry_type(mapnik::geometry_type::types::Polygon));
                for (int k = 0; k < (*lengths)[j]; ++k)
                {
                    read_linear_ring(pbf, (*lengths)[j + 1 + k], size, *poly, true);
                }
                paths.push_back(poly.release());
                j += (*lengths)[j] + 1;
            }
        }
    }

    template <typename T, typename GeometryContainer>
    void read_geometry(T & pbf, GeometryContainer & paths)
    {
        geometry_type_e type = Unknown;
        boost::optional<std::vector<int>> lengths;
        while (pbf.next())
        {
            switch (pbf.tag)
            {
            case 1: // type
            {
                type = static_cast<geometry_type_e>(pbf.varint());
                break;
            }
            case 2:
                lengths = std::move(read_lengths(pbf));
                break;
            case 3:
            {
                read_coords(pbf, type, lengths, paths);
                break;
            }
            case 4:
                std::cerr << "read geometry=?" << std::endl;
                break;
            case 11:
                pbf.string();
                std::cerr << "geom_id=" << pbf.string() << std::endl;
                break;
            case 12:
                pbf.varint();
                std::cerr << "geom_id=" << pbf.varint() << std::endl;
                break;
            case 13:
            {
                auto value = pbf.message();
                read_value(value);
                break;
            }
            case 14:
            case 15:
            {
                pbf.skipBytes(pbf.varint());
                break;
            }
            default:
                break;
            }
        }
    }
};

}}

#endif // MAPNIK_UTIL_GEOBUF_HPP
