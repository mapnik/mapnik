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

#ifndef MAPNIK_UTIL_GEOBUF_HPP
#define MAPNIK_UTIL_GEOBUF_HPP

#include <mapnik/debug.hpp>
#include <mapnik/value.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/util/noncopyable.hpp>
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

template <typename FeatureCallback>
struct geobuf : mapnik::util::noncopyable
{
    using value_type = mapnik::util::variant<bool, int, double, std::string>;
    unsigned dim = 2;
    double precision = std::pow(10,6);
    bool is_topo = false;
    bool transformed = false;
    std::size_t lengths = 0;
    std::vector<std::string> keys_;
    std::vector<value_type> values_;
    mbgl::pbf pbf_;
    FeatureCallback & callback_;
    mapnik::context_ptr ctx_;
    const std::unique_ptr<mapnik::transcoder> tr_;
public:
    //ctor
    geobuf (unsigned char const* buf, std::size_t size, FeatureCallback & callback)
        : pbf_(buf, size),
          callback_(callback),
          ctx_(std::make_shared<mapnik::context_type>()),
          tr_(new mapnik::transcoder("utf8")) {}

    void read()
    {
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
                read_feature_collection(feature_collection);
                break;
            }
            case 6:
            {
                pbf_.message();
                MAPNIK_LOG_DEBUG(geobuf) << "standalone Geometry is not supported";
                break;
            }
            case 7:
            {
                pbf_.message();
                MAPNIK_LOG_DEBUG(geobuf) << "Topology is not supported";
                break;
            }
            default:
                MAPNIK_LOG_DEBUG(geobuf) << "FIXME tag=" << pbf_.tag;
                break;
            }
        }
    }

private:

    double transform(std::int64_t input)
    {
        return (transformed) ? (static_cast<double>(input)) : (input/precision);
    }

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
        std::uint8_t const* end = pbf.data + pbf.varint();
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

    template <typename T>
    void read_feature (T & pbf)
    {
        auto feature = feature_factory::create(ctx_,1);
        while (pbf.next())
        {
            switch (pbf.tag)
            {
            case 1:
            {
                auto message = pbf.message();
                auto geom = read_geometry(message);
                feature->set_geometry(std::move(geom));
                break;
            }
            case 11:
            {
                auto feature_id =  pbf.string();
                break;
            }
            case 12:
            {
                feature->set_id(pbf.template svarint<std::int64_t>());
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
                MAPNIK_LOG_DEBUG(geobuf) << "FAIL tag=" << pbf.tag;
                break;
            }
        }
        callback_(feature);
    }

    template <typename T>
    void read_feature_collection(T & pbf)
    {
        while (pbf.next())
        {
            switch (pbf.tag)
            {
            case 1:
            {
                auto message = pbf.message();
                read_feature(message);
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

    template <typename T>
    mapnik::geometry::point<double> read_point(T & pbf)
    {
        auto size = pbf.varint();
        std::uint8_t const* end = pbf.data + size;
        std::size_t count = 0;
        double x, y;
        while (pbf.data < end)
        {
            auto p = pbf.svarint();
            auto index = count % dim;
            if ( index == 0)
            {
                x = transform(p);
            }
            else if (index == 1)
            {
                y = transform(p);
            }
            ++count;
        }
        return mapnik::geometry::point<double>(x, y);
    }

    template <typename T>
    mapnik::geometry::geometry<double> read_coords(T & pbf, geometry_type_e type,
                                                   boost::optional<std::vector<int>> const& lengths)
    {
        mapnik::geometry::geometry<double> geom = mapnik::geometry::geometry_empty();
        switch (type)
        {
        case Point:
        {
            geom = read_point(pbf);
            break;
        }
        case Polygon:
        {
            geom = read_polygon(pbf, lengths);
            break;
        }
        case LineString:
        {
            geom = read_line_string(pbf);
            break;
        }
        case MultiPoint:
        {
            geom = read_multi_point(pbf);
            break;
        }
        case MultiLineString:
        {
            geom = read_multi_linestring(pbf, lengths);
        }
        case MultiPolygon:
        {
            geom = read_multi_polygon(pbf, lengths);
            break;
        }
        case GeometryCollection:
        {
            throw std::runtime_error("GeometryCollection is not supported");
        }
        default:
        {
            break;
        }
        }
        return geom;
    }

    template <typename T>
    std::vector<int> read_lengths(T & pbf)
    {
        std::vector<int> lengths;
        auto size = pbf.varint();
        std::uint8_t const* end = pbf.data + size;
        while (pbf.data < end)
        {
            lengths.push_back(pbf.varint());
        }
        return lengths;
    }

    template <typename T, typename Ring>
    void read_linear_ring(T & pbf, int len, std::size_t size, Ring & ring, bool close = false)
    {
        int i = 0;
        double x = 0.0;
        double y = 0.0;
        ring.reserve(close ? size + 1 : size);
        std::uint8_t const* end = pbf.data + size;
        while ( (len > 0) ? i < len : pbf.data < end)
        {
            for (unsigned d = 0; d < dim; ++d)
            {
                std::int64_t delta = pbf.template svarint<std::int64_t>();
                if (d == 0) x += delta;
                else if (d == 1) y += delta;
            }
            ring.emplace_back(transform(x), transform(y));
            ++i;
        }
        if (close && !ring.empty())
        {
            ring.emplace_back(ring.front());
        }

    }

    template <typename T>
    mapnik::geometry::multi_point<double> read_multi_point(T & pbf)
    {
        mapnik::geometry::multi_point<double> multi_point;
        double x = 0.0;
        double y = 0.0;
        auto size = pbf.varint();
        std::uint8_t const* end = pbf.data + size;
        while (pbf.data < end)
        {
            for (unsigned d = 0; d < dim; ++d)
            {
                if (d == 0) x += pbf.svarint();
                else if (d == 1) y += pbf.svarint();
            }
            mapnik::geometry::point<double> pt(transform(x), transform(y));
            multi_point.push_back(std::move(pt));
        }
        return multi_point;
    }

    template <typename T>
    mapnik::geometry::line_string<double> read_line_string(T & pbf)
    {
        mapnik::geometry::line_string<double> line;
        auto size = pbf.varint();
        read_linear_ring(pbf, 0, size, line);
        return line;

    }

    template <typename T>
    mapnik::geometry::multi_line_string<double> read_multi_linestring(T & pbf, boost::optional<std::vector<int>> const& lengths)
    {
        mapnik::geometry::multi_line_string<double> multi_line;
        multi_line.reserve(!lengths ? 1 : lengths->size());
        auto size = pbf.varint();
        if (!lengths)
        {
            mapnik::geometry::line_string<double> line;
            read_linear_ring(pbf, 0, size, line);
            multi_line.push_back(std::move(line));
        }
        else
        {
            for (auto len : *lengths)
            {
                mapnik::geometry::line_string<double> line;
                read_linear_ring(pbf, len, size, line);
                multi_line.push_back(std::move(line));
            }
        }
        return multi_line;
    }

    template <typename T>
    mapnik::geometry::polygon<double> read_polygon(T & pbf, boost::optional<std::vector<int>> const& lengths)
    {
        mapnik::geometry::polygon<double> poly;
        poly.reserve(!lengths ? 1 : lengths->size());
        auto size = pbf.varint();

        if (!lengths)
        {
            mapnik::geometry::linear_ring<double> ring;
            read_linear_ring(pbf, 0, size, ring, true);
            poly.push_back(std::move(ring));
        }
        else
        {
            for (auto len : *lengths)
            {
                mapnik::geometry::linear_ring<double> ring;
                read_linear_ring(pbf, len, size, ring, true);
                poly.push_back(std::move(ring));
            }
        }
        return poly;
    }

    template <typename T>
    mapnik::geometry::multi_polygon<double> read_multi_polygon(T & pbf, boost::optional<std::vector<int>> const& lengths)
    {
        mapnik::geometry::multi_polygon<double> multi_poly;
        auto size = pbf.varint();
        if (!lengths)
        {
            auto poly = read_polygon(pbf, lengths);
            multi_poly.push_back(std::move(poly));
        }
        else if ((*lengths).size() > 0)
        {
            int j = 1;
            for (int i = 0; i < (*lengths)[0]; ++i)
            {
                mapnik::geometry::polygon<double> poly;
                for (int k = 0; k < (*lengths)[j]; ++k)
                {
                    mapnik::geometry::linear_ring<double> ring;
                    read_linear_ring(pbf, (*lengths)[j + 1 + k], size, ring, true);
                    poly.push_back(std::move(ring));
                }
                multi_poly.push_back(std::move(poly));
                j += (*lengths)[j] + 1;
            }
        }
        return multi_poly;
    }

    template <typename T>
    mapnik::geometry::geometry<double> read_geometry(T & pbf)
    {
        mapnik::geometry::geometry<double> geom = mapnik::geometry::geometry_empty();
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
            {
                lengths = std::move(read_lengths(pbf));
                break;
            }
            case 3:
            {
                geom = std::move(read_coords(pbf, type, lengths));
                break;
            }
            case 4:
            {
                MAPNIK_LOG_DEBUG(geobuf) << "fixme read geometry=?";
                break;
            }
            case 11:
            {
                auto geom_id = pbf.string();
                MAPNIK_LOG_DEBUG(geobuf) << "geometry id's are not supported geom_id=" << geom_id ;
                break;
            }
            case 12:
            {
                auto geom_id = pbf.template varint<std::int64_t>();
                MAPNIK_LOG_DEBUG(geobuf) << "geometry id's are not supported geom_id=" << geom_id ;
                break;
            }
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
        return geom;
    }
};

}}

#endif // MAPNIK_UTIL_GEOBUF_HPP
