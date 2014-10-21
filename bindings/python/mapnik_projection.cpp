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

#include <mapnik/config.hpp>

// boost
#include "boost_std_shared_shim.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <boost/python.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/coord.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/projection.hpp>

using mapnik::projection;

struct projection_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const projection& p)
    {
        using namespace boost::python;
        return boost::python::make_tuple(p.params());
    }
};

namespace {
mapnik::coord2d forward_pt(mapnik::coord2d const& pt,
                           mapnik::projection const& prj)
{
    double x = pt.x;
    double y = pt.y;
    prj.forward(x,y);
    return mapnik::coord2d(x,y);
}

mapnik::coord2d inverse_pt(mapnik::coord2d const& pt,
                           mapnik::projection const& prj)
{
    double x = pt.x;
    double y = pt.y;
    prj.inverse(x,y);
    return mapnik::coord2d(x,y);
}

mapnik::box2d<double> forward_env(mapnik::box2d<double> const & box,
                                  mapnik::projection const& prj)
{
    double minx = box.minx();
    double miny = box.miny();
    double maxx = box.maxx();
    double maxy = box.maxy();
    prj.forward(minx,miny);
    prj.forward(maxx,maxy);
    return mapnik::box2d<double>(minx,miny,maxx,maxy);
}

mapnik::box2d<double> inverse_env(mapnik::box2d<double> const & box,
                                  mapnik::projection const& prj)
{
    double minx = box.minx();
    double miny = box.miny();
    double maxx = box.maxx();
    double maxy = box.maxy();
    prj.inverse(minx,miny);
    prj.inverse(maxx,maxy);
    return mapnik::box2d<double>(minx,miny,maxx,maxy);
}

}

void export_projection ()
{
    using namespace boost::python;

    class_<projection>("Projection", "Represents a map projection.",init<std::string const&>(
                           (arg("proj4_string")),
                           "Constructs a new projection from its PROJ.4 string representation.\n"
                           "\n"
                           "The constructor will throw a RuntimeError in case the projection\n"
                           "cannot be initialized.\n"
                           )
        )
        .def_pickle(projection_pickle_suite())
        .def ("params", make_function(&projection::params,
                                      return_value_policy<copy_const_reference>()),
              "Returns the PROJ.4 string for this projection.\n")
        .def ("expanded",&projection::expanded,
              "normalize PROJ.4 definition by expanding +init= syntax\n")
        .add_property ("geographic", &projection::is_geographic,
                       "This property is True if the projection is a geographic projection\n"
                       "(i.e. it uses lon/lat coordinates)\n")
        ;

    def("forward_",&forward_pt);
    def("inverse_",&inverse_pt);
    def("forward_",&forward_env);
    def("inverse_",&inverse_env);

}
