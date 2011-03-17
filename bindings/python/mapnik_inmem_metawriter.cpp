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
//$Id$

// boost
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>

// mapnik
#include <mapnik/metawriter_inmem.hpp>

using mapnik::metawriter_inmem;
using mapnik::metawriter_inmem_ptr;

namespace {
std::map<std::string, mapnik::value>::const_iterator 
mapnik_value_map_begin(const std::map<std::string, mapnik::value> &m) {
  return m.begin();
}

std::map<std::string, mapnik::value>::const_iterator 
mapnik_value_map_end(const std::map<std::string, mapnik::value> &m) {
  return m.end();
}
}

void export_inmem_metawriter() {
  using namespace boost::python;

  class_<std::map<std::string, mapnik::value> >
    ("MapnikProperties", "Retarded.", init<>())
    .def("__iter__", range(&mapnik_value_map_begin, &mapnik_value_map_end))
    ;

  class_<metawriter_inmem::meta_instance>
    ("MetaInstance", "Single rendered instance of meta-information.", no_init)
    .def_readonly("box", &metawriter_inmem::meta_instance::box)
    .def_readonly("properties", &metawriter_inmem::meta_instance::properties)
    ;

  class_<metawriter_inmem, metawriter_inmem_ptr, boost::noncopyable>
    ("MetaWriterInMem", 
     "Collects meta-information about elements rendered.",
     no_init)
    .def("__iter__", range(&metawriter_inmem::inst_begin,
                           &metawriter_inmem::inst_end))
    ;
}
