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
//$Id: mapnik_envelope.cc 27 2005-03-30 21:45:40Z pavlenko $

#include <boost/python.hpp>
#include <mapnik.hpp>


using mapnik::coord;
using mapnik::Envelope;

struct envelope_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const Envelope<double>& e)
    {
        using namespace boost::python;
        return boost::python::make_tuple(e.minx(),e.miny(),e.maxx(),e.maxy());
    }
};

//define overloads here
void (Envelope<double>::*width_p1)(double) = &Envelope<double>::width;
double (Envelope<double>::*width_p2)() const = &Envelope<double>::width;

void (Envelope<double>::*height_p1)(double) = &Envelope<double>::height;
double (Envelope<double>::*height_p2)() const = &Envelope<double>::height;

void (Envelope<double>::*expand_to_include_p1)(double,double) = &Envelope<double>::expand_to_include;
void (Envelope<double>::*expand_to_include_p2)(coord<double,2> const& ) = &Envelope<double>::expand_to_include;
void (Envelope<double>::*expand_to_include_p3)(Envelope<double> const& ) = &Envelope<double>::expand_to_include;

bool (Envelope<double>::*contains_p1)(double,double) const = &Envelope<double>::contains;
bool (Envelope<double>::*contains_p2)(coord<double,2> const&) const = &Envelope<double>::contains;
bool (Envelope<double>::*contains_p3)(Envelope<double> const&) const = &Envelope<double>::contains;

bool (Envelope<double>::*intersects_p1)(double,double) const = &Envelope<double>::intersects;
bool (Envelope<double>::*intersects_p2)(coord<double,2> const&) const = &Envelope<double>::intersects;
bool (Envelope<double>::*intersects_p3)(Envelope<double> const&) const = &Envelope<double>::intersects;


void export_envelope()
{
    using namespace boost::python;
    class_<Envelope<double> >("Envelope","A spacial envelope (i.e. bounding box) which also defines some basic operators.",init<double,double,double,double>())
        .def(init<>())
	.def(init<const coord<double,2>&, const coord<double,2>&>())
        .add_property("minx",&Envelope<double>::minx, "X coordinate for the lower left corner")
        .add_property("miny",&Envelope<double>::miny, "Y coordinate for the lower left corner")
        .add_property("maxx",&Envelope<double>::maxx, "X coordinate for the upper right corner")
        .add_property("maxy",&Envelope<double>::maxy, "Y coordinate for the upper right corner")
        .def("center",&Envelope<double>::center)
	.def("center",&Envelope<double>::re_center)
	.def("width",width_p1)
	.def("width",width_p2)
	.def("height",height_p1)
	.def("height",height_p2)
	.def("expand_to_include",expand_to_include_p1)
	.def("expand_to_include",expand_to_include_p2)
	.def("expand_to_include",expand_to_include_p3)
	.def("contains",contains_p1)
	.def("contains",contains_p2)
	.def("contains",contains_p3)
	.def("intersects",intersects_p1)
	.def("intersects",intersects_p2)
	.def("intersects",intersects_p3)
	.def(self == self)
        .def_pickle(envelope_pickle_suite())
        ;
}
