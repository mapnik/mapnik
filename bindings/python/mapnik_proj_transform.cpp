/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2009 Artem Pavlenko
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

// mapnik
#include <mapnik/proj_transform.hpp>

// boost
#include <boost/python.hpp>

using mapnik::proj_transform;
using mapnik::projection;

struct proj_transform_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const proj_transform& p)
    {
        using namespace boost::python;
        return boost::python::make_tuple(p.source(),p.dest());
    }
};

namespace  {

mapnik::coord2d forward_transform_c(mapnik::proj_transform& t, mapnik::coord2d const& c)
{
    double x = c.x;
    double y = c.y;
    double z = 0.0;
    if (!t.forward(x,y,z)) {
        std::ostringstream s;
        s << "Failed to forward project "
          << c << " from " << t.source().params() << " to: " << t.dest().params();
        throw std::runtime_error(s.str());
    }
    return mapnik::coord2d(x,y);
}

mapnik::coord2d backward_transform_c(mapnik::proj_transform& t, mapnik::coord2d const& c)
{
    double x = c.x;
    double y = c.y;
    double z = 0.0;
    if (!t.backward(x,y,z)) {
        std::ostringstream s;
        s << "Failed to back project "
          << c << " from " <<  t.dest().params() << " to: " << t.source().params();
        throw std::runtime_error(s.str());
    }
    return mapnik::coord2d(x,y);
}

mapnik::box2d<double> forward_transform_env(mapnik::proj_transform& t, mapnik::box2d<double> const & box)
{
    mapnik::box2d<double> new_box = box;
    if (!t.forward(new_box)) {
        std::ostringstream s;
        s << "Failed to forward project "
          << box << " from " << t.source().params() << " to: " << t.dest().params();
        throw std::runtime_error(s.str());
    }
    return new_box;
}

mapnik::box2d<double> backward_transform_env(mapnik::proj_transform& t, mapnik::box2d<double> const & box)
{
    mapnik::box2d<double> new_box = box;
    if (!t.backward(new_box)){
        std::ostringstream s;
        s << "Failed to back project "
          << box << " from " <<  t.dest().params() << " to: " << t.source().params();
        throw std::runtime_error(s.str());
    }
    return new_box;
}

mapnik::box2d<double> forward_transform_env_p(mapnik::proj_transform& t, mapnik::box2d<double> const & box, unsigned int points)
{
    mapnik::box2d<double> new_box = box;
    if (!t.forward(new_box,points)) {
        std::ostringstream s;
        s << "Failed to forward project "
          << box << " from " << t.source().params() << " to: " << t.dest().params();
        throw std::runtime_error(s.str());
    }
    return new_box;
}

mapnik::box2d<double> backward_transform_env_p(mapnik::proj_transform& t, mapnik::box2d<double> const & box, unsigned int points)
{
    mapnik::box2d<double> new_box = box;
    if (!t.backward(new_box,points)){
        std::ostringstream s;
        s << "Failed to back project "
          << box << " from " <<  t.dest().params() << " to: " << t.source().params();
        throw std::runtime_error(s.str());
    }
    return new_box;
}

}

void export_proj_transform ()
{
    using namespace boost::python;

    class_<proj_transform, boost::noncopyable>("ProjTransform", init< projection const&, projection const& >())
        .def_pickle(proj_transform_pickle_suite())
        .def("forward", forward_transform_c)
        .def("backward",backward_transform_c)
        .def("forward", forward_transform_env)
        .def("backward",backward_transform_env)
        .def("forward", forward_transform_env_p)
        .def("backward",backward_transform_env_p)
        ;

}
