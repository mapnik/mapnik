/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_UTIL_SPATIAL_INDEX_HPP
#define MAPNIK_UTIL_SPATIAL_INDEX_HPP

//mapnik
#include <mapnik/coord.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/query.hpp>
#include <mapnik/geom_util.hpp>
// stl
#include <type_traits>

using mapnik::box2d;
using mapnik::query;

namespace mapnik { namespace util {

template <typename Value, typename Filter, typename InputStream>
class spatial_index
{
public:
    static void query(Filter const& filter, InputStream& in,std::vector<Value>& pos);
    static box2d<double> bounding_box( InputStream& in );
    static void query_first_n(Filter const& filter, InputStream & in, std::vector<Value>& pos, std::size_t count);
private:

    spatial_index();
    ~spatial_index();
    spatial_index(spatial_index const&);
    spatial_index& operator=(spatial_index const&);
    static int read_ndr_integer(InputStream& in);
    static void read_envelope(InputStream& in, box2d<double>& envelope);
    static void query_node(Filter const& filter, InputStream& in, std::vector<Value> & results);
    static void query_first_n_impl(Filter const& filter, InputStream& in, std::vector<Value> & results, std::size_t count);
};

template <typename Value, typename Filter, typename InputStream>
box2d<double> spatial_index<Value, Filter, InputStream>::bounding_box(InputStream& in)
{
    static_assert(std::is_standard_layout<Value>::value, "Values stored in quad-tree must be standard layout type");
    in.seekg(16 + 4, std::ios::beg);
    box2d<double> box;
    read_envelope(in, box);
    in.seekg(0, std::ios::beg);
    return box;
}

template <typename Value, typename Filter, typename InputStream>
void spatial_index<Value, Filter, InputStream>::query(Filter const& filter, InputStream& in, std::vector<Value>& results)
{
    static_assert(std::is_standard_layout<Value>::value, "Values stored in quad-tree must be standard layout types");
    in.seekg(16, std::ios::beg);
    query_node(filter, in, results);
}

template <typename Value, typename Filter, typename InputStream>
void spatial_index<Value, Filter, InputStream>::query_node(Filter const& filter, InputStream& in, std::vector<Value>& results)
{
    int offset = read_ndr_integer(in);
    box2d<double> node_ext;
    read_envelope(in, node_ext);
    int num_shapes = read_ndr_integer(in);
    if (!filter.pass(node_ext))
    {
        in.seekg(offset + num_shapes * sizeof(Value) + 4, std::ios::cur);
        return;
    }

    for (int i = 0; i < num_shapes; ++i)
    {
        Value item;
        in.read(reinterpret_cast<char*>(&item), sizeof(Value));
        results.push_back(std::move(item));
    }

    int children = read_ndr_integer(in);
    for (int j = 0; j < children; ++j)
    {
        query_node(filter, in, results);
    }
}

template <typename Value, typename Filter, typename InputStream>
void spatial_index<Value, Filter, InputStream>::query_first_n(Filter const& filter, InputStream& in, std::vector<Value>& results, std::size_t count)
{
    static_assert(std::is_standard_layout<Value>::value, "Values stored in quad-tree must be standard layout types");
    in.seekg(16, std::ios::beg);
    query_first_n_impl(filter, in, results, count);
}

template <typename Value, typename Filter, typename InputStream>
void spatial_index<Value, Filter, InputStream>::query_first_n_impl(Filter const& filter, InputStream& in, std::vector<Value>& results, std::size_t count)
{
    if (results.size() == count) return;
    int offset = read_ndr_integer(in);
    box2d<double> node_ext;
    read_envelope(in, node_ext);
    int num_shapes = read_ndr_integer(in);
    if (!filter.pass(node_ext))
    {
        in.seekg(offset + num_shapes * sizeof(Value) + 4, std::ios::cur);
        return;
    }

    for (int i = 0; i < num_shapes; ++i)
    {
        Value item;
        in.read(reinterpret_cast<char*>(&item), sizeof(Value));
        if (results.size() < count) results.push_back(std::move(item));
    }
    int children = read_ndr_integer(in);
    for (int j = 0; j < children; ++j)
    {
        query_first_n_impl(filter, in, results, count);
    }
}

template <typename Value, typename Filter, typename InputStream>
int spatial_index<Value, Filter, InputStream>::read_ndr_integer(InputStream& in)
{
    char b[4];
    in.read(b, 4);
    return (b[0] & 0xff) | (b[1] & 0xff) << 8 | (b[2] & 0xff) << 16 | (b[3] & 0xff) << 24;
}

template <typename Value, typename Filter, typename InputStream>
void spatial_index<Value, Filter, InputStream>::read_envelope(InputStream& in, box2d<double>& envelope)
{
    in.read(reinterpret_cast<char*>(&envelope), sizeof(envelope));
}

}} // mapnik/util

#endif // MAPNIK_UTIL_SPATIAL_INDEX_HPP
