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

#ifndef OGR_INDEX_HPP
#define OGR_INDEX_HPP

// stl
#include <fstream>
#include <vector>

// mapnik
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/query.hpp>

using mapnik::box2d;
using mapnik::query;

template<typename filterT, typename IStream = std::ifstream>
class ogr_index
{
  public:
    static void query(filterT const& filter, IStream& file, std::vector<int>& pos);

  private:
    ogr_index();
    ~ogr_index();
    ogr_index(ogr_index const&);
    ogr_index& operator=(ogr_index const&);
    static int read_ndr_integer(IStream& in);
    static void read_envelope(IStream& in, box2d<double>& envelope);
    static void query_node(filterT const& filter, IStream& in, std::vector<int>& pos);
};

template<typename filterT, typename IStream>
void ogr_index<filterT, IStream>::query(filterT const& filter, IStream& file, std::vector<int>& pos)
{
    file.seekg(16, std::ios::beg);
    query_node(filter, file, pos);
}

template<typename filterT, typename IStream>
void ogr_index<filterT, IStream>::query_node(filterT const& filter, IStream& file, std::vector<int>& ids)
{
    int const offset = read_ndr_integer(file);

    box2d<double> node_ext;
    read_envelope(file, node_ext);

    int const num_shapes = read_ndr_integer(file);

    if (!filter.pass(node_ext))
    {
        file.seekg(offset + num_shapes * 4 + 4, std::ios::cur);
        return;
    }

    for (int i = 0; i < num_shapes; ++i)
    {
        int const id = read_ndr_integer(file);
        ids.push_back(id);
    }

    int const children = read_ndr_integer(file);

    for (int j = 0; j < children; ++j)
    {
        query_node(filter, file, ids);
    }
}

template<typename filterT, typename IStream>
int ogr_index<filterT, IStream>::read_ndr_integer(IStream& file)
{
    char b[4];
    file.read(b, 4);
    return (b[0] & 0xff) | (b[1] & 0xff) << 8 | (b[2] & 0xff) << 16 | (b[3] & 0xff) << 24;
}

template<typename filterT, typename IStream>
void ogr_index<filterT, IStream>::read_envelope(IStream& file, box2d<double>& envelope)
{
    file.read(reinterpret_cast<char*>(&envelope), sizeof(envelope));
}

#endif // OGR_INDEX_HPP
