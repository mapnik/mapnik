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

#include <mapnik.hh>
#include <boost/python.hpp>

using mapnik::coord;
using mapnik::Envelope;

struct envelope_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const Envelope<double>& e)
    {
        using namespace boost::python;
        return make_tuple(e.minx(),e.miny(),e.maxx(),e.maxy());
    }
};

void export_envelope()
{
    using namespace boost::python;
     class_<Envelope<double> >("envelope",init<double,double,double,double>())
        .def(init<>())
	 .def(init<const coord<double,2>&, const coord<double,2>&>())
        .add_property("minx",&Envelope<double>::minx)
        .add_property("miny",&Envelope<double>::miny)
        .add_property("maxx",&Envelope<double>::maxx)
        .add_property("maxy",&Envelope<double>::maxy)
        .def("center",&Envelope<double>::center)
        .def_pickle(envelope_pickle_suite())
        ;
}
