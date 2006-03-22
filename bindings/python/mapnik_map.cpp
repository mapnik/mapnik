/* This file is part of python_mapnik (c++/python mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko, Jean-Francois Doyon
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

//$Id: mapnik_map.cc 17 2005-03-08 23:58:43Z pavlenko $


#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <mapnik.hpp>

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
        return boost::python::make_tuple(m.getWidth(),m.getHeight(),m.srid());
    }

    static  boost::python::tuple
    getstate(const Map& m)
    {
        boost::python::list l;
        for (unsigned i=0;i<m.layerCount();++i)
        {
            l.append(m.getLayer(i));
        }
        return boost::python::make_tuple(m.getCurrentExtent(),m.getBackground(),l);
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
        boost::python::list l=extract<boost::python::list>(state[2]);
        for (int i=0;i<len(l);++i)
        {
            m.addLayer(extract<Layer>(l[i]));
        }
    }
};

void export_map() 
{
    using namespace boost::python;
    class_<std::vector<Layer> >("Layers")
    	.def(vector_indexing_suite<std::vector<Layer> >())
    	;
    class_<Map>("Map",init<int,int,boost::python::optional<int> >())
        .add_property("width",&Map::getWidth,&Map::setWidth)
        .add_property("height",&Map::getHeight,&Map::setHeight)
	.add_property("srid",&Map::srid)
	.add_property("background",make_function
		      (&Map::getBackground,return_value_policy<copy_const_reference>()),
		      &Map::setBackground)
        .def("scale", &Map::scale)
        .def("zoom_to_box",&Map::zoomToBox)
        .def("pan",&Map::pan)
        .def("zoom",&Map::zoom)
        .def("pan_and_zoom",&Map::pan_and_zoom)
	.def("append_style",&Map::insert_style)
	.def("remove_style",&Map::remove_style)
	.add_property("layers",make_function
		      (&Map::layers,return_value_policy<reference_existing_object>()))
        .def_pickle(map_pickle_suite())
        ;
}
