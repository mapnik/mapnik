/* This file is part of python_mapnik (c++/python mapping toolkit)
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

//$Id$


#include <mapnik.hpp>
#include <boost/python.hpp>

using mapnik::filter;
using mapnik::filter_ptr;
using mapnik::filter_factory;
using mapnik::Feature;

namespace  
{
    using namespace boost::python;
    filter_ptr create_filter(string const& filter_text)
    {
	return filter_factory<Feature>::compile(filter_text);
    }
}

void export_filter()
{
    using namespace boost::python;
    class_<filter<Feature>,boost::noncopyable>("filter",no_init)
	.def("__str__",&filter<Feature>::to_string);
	;
    def("filter",&create_filter);
}
