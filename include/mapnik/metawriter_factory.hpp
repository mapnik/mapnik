/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 MapQuest
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


#ifndef METAWRITER_FACTORY_HPP
#define METAWRITER_FACTORY_HPP

// mapnik
#include <mapnik/metawriter.hpp>

// boost
#include <boost/property_tree/ptree.hpp>

namespace mapnik {

/**
 * Creates a metawriter with the properties specified in the property
 * tree argument. Currently, this is hard-coded to the JSON and inmem
 * metawriters, but should provide an easy point to make them a
 * proper factory method if this is wanted in the future.
 */
metawriter_ptr metawriter_create(const boost::property_tree::ptree &pt);

/**
 * Writes properties into the given property tree representing the 
 * metawriter argument, and which can be used to reconstruct it.
 */
void metawriter_save(
  const metawriter_ptr &m, 
  boost::property_tree::ptree &pt,
  bool explicit_defaults);

}

#endif /* METAWRITER_FACTORY_HPP */

