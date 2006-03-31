/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

#ifndef SHP_INDEX_HH
#define SHP_INDEX_HH

#include "envelope.hpp"
#include "query.hpp"
#include <fstream>
#include <set>

using namespace mapnik;

template <typename filterT>
class shp_index
{
public:
    static void query(const filterT& filter,std::ifstream& file,std::set<int>& pos);
private:
    shp_index();
    ~shp_index();
    shp_index(const shp_index&);
    shp_index& operator=(const shp_index&);
    static int read_ndr_integer(std::ifstream& in);
    static void read_envelope(std::ifstream& in,Envelope<double> &envelope);
    static void query_node(const filterT& filter,std::ifstream& file,std::set<int>& pos);
};

#endif //SHP_INDEX_HH
