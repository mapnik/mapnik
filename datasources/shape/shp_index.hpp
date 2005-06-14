/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef SHP_INDEX_HH
#define SHP_INDEX_HH

#include "mapnik.hh"
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
