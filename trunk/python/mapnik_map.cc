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
#include <boost/python/detail/api_placeholder.hpp>

using mapnik::Color;
using mapnik::coord;
using mapnik::Envelope;
using mapnik::Layer;
using mapnik::Map;

struct map_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const Map& m)
    {
        using namespace boost::python;
        return make_tuple(m.getWidth(),m.getHeight(),m.srid());
    }

    static  boost::python::tuple
    getstate(const Map& m)
    {
        using namespace boost::python;
        list l;
        for (unsigned i=0;i<m.layerCount();++i)
        {
            l.append(m.getLayer(i));
        }
        return make_tuple(m.getCurrentExtent(),m.getBackground(),l);
    }

    static void
    setstate (Map& m, boost::python::tuple state)
    {
        using namespace boost::python;
        if (len(state) != 3)
        {
            PyErr_SetObject(PyExc_ValueError,
			    ("expected 3-item tuple in call to __setstate__; got %s"
			     % state).ptr()
			    );
            throw_error_already_set();
        }
        Envelope<double> ext = extract<Envelope<double> >(state[0]);
        Color bg = extract<Color>(state[1]);
        m.zoomToBox(ext);
        m.setBackground(bg);
        list l=extract<list>(state[2]);
        for (int i=0;i<len(l);++i)
        {
            m.addLayer(extract<Layer>(l[i]));
        }
    }
};

void export_map() 
{
    using namespace boost::python;
    class_<Map>("map",init<int,int,optional<int> >())
        .add_property("width",&Map::getWidth)
        .add_property("height",&Map::getHeight)
        .def("background",&Map::setBackground)
        .def("scale", &Map::scale)
        .def("add",&Map::addLayer)
        .def("zoom_to_box",&Map::zoomToBox)
        .def("pan",&Map::pan)
        .def("zoom",&Map::zoom)
        .def("pan_and_zoom",&Map::pan_and_zoom)
        .def_pickle(map_pickle_suite())
        ;
    
}
