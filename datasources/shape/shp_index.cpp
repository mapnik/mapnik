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

#include "shp_index.hh"

template <typename filterT>
void shp_index<filterT>::query(const filterT& filter,std::ifstream& file,std::set<int>& pos)
{
    file.seekg(16,std::ios::beg);
    query_node(filter,file,pos);
}


template <typename filterT>
void shp_index<filterT>::query_node(const filterT& filter,std::ifstream& file,std::set<int>& ids)
{
    int offset=read_ndr_integer(file);

    Envelope<double> node_ext;
    read_envelope(file,node_ext);

    int num_shapes=read_ndr_integer(file);

    if (!filter.pass(node_ext))
    {
        file.seekg(offset+num_shapes*4+4,std::ios::cur);
        return;
    }

    for (int i=0;i<num_shapes;++i)
    {
        int id=read_ndr_integer(file);
        ids.insert(id);
    }

    int children=read_ndr_integer(file);

    for (int j=0;j<children;++j)
    {
        query_node(filter,file,ids);
    }
}


template <typename filterT>
int shp_index<filterT>::read_ndr_integer(std::ifstream& file)
{
    char b[4];
    file.read(b,4);
    return (b[0]&0xff) | (b[1]&0xff)<<8 | (b[2]&0xff)<<16 | (b[3]&0xff)<<24;
}


template <typename filterT>
void shp_index<filterT>::read_envelope(std::ifstream& file,Envelope<double>& envelope)
{
    file.read(reinterpret_cast<char*>(&envelope),sizeof(envelope));
}

template class shp_index<filter_in_box>;
template class shp_index<filter_at_point>;
