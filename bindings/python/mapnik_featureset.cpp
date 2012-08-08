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
#include <mapnik/feature.hpp>
#include <mapnik/datasource.hpp>

namespace {
using namespace boost::python;

inline list features(mapnik::featureset_ptr const& itr)
{
    list l;
    while (true)
    {
        mapnik::feature_ptr fp = itr->next();
        if (!fp)
        {
            break;
        }
        l.append(fp);
    }
    return l;
}

inline object pass_through(object const& o) { return o; }

inline mapnik::feature_ptr next(mapnik::featureset_ptr const& itr)
{
    mapnik::feature_ptr f = itr->next();
    if (!f)
    {
        PyErr_SetString(PyExc_StopIteration, "No more features.");
        boost::python::throw_error_already_set();
    }

    return f;
}

}

void export_featureset()
{
    using namespace boost::python;
    using mapnik::Feature;
    using mapnik::Featureset;

    class_<Featureset,boost::shared_ptr<Featureset>,
        boost::noncopyable>("Featureset",no_init)
        .def("__iter__",pass_through)
        .def("next",next)
        .add_property("features",features,
                      "The list of features.\n"
                      "\n"
                      "Usage:\n"
                      ">>> m.query_map_point(0, 10, 10)\n"
                      "<mapnik._mapnik.Featureset object at 0x1004d2938>\n"
                      ">>> fs = m.query_map_point(0, 10, 10)\n"
                      ">>> for f in fs.features:\n"
                      ">>>     print f\n"
                      "<mapnik.Feature object at 0x105e64140>\n"
            )
        ;
}
