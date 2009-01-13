/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
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

#include <boost/python.hpp>

#include <mapnik/spatial.hpp>
#include <mapnik/logical.hpp>
#include <mapnik/comparison.hpp>
#include <mapnik/regex_filter.hpp>
#include <mapnik/filter.hpp>
#include <mapnik/filter_factory.hpp>

using mapnik::filter;
using mapnik::filter_ptr;
using mapnik::filter_factory;
using mapnik::Feature;
using mapnik::create_filter;

filter_ptr create_filter_(std::string const& wkt)
{
   return create_filter(wkt,"utf8");
}

void export_filter()
{
    using namespace boost::python;
    class_<filter<Feature>,boost::noncopyable>("Filter",
                                               "An expression which allows "
                                               "to select features.",no_init)
       .def("passes", &filter<Feature>::pass) // note: "pass" is a reserved word in Python
       .def("__str__",&filter<Feature>::to_string);
    ;
    
    def("Filter",&create_filter_);
}
