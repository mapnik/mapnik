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

// boost
#include <boost/python.hpp>

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/value_error.hpp>

using mapnik::coord;
using mapnik::box2d;

struct envelope_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const box2d<double>& e)
    {
        using namespace boost::python;
        return boost::python::make_tuple(e.minx(),e.miny(),e.maxx(),e.maxy());
    }
};

box2d<double> from_string(std::string const& s)
{
    box2d<double> bbox;
    bool success = bbox.from_string(s);
    if (success)
    {
        return bbox;
    }
    else
    {
        std::stringstream ss;
        ss << "Could not parse bbox from string: '" << s << "'";
        throw mapnik::value_error(ss.str());
    }
}

//define overloads here
void (box2d<double>::*width_p1)(double) = &box2d<double>::width;
double (box2d<double>::*width_p2)() const = &box2d<double>::width;

void (box2d<double>::*height_p1)(double) = &box2d<double>::height;
double (box2d<double>::*height_p2)() const = &box2d<double>::height;

void (box2d<double>::*expand_to_include_p1)(double,double) = &box2d<double>::expand_to_include;
void (box2d<double>::*expand_to_include_p2)(coord<double,2> const& ) = &box2d<double>::expand_to_include;
void (box2d<double>::*expand_to_include_p3)(box2d<double> const& ) = &box2d<double>::expand_to_include;

bool (box2d<double>::*contains_p1)(double,double) const = &box2d<double>::contains;
bool (box2d<double>::*contains_p2)(coord<double,2> const&) const = &box2d<double>::contains;
bool (box2d<double>::*contains_p3)(box2d<double> const&) const = &box2d<double>::contains;

//intersects
bool (box2d<double>::*intersects_p1)(double,double) const = &box2d<double>::intersects;
bool (box2d<double>::*intersects_p2)(coord<double,2> const&) const = &box2d<double>::intersects;
bool (box2d<double>::*intersects_p3)(box2d<double> const&) const = &box2d<double>::intersects;

// intersect
box2d<double> (box2d<double>::*intersect)(box2d<double> const&) const = &box2d<double>::intersect;

// re_center
void (box2d<double>::*re_center_p1)(double,double) = &box2d<double>::re_center;
void (box2d<double>::*re_center_p2)(coord<double,2> const& ) = &box2d<double>::re_center;

// clip
void (box2d<double>::*clip)(box2d<double> const&) = &box2d<double>::clip;

// deepcopy
box2d<double> box2d_deepcopy(box2d<double> & obj, boost::python::dict memo)
{
    // FIXME::ignore memo for now
    box2d<double> result(obj);
    return result;
}

void export_envelope()
{
    using namespace boost::python;
    class_<box2d<double> >("Box2d",
                           // class docstring is in mapnik/__init__.py, class _Coord
                           init<double,double,double,double>(
                               (arg("minx"),arg("miny"),arg("maxx"),arg("maxy")),
                               "Constructs a new envelope from the coordinates\n"
                               "of its lower left and upper right corner points.\n"))
        .def(init<>("Equivalent to Box2d(0, 0, -1, -1).\n"))
        .def(init<const coord<double,2>&, const coord<double,2>&>(
                 (arg("ll"),arg("ur")),
                 "Equivalent to Box2d(ll.x, ll.y, ur.x, ur.y).\n"))
        .def("from_string",from_string)
        .staticmethod("from_string")
        .add_property("minx", &box2d<double>::minx,
                      "X coordinate for the lower left corner")
        .add_property("miny", &box2d<double>::miny,
                      "Y coordinate for the lower left corner")
        .add_property("maxx", &box2d<double>::maxx,
                      "X coordinate for the upper right corner")
        .add_property("maxy", &box2d<double>::maxy,
                      "Y coordinate for the upper right corner")
        .def("center", &box2d<double>::center,
             "Returns the coordinates of the center of the bounding box.\n"
             "\n"
             "Example:\n"
             ">>> e = Box2d(0, 0, 100, 100)\n"
             ">>> e.center()\n"
             "Coord(50, 50)\n")
        .def("center", re_center_p1,
             (arg("x"), arg("y")),
             "Moves the envelope so that the given coordinates become its new center.\n"
             "The width and the height are preserved.\n"
             "\n "
             "Example:\n"
             ">>> e = Box2d(0, 0, 100, 100)\n"
             ">>> e.center(60, 60)\n"
             ">>> e.center()\n"
             "Coord(60.0,60.0)\n"
             ">>> (e.width(), e.height())\n"
             "(100.0, 100.0)\n"
             ">>> e\n"
             "Box2d(10.0, 10.0, 110.0, 110.0)\n"
            )
        .def("center", re_center_p2,
             (arg("Coord")),
             "Moves the envelope so that the given coordinates become its new center.\n"
             "The width and the height are preserved.\n"
             "\n "
             "Example:\n"
             ">>> e = Box2d(0, 0, 100, 100)\n"
             ">>> e.center(Coord60, 60)\n"
             ">>> e.center()\n"
             "Coord(60.0,60.0)\n"
             ">>> (e.width(), e.height())\n"
             "(100.0, 100.0)\n"
             ">>> e\n"
             "Box2d(10.0, 10.0, 110.0, 110.0)\n"
            )
        .def("clip", clip,
             (arg("other")),
             "Clip the envelope based on the bounds of another envelope.\n"
             "\n "
             "Example:\n"
             ">>> e = Box2d(0, 0, 100, 100)\n"
             ">>> e.center(Coord60, 60)\n"
             ">>> e.center()\n"
             "Coord(60.0,60.0)\n"
             ">>> (e.width(), e.height())\n"
             "(100.0, 100.0)\n"
             ">>> e\n"
             "Box2d(10.0, 10.0, 110.0, 110.0)\n"
            )
        .def("width", width_p1,
             (arg("new_width")),
             "Sets the width to new_width of the envelope preserving its center.\n"
             "\n "
             "Example:\n"
             ">>> e = Box2d(0, 0, 100, 100)\n"
             ">>> e.width(120)\n"
             ">>> e.center()\n"
             "Coord(50.0,50.0)\n"
             ">>> e\n"
             "Box2d(-10.0, 0.0, 110.0, 100.0)\n"
            )
        .def("width", width_p2,
             "Returns the width of this envelope.\n"
            )
        .def("height", height_p1,
             (arg("new_height")),
             "Sets the height to new_height of the envelope preserving its center.\n"
             "\n "
             "Example:\n"
             ">>> e = Box2d(0, 0, 100, 100)\n"
             ">>> e.height(120)\n"
             ">>> e.center()\n"
             "Coord(50.0,50.0)\n"
             ">>> e\n"
             "Box2d(0.0, -10.0, 100.0, 110.0)\n"
            )
        .def("height", height_p2,
             "Returns the height of this envelope.\n"
            )
        .def("expand_to_include",expand_to_include_p1,
             (arg("x"),arg("y")),
             "Expands this envelope to include the point given by x and y.\n"
             "\n"
             "Example:\n",
             ">>> e = Box2d(0, 0, 100, 100)\n"
             ">>> e.expand_to_include(110, 110)\n"
             ">>> e\n"
             "Box2d(0.0, 00.0, 110.0, 110.0)\n"
            )
        .def("expand_to_include",expand_to_include_p2,
             (arg("p")),
             "Equivalent to expand_to_include(p.x, p.y)\n"
            )
        .def("expand_to_include",expand_to_include_p3,
             (arg("other")),
             "Equivalent to:\n"
             "  expand_to_include(other.minx, other.miny)\n"
             "  expand_to_include(other.maxx, other.maxy)\n"
            )
        .def("contains",contains_p1,
             (arg("x"),arg("y")),
             "Returns True iff this envelope contains the point\n"
             "given by x and y.\n"
            )
        .def("contains",contains_p2,
             (arg("p")),
             "Equivalent to contains(p.x, p.y)\n"
            )
        .def("contains",contains_p3,
             (arg("other")),
             "Equivalent to:\n"
             "  contains(other.minx, other.miny) and contains(other.maxx, other.maxy)\n"
            )
        .def("intersects",intersects_p1,
             (arg("x"),arg("y")),
             "Returns True iff this envelope intersects the point\n"
             "given by x and y.\n"
             "\n"
             "Note: For points, intersection is equivalent\n"
             "to containment, i.e. the following holds:\n"
             "   e.contains(x, y) == e.intersects(x, y)\n"
            )
        .def("intersects",intersects_p2,
             (arg("p")),
             "Equivalent to contains(p.x, p.y)\n")
        .def("intersects",intersects_p3,
             (arg("other")),
             "Returns True iff this envelope intersects the other envelope,\n"
             "This relationship is symmetric."
             "\n"
             "Example:\n"
             ">>> e1 = Box2d(0, 0, 100, 100)\n"
             ">>> e2 = Box2d(50, 50, 150, 150)\n"
             ">>> e1.intersects(e2)\n"
             "True\n"
             ">>> e1.contains(e2)\n"
             "False\n"
            )
        .def("intersect",intersect,
             (arg("other")),
             "Returns the overlap of this envelope and the other envelope\n"
             "as a new envelope.\n"
             "\n"
             "Example:\n"
             ">>> e1 = Box2d(0, 0, 100, 100)\n"
             ">>> e2 = Box2d(50, 50, 150, 150)\n"
             ">>> e1.intersect(e2)\n"
             "Box2d(50.0, 50.0, 100.0, 100.0)\n"
            )
        .def(self == self) // __eq__
        .def(self != self) // __neq__
        .def(self + self)  // __add__
        .def(self * float()) // __mult__
        .def(float() * self)
        .def(self / float()) // __div__
        .def("__getitem__",&box2d<double>::operator[])
        .def("valid",&box2d<double>::valid)
        .def_pickle(envelope_pickle_suite())
        .def("__deepcopy__", &box2d_deepcopy)
        ;

}
