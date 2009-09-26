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

// boost
#include <boost/version.hpp>
#include <boost/python.hpp>

// mapnik
#include <mapnik/envelope.hpp>

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

//intersects
bool (Envelope<double>::*intersects_p1)(double,double) const = &Envelope<double>::intersects;
bool (Envelope<double>::*intersects_p2)(coord<double,2> const&) const = &Envelope<double>::intersects;
bool (Envelope<double>::*intersects_p3)(Envelope<double> const&) const = &Envelope<double>::intersects;

// intersect
Envelope<double> (Envelope<double>::*intersect)(Envelope<double> const&) const = &Envelope<double>::intersect;

void export_envelope()
{
    using namespace boost::python;
    class_<Envelope<double> >("Envelope",
                              // class docstring is in mapnik/__init__.py, class _Coord
                              init<double,double,double,double>
                              (
#if BOOST_VERSION >= 103500
                               (arg("self"), arg("minx"), arg("miny"), arg("maxx"), arg("maxy")),
#endif
                               "Constructs a new envelope from the coordinates\n"
                               "of its lower left and upper right corner points."))
        .def(init<>
             (
#if BOOST_VERSION >= 103500
              (arg("self")), 
#endif
              "Equivalent to Envelope(0, 0, -1, -1)."))
        .def(init<const coord<double,2>&, const coord<double,2>&>
             (
#if BOOST_VERSION >= 103500
              (arg("self"), arg("ll"), arg("ur")),
#endif
              "Equivalent to Envelope(ll.x, ll.y, ur.x, ur.y)."))
        .add_property("minx", &Envelope<double>::minx, 
                      "X coordinate for the lower left corner")
        .add_property("miny", &Envelope<double>::miny, 
                      "Y coordinate for the lower left corner")
        .add_property("maxx", &Envelope<double>::maxx,
                      "X coordinate for the upper right corner")
        .add_property("maxy", &Envelope<double>::maxy, 
                      "Y coordinate for the upper right corner")
        .def("center", &Envelope<double>::center,
#if BOOST_VERSION >= 103500
             (arg("self")),
#endif
             "Returns the coordinates of the center of the bounding box.\n"
             "\n"
             "Example:\n"
             ">>> e = Envelope(0, 0, 100, 100)\n"
             ">>> e.center()\n"
             "Coord(50, 50)")
        .def("center", &Envelope<double>::re_center,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("x"), arg("y")),
#endif
             "Moves the envelope so that the given coordinates become its new center.\n"
             "The width and the height are preserved.\n"
             "\n "
             "Example:\n"
             ">>> e = Envelope(0, 0, 100, 100)\n"
             ">>> e.center(60, 60)\n"
             ">>> e.center()\n"
             "Coord(60.0,60.0)\n"
             ">>> (e.width(), e.height())\n"
             "(100.0, 100.0)\n"
             ">>> e\n"
             "Envelope(10.0, 10.0, 110.0, 110.0)"
             )
        .def("width", width_p1,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("new_width")),
#endif
             "Sets the width to new_width of the envelope preserving its center.\n"
             "\n "
             "Example:\n"
             ">>> e = Envelope(0, 0, 100, 100)\n"
             ">>> e.width(120)\n"
             ">>> e.center()\n"
             "Coord(50.0,50.0)\n"
             ">>> e\n"
             "Envelope(-10.0, 0.0, 110.0, 100.0)"
             )
        .def("width", width_p2,
#if BOOST_VERSION >= 103500
             (arg("self")),
#endif
             "Returns the width of this envelope."
             )
        .def("height", height_p1,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("new_height")),
#endif
             "Sets the height to new_height of the envelope preserving its center.\n"
             "\n "
             "Example:\n"
             ">>> e = Envelope(0, 0, 100, 100)\n"
             ">>> e.height(120)\n"
             ">>> e.center()\n"
             "Coord(50.0,50.0)\n"
             ">>> e\n"
             "Envelope(0.0, -10.0, 100.0, 110.0)"
             )
        .def("height", height_p2,
#if BOOST_VERSION >= 103500
             (arg("self")),
#endif
             "Returns the height of this envelope."
             )
        .def("expand_to_include",expand_to_include_p1,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("x"), arg("y")),
#endif
             "Expands this envelope to include the point given by x and y.\n"
             "\n"
             "Example:\n",
             ">>> e = Envelope(0, 0, 100, 100)\n"
             ">>> e.expand_to_include(110, 110)\n"
             ">>> e\n"
             "Envelope(0.0, 00.0, 110.0, 110.0)"
             )
        .def("expand_to_include",expand_to_include_p2,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("p")),
#endif
             "Equivalent to expand_to_include(p.x, p.y)"
             )
        .def("expand_to_include",expand_to_include_p3,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("other")),
#endif
             "Equivalent to:\n"
             "  expand_to_include(other.minx, other.miny)\n"
             "  expand_to_include(other.maxx, other.maxy)"
             )
        .def("contains",contains_p1,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("x"), arg("y")),
#endif
             "Returns True iff this envelope contains the point\n"
             "given by x and y."
             )
        .def("contains",contains_p2,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("p")),
#endif
             "Equivalent to contains(p.x, p.y)"
             )
        .def("contains",contains_p3,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("other")),
#endif
             "Equivalent to:\n"
             "  contains(other.minx, other.miny) and contains(other.maxx, other.maxy)"
             )
        .def("intersects",intersects_p1,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("x"), arg("y")),
#endif
             "Returns True iff this envelope intersects the point\n"
             "given by x and y.\n"
             "\n"
             "Note: For points, intersection is equivalent\n"
             "to containment, i.e. the following holds:\n"
             "   e.contains(x, y) == e.intersects(x, y)"
             )
        .def("intersects",intersects_p2,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("p")),
#endif
             "Equivalent to contains(p.x, p.y)")
        .def("intersects",intersects_p3,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("other")),
#endif
             "Returns True iff this envelope intersects the other envelope,\n"
             "This relationship is symmetric."
             "\n"
             "Example:\n"
             ">>> e1 = Envelope(0, 0, 100, 100)\n"
             ">>> e2 = Envelope(50, 50, 150, 150)\n"
             ">>> e1.intersects(e2)\n"
             "True\n"
             ">>> e1.contains(e2)\n"
             "False"
             )
        .def("intersect",intersect,
#if BOOST_VERSION >= 103500
             (arg("self"), arg("other")),
#endif
             "Returns the overlap of this envelope and the other envelope\n"
             "as a new envelope.\n"
             "\n"
             "Example:\n"
             ">>> e1 = Envelope(0, 0, 100, 100)\n"
             ">>> e2 = Envelope(50, 50, 150, 150)\n"
             ">>> e1.intersect(e2)\n"
             "Envelope(50.0, 50.0, 100.0, 100.0)"     
             )
        .def(self == self) // __eq__
        .def(self + self)  // __add__
        .def(self - self)  // __sub__
        .def(self * float()) // __mult__
        .def(float() * self) 
        .def(self / float()) // __div__
        .def_pickle(envelope_pickle_suite())
        ;
}
