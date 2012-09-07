/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#ifndef MAPNIK_PYTHON_BINDING_GRID_UTILS_INCLUDED
#define MAPNIK_PYTHON_BINDING_GRID_UTILS_INCLUDED

// boost
#include <boost/python.hpp>

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/grid/grid.hpp>

namespace mapnik {


template <typename T>
void grid2utf(T const& grid_type,
                     boost::python::list& l,
                     std::vector<typename T::lookup_type>& key_order);


template <typename T>
void grid2utf(T const& grid_type,
                     boost::python::list& l,
                     std::vector<typename T::lookup_type>& key_order,
                     unsigned int resolution);


template <typename T>
void grid2utf2(T const& grid_type,
                      boost::python::list& l,
                      std::vector<typename T::lookup_type>& key_order,
                      unsigned int resolution);


template <typename T>
void write_features(T const& grid_type,
                           boost::python::dict& feature_data,
                           std::vector<typename T::lookup_type> const& key_order);

template <typename T>
void grid_encode_utf(T const& grid_type,
                            boost::python::dict & json,
                            bool add_features,
                            unsigned int resolution);

template <typename T>
boost::python::dict grid_encode( T const& grid, std::string const& format, bool add_features, unsigned int resolution);

/* new approach: key comes from grid object
 * grid size should be same as the map
 * encoding, resizing handled as method on grid object
 * whether features are dumped is determined by argument not 'fields'
 */
void render_layer_for_grid(const mapnik::Map& map,
                                  mapnik::grid& grid,
                                  unsigned layer_idx, // TODO - layer by name or index
                                  boost::python::list const& fields);

/* old, original impl - to be removed after further testing
 * grid object is created on the fly at potentially reduced size
 */
boost::python::dict render_grid(const mapnik::Map& map,
                                       unsigned layer_idx, // layer
                                       std::string const& key, // key_name
                                       unsigned int step, // resolution
                                       boost::python::list const& fields);
}

#endif // MAPNIK_PYTHON_BINDING_GRID_UTILS_INCLUDED
