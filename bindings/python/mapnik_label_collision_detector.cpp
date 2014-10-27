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

#include <mapnik/config.hpp>

// boost
#include "boost_std_shared_shim.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#pragma GCC diagnostic pop

#include <mapnik/label_collision_detector.hpp>
#include <mapnik/map.hpp>

#include <list>

using mapnik::label_collision_detector4;
using mapnik::box2d;
using mapnik::Map;

namespace
{

std::shared_ptr<label_collision_detector4>
create_label_collision_detector_from_extent(box2d<double> const &extent)
{
    return std::make_shared<label_collision_detector4>(extent);
}

std::shared_ptr<label_collision_detector4>
create_label_collision_detector_from_map(Map const &m)
{
    double buffer = m.buffer_size();
    box2d<double> extent(-buffer, -buffer, m.width() + buffer, m.height() + buffer);
    return std::make_shared<label_collision_detector4>(extent);
}

boost::python::list
make_label_boxes(std::shared_ptr<label_collision_detector4> det)
{
    boost::python::list boxes;

    for (label_collision_detector4::query_iterator jtr = det->begin();
         jtr != det->end(); ++jtr)
    {
        boxes.append<box2d<double> >(jtr->box);
    }

    return boxes;
}

}

void export_label_collision_detector()
{
    using namespace boost::python;

    // for overload resolution
    void (label_collision_detector4::*insert_box)(box2d<double> const &) = &label_collision_detector4::insert;

    class_<label_collision_detector4, std::shared_ptr<label_collision_detector4>, boost::noncopyable>
        ("LabelCollisionDetector",
         "Object to detect collisions between labels, used in the rendering process.",
         no_init)

        .def("__init__", make_constructor(create_label_collision_detector_from_extent),
             "Creates an empty collision detection object with a given extent. Note "
             "that the constructor from Map objects is a sensible default and usually "
             "what you want to do.\n"
             "\n"
             "Example:\n"
             ">>> m = Map(size_x, size_y)\n"
             ">>> buf_sz = m.buffer_size\n"
             ">>> extent = mapnik.Box2d(-buf_sz, -buf_sz, m.width + buf_sz, m.height + buf_sz)\n"
             ">>> detector = mapnik.LabelCollisionDetector(extent)")

        .def("__init__", make_constructor(create_label_collision_detector_from_map),
             "Creates an empty collision detection object matching the given Map object. "
             "The created detector will have the same size, including the buffer, as the "
             "map object. This is usually what you want to do.\n"
             "\n"
             "Example:\n"
             ">>> m = Map(size_x, size_y)\n"
             ">>> detector = mapnik.LabelCollisionDetector(m)")

        .def("extent", &label_collision_detector4::extent, return_value_policy<copy_const_reference>(),
             "Returns the total extent (bounding box) of all labels inside the detector.\n"
             "\n"
             "Example:\n"
             ">>> detector.extent()\n"
             "Box2d(573.252589209,494.789179821,584.261023823,496.83610261)")

        .def("boxes", &make_label_boxes,
             "Returns a list of all the label boxes inside the detector.")

        .def("insert", insert_box,
             "Insert a 2d box into the collision detector. This can be used to ensure that "
             "some space is left clear on the map for later overdrawing, for example by "
             "non-Mapnik processes.\n"
             "\n"
             "Example:\n"
             ">>> m = Map(size_x, size_y)\n"
             ">>> detector = mapnik.LabelCollisionDetector(m)"
             ">>> detector.insert(mapnik.Box2d(196, 254, 291, 389))")
        ;
}
