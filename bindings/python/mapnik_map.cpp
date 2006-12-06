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
//$Id: mapnik_map.cc 17 2005-03-08 23:58:43Z pavlenko $


#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <mapnik/layer.hpp>
#include <mapnik/map.hpp>

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
        return boost::python::make_tuple(m.getWidth(),m.getHeight(),m.srs());
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

std::vector<Layer>& (Map::*layers_nonconst)() =  &Map::layers;
std::vector<Layer> const& (Map::*layers_const)() const =  &Map::layers;

void export_map() 
{
    using namespace boost::python;
    class_<std::vector<Layer> >("Layers")
    	.def(vector_indexing_suite<std::vector<Layer> >())
    	;
    
    class_<Map>("Map","The map object.",init<int,int,optional<std::string const&> >())
        .add_property("width",&Map::getWidth, &Map::setWidth, "The width of the map.")
        .add_property("height",&Map::getHeight, &Map::setHeight, "The height of the map.")
        .add_property("srs",make_function(&Map::srs,return_value_policy<copy_const_reference>()),
                      &Map::set_srs,"Spatial reference in proj4 format e.g. \"+proj=latlong +datum=WGS84\"")
        .add_property("background",make_function
                      (&Map::getBackground,return_value_policy<copy_const_reference>()),
                      &Map::setBackground, "The background color of the map.")
        .def("envelope",make_function(&Map::getCurrentExtent,
                                      return_value_policy<copy_const_reference>()),
             "The current extent of the map")
        .def("scale", &Map::scale)
        .def("zoom_all",&Map::zoom_all,
             "Set the geographical extent of the map "
             "to the combined extents of all active layers")
        .def("zoom_to_box",&Map::zoomToBox, "Set the geographical extent of the map.")
        .def("pan",&Map::pan)
        .def("zoom",&Map::zoom)
        .def("zoom_all",&Map::zoom_all)
        .def("pan_and_zoom",&Map::pan_and_zoom)
        .def("append_style",&Map::insert_style)
        .def("remove_style",&Map::remove_style)
        .def("query_point",&Map::query_point)
        .def("query_map_point",&Map::query_map_point)
        .add_property("layers",make_function
                      (layers_nonconst,return_value_policy<reference_existing_object>()), 
                      "Get the list of layers in this map.")
        .def("find_style",&Map::find_style,return_value_policy<copy_const_reference>())
        .def_pickle(map_pickle_suite())
        ;
}
