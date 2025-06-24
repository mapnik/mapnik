/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <optional>

#include <protozero/pbf_reader.hpp>

namespace mapnik {
namespace util {

enum geometry_type_e {
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
    value_visitor(feature_impl& feature, transcoder const& tr, std::string const& name)
        : feature_(feature)
        , tr_(tr)
        , name_(name)
    {}

    void operator()(std::string const& val) // unicode
    {
        feature_.put_new(name_, tr_.transcode(val.c_str()));
    }

    template<typename T>
    void operator()(T val)
    {
        feature_.put_new(name_, val);
    }

    feature_impl& feature_;
    transcoder const& tr_;
    std::string const& name_;
};
} // namespace detail

template<typename FeatureCallback>
struct geobuf : util::noncopyable
{
    using value_type = util::variant<bool, int, double, std::string>;
    unsigned dim = 2;
    double precision = std::pow(10, 6);
    bool is_topo = false;
    bool transformed = false;
    std::size_t lengths = 0;
    std::vector<std::string> keys_;
    std::vector<value_type> values_;
    protozero::pbf_reader reader_;
    FeatureCallback& callback_;
    context_ptr ctx_;
    const std::unique_ptr<transcoder> tr_;

  public:
    // ctor
    geobuf(char const* buf, std::size_t size, FeatureCallback& callback)
        : reader_(buf, size)
        , callback_(callback)
        , ctx_(std::make_shared<context_type>())
        , tr_(new transcoder("utf8"))
    {}

    void read()
    {
        while (reader_.next())
        {
            switch (reader_.tag())
            {
                case 1: // keys
                {
                    keys_.push_back(reader_.get_string());
                    break;
                }
                case 2: {
                    dim = reader_.get_uint32();
                    break;
                }
                case 3: {
                    precision = std::pow(10, reader_.get_uint32());
                    break;
                }
                case 4: {
                    auto feature_collection = reader_.get_message();
                    read_feature_collection(feature_collection);
                    break;
                }
                case 5: {
                    // standalone Feature
                    auto message = reader_.get_message();
                    read_feature(message);
                    break;
                }
                case 6: {
                    // standalone Geometry
                    auto feature = feature_factory::create(ctx_, 1);
                    auto message = reader_.get_message();
                    feature->set_geometry(std::move(read_geometry(message)));
                    callback_(feature);
                    break;
                }
                default:
                    MAPNIK_LOG_DEBUG(geobuf) << "Unsupported tag=" << reader_.tag();
                    reader_.skip();
                    break;
            }
        }
    }

  private:

    double transform(std::int64_t input) { return (transformed) ? (static_cast<double>(input)) : (input / precision); }

    template<typename T>
    void read_value(T& reader)
    {
        while (reader.next())
        {
            switch (reader.tag())
            {
                case 1: {
                    values_.emplace_back(reader.get_string());
                    break;
                }
                case 2: {
                    values_.emplace_back(reader.get_double());
                    break;
                }
                case 3: {
                    values_.emplace_back(static_cast<int>(reader.get_uint32()));
                    break;
                }
                case 4: {
                    values_.emplace_back(-static_cast<int>(reader.get_uint32()));
                    break;
                }
                case 5: {
                    values_.emplace_back(reader.get_bool());
                    break;
                }
                case 6: {
                    values_.emplace_back(reader.get_string()); // JSON value
                    break;
                }
                default:
                    break;
            }
        }
    }

    template<typename T, typename Feature>
    void read_props(T& reader, Feature& feature)
    {
        auto pi = reader.get_packed_uint32();
        for (auto it = pi.first; it != pi.second; ++it)
        {
            auto key_index = *it++;
            auto value_index = *it;
            assert(key_index < keys_.size());
            assert(value_index < values_.size());
            std::string const& name = keys_[key_index];
            util::apply_visitor(detail::value_visitor(feature, *tr_, name), values_[value_index]);
        }
        values_.clear();
    }

    template<typename T>
    void read_feature(T& reader)
    {
        auto feature = feature_factory::create(ctx_, 1);
        while (reader.next())
        {
            switch (reader.tag())
            {
                case 1: {
                    auto message = reader.get_message();
                    auto geom = read_geometry(message);
                    feature->set_geometry(std::move(geom));
                    break;
                }
                case 11: {
                    auto feature_id = reader.get_string();
                    break;
                }
                case 12: {
                    feature->set_id(reader.get_sint64());
                    break;
                }
                case 13: {
                    auto message = reader.get_message();
                    read_value(message);
                    break;
                }
                case 14: {
                    // feature props
                    read_props(reader, *feature);
                    break;
                }
                case 15: {
                    // generic props
                    read_props(reader, *feature);
                    break;
                }
                default:
                    MAPNIK_LOG_DEBUG(geobuf) << "Unsupported tag=" << reader.tag();
                    break;
            }
        }
        callback_(feature);
    }

    template<typename T>
    void read_feature_collection(T& reader)
    {
        while (reader.next())
        {
            switch (reader.tag())
            {
                case 1: {
                    auto message = reader.get_message();
                    read_feature(message);
                    break;
                }
                case 13: {
                    auto message = reader.get_message();
                    read_value(message);
                    break;
                }
                default: {
                    reader.skip();
                    break;
                }
            }
        }
    }

    template<typename T>
    geometry::point<double> read_point(T& reader)
    {
        double x = 0.0;
        double y = 0.0;
        auto pi = reader.get_packed_sint64();
        std::size_t count = 0;
        for (auto it = pi.first; it != pi.second; ++it, ++count)
        {
            if (count == 0)
                x = transform(*it);
            else if (count == 1)
                y = transform(*it);
        }
        return geometry::point<double>(x, y);
    }

    template<typename T>
    geometry::geometry<double>
      read_coords(T& reader, geometry_type_e type, std::optional<std::vector<std::uint32_t>> const& lengths)
    {
        geometry::geometry<double> geom = geometry::geometry_empty();
        switch (type)
        {
            case Point: {
                geom = read_point(reader);
                break;
            }
            case Polygon: {
                geom = read_polygon(reader, lengths);
                break;
            }
            case LineString: {
                geom = read_line_string(reader);
                break;
            }
            case MultiPoint: {
                geom = read_multi_point(reader);
                break;
            }
            case MultiLineString: {
                geom = read_multi_linestring(reader, lengths);
                break;
            }
            case MultiPolygon: {
                geom = read_multi_polygon(reader, lengths);
                break;
            }
            default: {
                reader.skip();
                break;
            }
        }
        return geom;
    }

    template<typename T>
    std::vector<std::uint32_t> read_lengths(T& reader)
    {
        std::vector<std::uint32_t> lengths;
        auto pi = reader.get_packed_uint32();
        for (auto it = pi.first; it != pi.second; ++it)
        {
            lengths.push_back(*it);
        }
        return lengths;
    }

    template<typename T, typename Iterator, typename Ring>
    void read_linear_ring(T& reader, Iterator begin, Iterator end, Ring& ring, bool close = false)
    {
        double x = 0.0;
        double y = 0.0;
        std::size_t count = 0;
        for (auto it = begin; it != end; ++it)
        {
            std::int64_t delta = *it;
            auto d = count % dim;
            if (d == 0)
                x += delta;
            else if (d == 1)
            {
                y += delta;
                ring.emplace_back(transform(x), transform(y));
            }
            // we're only interested in X and Y, ignoring any extra coordinates
            ++count;
        }
        if (close && !ring.empty())
        {
            ring.emplace_back(ring.front());
        }
    }

    template<typename T>
    geometry::multi_point<double> read_multi_point(T& reader)
    {
        geometry::multi_point<double> multi_point;
        double x = 0.0;
        double y = 0.0;
        auto pi = reader.get_packed_sint64();
        std::size_t count = 0;
        for (auto it = pi.first; it != pi.second; ++it)
        {
            auto delta = *it;
            auto d = count % dim;
            if (d == 0)
                x += delta;
            else if (d == 1)
            {
                y += delta;
                geometry::point<double> pt(transform(x), transform(y));
                multi_point.push_back(std::move(pt));
            }
            ++count;
        }
        return multi_point;
    }

    template<typename T>
    geometry::line_string<double> read_line_string(T& reader)
    {
        geometry::line_string<double> line;
        auto pi = reader.get_packed_sint64();
        line.reserve(pi.size());
        read_linear_ring(reader, pi.first, pi.second, line);
        return line;
    }

    template<typename T>
    geometry::multi_line_string<double> read_multi_linestring(T& reader,
                                                              std::optional<std::vector<std::uint32_t>> const& lengths)
    {
        geometry::multi_line_string<double> multi_line;
        multi_line.reserve(!lengths ? 1 : lengths->size());
        auto pi = reader.get_packed_sint64();
        if (!lengths)
        {
            geometry::line_string<double> line;
            line.reserve(pi.size());
            read_linear_ring(reader, pi.first, pi.second, line);
            multi_line.push_back(std::move(line));
        }
        else
        {
            for (auto len : *lengths)
            {
                geometry::line_string<double> line;
                line.reserve(len);
                read_linear_ring(reader, pi.first, std::next(pi.first, dim * len), line);
                multi_line.push_back(std::move(line));
                std::advance(pi.first, dim * len);
            }
        }
        return multi_line;
    }

    template<typename T>
    geometry::polygon<double> read_polygon(T& reader, std::optional<std::vector<std::uint32_t>> const& lengths)
    {
        geometry::polygon<double> poly;
        poly.reserve(!lengths ? 1 : lengths->size());
        auto pi = reader.get_packed_sint64();
        if (!lengths)
        {
            geometry::linear_ring<double> ring;
            ring.reserve(pi.size());
            read_linear_ring(reader, pi.first, pi.second, ring, true);
            poly.push_back(std::move(ring));
        }
        else
        {
            for (auto len : *lengths)
            {
                geometry::linear_ring<double> ring;
                ring.reserve(len);
                read_linear_ring(reader, pi.first, std::next(pi.first, dim * len), ring, true);
                poly.push_back(std::move(ring));
                std::advance(pi.first, dim * len);
            }
        }
        return poly;
    }

    template<typename T>
    geometry::multi_polygon<double> read_multi_polygon(T& reader,
                                                       std::optional<std::vector<std::uint32_t>> const& lengths)
    {
        geometry::multi_polygon<double> multi_poly;
        if (!lengths)
        {
            auto poly = read_polygon(reader, lengths);
            multi_poly.push_back(std::move(poly));
        }
        else if ((*lengths).size() > 0)
        {
            int j = 1;
            auto pi = reader.get_packed_sint64();
            for (std::size_t i = 0; i < (*lengths)[0]; ++i)
            {
                geometry::polygon<double> poly;
                for (std::size_t k = 0; k < (*lengths)[j]; ++k)
                {
                    geometry::linear_ring<double> ring;
                    std::size_t len = (*lengths)[j + k + 1];
                    ring.reserve(len);
                    read_linear_ring(reader, pi.first, std::next(pi.first, len * dim), ring, true);
                    poly.push_back(std::move(ring));
                    std::advance(pi.first, len * dim);
                }
                multi_poly.push_back(std::move(poly));
                j += (*lengths)[j] + 1;
            }
        }
        return multi_poly;
    }

    template<typename T>
    geometry::geometry<double> read_geometry(T& reader)
    {
        geometry::geometry<double> geom = geometry::geometry_empty();
        geometry_type_e type = Unknown;
        std::optional<std::vector<std::uint32_t>> lengths;
        while (reader.next())
        {
            switch (reader.tag())
            {
                case 1: // type
                {
                    type = static_cast<geometry_type_e>(reader.get_uint32());
                    break;
                }
                case 2: {
                    auto val = read_lengths(reader);
                    if (!val.empty())
                        lengths = std::move(val);
                    break;
                }
                case 3: {
                    geom = std::move(read_coords(reader, type, lengths));
                    break;
                }
                case 4: {
                    if (geom.is<geometry::geometry_empty>())
                        geom = geometry::geometry_collection<double>();
                    auto& collection = geom.get<geometry::geometry_collection<double>>();
                    auto message = reader.get_message();
                    collection.push_back(std::move(read_geometry(message)));
                    break;
                }
                case 13: {
                    auto value = reader.get_message();
                    read_value(value);
                    break;
                }
                default: {
                    reader.skip();
                    break;
                }
            }
        }
        return geom;
    }
};

} // namespace util
} // namespace mapnik

#endif // MAPNIK_UTIL_GEOBUF_HPP
