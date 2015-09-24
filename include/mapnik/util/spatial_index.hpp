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

#ifndef MAPNIK_SPATIAL_INDEX_HPP
#define MAPNIK_SPATIAL_INDEX_HPP


#include <mapnik/coord.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/query.hpp>
#include <mapnik/geom_util.hpp>

#include <fstream>

using mapnik::box2d;
using mapnik::query;

namespace mapnik { namespace util {

template <typename Filter, typename InputStream>
class spatial_index
{
public:
    static void query(Filter const& filter, InputStream& file, std::string const& indexname, std::vector<std::pair<std::size_t, std::size_t>>& pos);
    static void query_node(const Filter& filter, InputStream& in, std::vector<std::pair<std::size_t, std::size_t>> & results);
    static box2d<double> bounding_box( InputStream& file );
private:
    spatial_index();
    ~spatial_index();
    spatial_index(const spatial_index&);
    spatial_index& operator=(const spatial_index&);
    static int read_ndr_integer(InputStream& in);
    static void read_envelope(InputStream& in, box2d<double>& envelope);
};

template <typename Filter, typename Stream>
struct query_node_op
{
    query_node_op(Filter const& filter, Stream & in, std::vector<std::pair<std::size_t, std::size_t>>& results)
        : filter_(filter),
          in_(in),
          results_(results) {}

    bool operator() ()
    {
        spatial_index<filter_in_box, Stream>::query_node(filter_, in_, results_);
    }

    Filter const& filter_;
    Stream & in_;
    std::vector<std::pair<std::size_t, std::size_t>> & results_;
};


template <typename Filter, typename InputStream>
box2d<double> spatial_index<Filter, InputStream>::bounding_box(InputStream& file)
{
    file.seekg(16 + 4, std::ios::beg);
    box2d<double> box;
    read_envelope(file, box);
    file.seekg(0, std::ios::beg);
    return box;
}

template <typename Filter, typename InputStream>
void spatial_index<Filter, InputStream>::query(Filter const& filter, InputStream& file,
                                            std::string const& indexname, std::vector<std::pair<std::size_t, std::size_t>>& results)
{
    file.seekg(16, std::ios::beg);
    int offset = read_ndr_integer(file);
    box2d<double> node_ext;
    read_envelope(file, node_ext);
    int num_shapes = read_ndr_integer(file);
    if (!filter.pass(node_ext))
    {
        file.seekg(offset + num_shapes * 4 + 4, std::ios::cur);
        return;
    }
    for (int i = 0; i < num_shapes; ++i)
    {
        std::pair<std::size_t, std::size_t> item;
        file.read(reinterpret_cast<char*>(&item), sizeof(item));
        results.push_back(item);
    }

    int children = read_ndr_integer(file);
    for (int j = 0; j < children; ++j)
    {
        query_node(filter, file, results);
    }
    query_node(filter, file, results);
}

template <typename Filter, typename InputStream>
void spatial_index<Filter, InputStream>::query_node(Filter const& filter, InputStream& file, std::vector<std::pair<std::size_t, std::size_t>>& ids)
{
    int offset = read_ndr_integer(file);
    box2d<double> node_ext;
    read_envelope(file, node_ext);
    int num_shapes = read_ndr_integer(file);
    if (! filter.pass(node_ext))
    {
        file.seekg(offset + num_shapes * 4 + 4, std::ios::cur);
        return;
    }

    for (int i = 0; i < num_shapes; ++i)
    {
        std::pair<std::size_t, std::size_t> item;
        //file >> item;
        //int id = read_ndr_integer(file);
        file.read(reinterpret_cast<char*>(&item), sizeof(item));
        ids.push_back(item);
    }

    int children = read_ndr_integer(file);
    for (int j = 0; j < children; ++j)
    {
        query_node(filter, file, ids);
    }
}

template <typename Filter, typename InputStream>
int spatial_index<Filter, InputStream>::read_ndr_integer(InputStream& file)
{
    char b[4];
    file.read(b, 4);
    return (b[0] & 0xff) | (b[1] & 0xff) << 8 | (b[2] & 0xff) << 16 | (b[3] & 0xff) << 24;
}

template <typename Filter, typename InputStream>
void spatial_index<Filter, InputStream>::read_envelope(InputStream& file, box2d<double>& envelope)
{
    file.read(reinterpret_cast<char*>(&envelope), sizeof(envelope));
}

}} // mapnik/util
