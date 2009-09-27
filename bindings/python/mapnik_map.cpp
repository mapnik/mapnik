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

// boost
#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

// mapnik
#include <mapnik/layer.hpp>
#include <mapnik/map.hpp>
#include <mapnik/feature_type_style.hpp>

#include "mapnik_enumeration.hpp"
#include "python_optional.hpp"

using mapnik::color;
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
                            
        boost::python::list s;
        Map::const_style_iterator it = m.styles().begin();
        Map::const_style_iterator end = m.styles().end();
        for (; it != end; ++it)
        {
            const std::string & name = it->first;
            const mapnik::feature_type_style & style = it->second;
            boost::python::tuple style_pair = boost::python::make_tuple(name,style);
            s.append(style_pair);
        }

      return boost::python::make_tuple(m.getCurrentExtent(),m.background(),l,s);
   }

   static void
   setstate (Map& m, boost::python::tuple state)
   {
        using namespace boost::python;
        if (len(state) != 4)
        {
            PyErr_SetObject(PyExc_ValueError,
                         ("expected 4-item tuple in call to __setstate__; got %s"
                          % state).ptr()
            );
            throw_error_already_set();
        }

        Envelope<double> ext = extract<Envelope<double> >(state[0]);
        m.zoomToBox(ext);
        if (state[1])
        {
            color bg = extract<color>(state[1]);
            m.set_background(bg);
        }    
        
        boost::python::list l=extract<boost::python::list>(state[2]);
        for (int i=0;i<len(l);++i)
        {
            m.addLayer(extract<Layer>(l[i]));
        }
        
        boost::python::list s=extract<boost::python::list>(state[3]);
        for (int i=0;i<len(s);++i)
        {
            boost::python::tuple style_pair=extract<boost::python::tuple>(s[i]);
            std::string name = extract<std::string>(style_pair[0]);
            mapnik::feature_type_style style = extract<mapnik::feature_type_style>(style_pair[1]);
            m.insert_style(name, style);          
        }
   }
};

std::vector<Layer>& (Map::*layers_nonconst)() =  &Map::layers;
std::vector<Layer> const& (Map::*layers_const)() const =  &Map::layers;


mapnik::feature_type_style find_style (mapnik::Map const& m, std::string const& name)
{
   boost::optional<mapnik::feature_type_style const&> style = m.find_style(name);
   if (!style)
   {
      PyErr_SetString(PyExc_KeyError, "Invalid style name");
      boost::python::throw_error_already_set();
   }
   return *style;
}

void export_map() 
{
   using namespace boost::python;
   
   // aspect ratio fix modes
   mapnik::enumeration_<mapnik::aspect_fix_mode_e>("aspect_fix_mode")
      .value("GROW_BBOX", mapnik::Map::GROW_BBOX)
      .value("GROW_CANVAS",mapnik::Map::GROW_CANVAS)
      .value("SHRINK_BBOX",mapnik::Map::SHRINK_BBOX)
      .value("SHRINK_CANVAS",mapnik::Map::SHRINK_CANVAS)
      .value("ADJUST_BBOX_WIDTH",mapnik::Map::ADJUST_BBOX_WIDTH)
      .value("ADJUST_BBOX_HEIGHT",mapnik::Map::ADJUST_BBOX_HEIGHT)
      .value("ADJUST_CANVAS_WIDTH",mapnik::Map::ADJUST_CANVAS_WIDTH)
      .value("ADJUST_CANVAS_HEIGHT", mapnik::Map::ADJUST_CANVAS_HEIGHT)
      ;
   
   python_optional<mapnik::color> ();
   class_<std::vector<Layer> >("Layers")
      .def(vector_indexing_suite<std::vector<Layer> >())
      ;
    
   class_<Map>("Map","The map object.",init<int,int,optional<std::string const&> >(
                  ( arg("width"),arg("height"),arg("srs") ),
                  "Create a Map with a width and height as integers and, optionally,\n"
                  "an srs string either with a Proj.4 epsg code ('+init=epsg:<code>')\n"
                  "or with a Proj.4 literal ('+proj=<literal>').\n"
                  "If no srs is specified the map will default to '+proj=latlong +datum=WGS84'\n"
                  "\n"
                  "Usage:\n"
                  ">>> from mapnik import Map\n"
                  ">>> m = Map(600,400)\n"
                  ">>> m\n"
                  "<mapnik._mapnik.Map object at 0x6a240>\n"
                  ">>> m.srs\n"
                  "'+proj=latlong +datum=WGS84'\n"
                  ))
        
      .def_pickle(map_pickle_suite()
         )
        
      .def("append_style",&Map::insert_style,
           "Insert a Mapnik Style onto the map by appending it.\n"
           "\n"
           "Usage:\n"
           ">>> sty\n"
           "<mapnik._mapnik.Style object at 0x6a330>\n"
           ">>> m.append_style('Style Name', sty)\n"
           "True # style object added to map by name\n"
           ">>> m.append_style('Style Name', sty)\n"
           "False # you can only append styles with unique names\n"
         )

      .def("buffered_envelope",
           &Map::get_buffered_extent,
           "Get the Envelope() of the Map given\n"
           "the Map.buffer_size.\n"
           "\n"
           "Usage:\n"
           ">>> m = Map(600,400)\n"
           ">>> m.envelope()\n"
           "Envelope(-1.0,-1.0,0.0,0.0)\n"
           ">>> m.buffered_envelope()\n"
           "Envelope(-1.0,-1.0,0.0,0.0)\n"
           ">>> m.buffer_size = 1\n"
           ">>> m.buffered_envelope()\n"
           "Envelope(-1.02222222222,-1.02222222222,0.0222222222222,0.0222222222222)\n"
         )

      .def("envelope",
           make_function(&Map::getCurrentExtent,
                         return_value_policy<copy_const_reference>()),
           "Return the Map Envelope object\n"
           "and print the string representation\n"
           "of the current extent of the map.\n"
           "\n"
           "Usage:\n"
           ">>> m.envelope()\n"
           "Envelope(-0.185833333333,-0.96,0.189166666667,-0.71)\n"
           ">>> dir(m.envelope())\n"
           "...'center', 'contains', 'expand_to_include', 'forward',\n"
           "...'height', 'intersect', 'intersects', 'inverse', 'maxx',\n"
           "...'maxy', 'minx', 'miny', 'width'\n"
         )

      .def("find_style",
           find_style,             
           "Query the Map for a style by name and return\n"
           "a style object if found or raise KeyError\n"
           "style if not found.\n"
           "\n"
           "Usage:\n"
           ">>> m.find_style('Style Name')\n"
           "<mapnik._mapnik.Style object at 0x654f0>\n"
         )
        
      .def("get_aspect_fix_mode",&Map::getAspectFixMode,
           "Get aspect fix mode.\n"
           "Usage:\n"
           "\n"
           ">>> m.get_aspect_fix_mode()\n"
         )

      .def("pan",&Map::pan,
           "Set the Map center at a given x,y location\n"
           "as integers in the coordinates of the pixmap or map surface.\n"
           "\n"
           "Usage:\n"
           ">>> m = Map(600,400)\n"
           ">>> m.envelope().center()\n"
           "Coord(-0.5,-0.5) # default Map center\n"
           ">>> m.pan(-1,-1)\n"
           ">>> m.envelope().center()\n"
           "Coord(0.00166666666667,-0.835)\n"
         )
        
      .def("pan_and_zoom",&Map::pan_and_zoom,
           "Set the Map center at a given x,y location\n"
           "and zoom factor as a float.\n"
           "\n"
           "Usage:\n"
           ">>> m = Map(600,400)\n"
           ">>> m.envelope().center()\n"
           "Coord(-0.5,-0.5) # default Map center\n"
           ">>> m.scale()\n"
           "-0.0016666666666666668\n"
           ">>> m.pan_and_zoom(-1,-1,0.25)\n"
           ">>> m.scale()\n"
           "0.00062500000000000001\n"
         )
        
      .def("query_map_point",&Map::query_map_point,
           "Query a Map Layer (by layer index) for features \n"
           "intersecting the given x,y location in the coordinates\n"
           "of the pixmap or map surface.\n"
           "Will return a Mapnik Featureset if successful\n"
           "otherwise will return None.\n"
           "\n"
           "Usage:\n"
           ">>> featureset = m.query_map_point(0,200,200)\n"
           ">>> featureset\n"
           "<mapnik._mapnik.Featureset object at 0x23b0b0>\n"
           ">>> featureset.features\n"
           ">>> [<mapnik.Feature object at 0x3995630>]\n"
         )
        
      .def("query_point",&Map::query_point,
           "Query a Map Layer (by layer index) for features \n"
           "intersecting the given x,y location in the coordinates\n"
           "of map projection.\n"
           "Will return a Mapnik Featureset if successful\n"
           "otherwise will return None.\n"
           "\n"
           "Usage:\n"
           ">>> featureset = m.query_point(0,-122,48)\n"
           ">>> featureset\n"
           "<mapnik._mapnik.Featureset object at 0x23b0b0>\n"
           ">>> featureset.features\n"
           ">>> [<mapnik.Feature object at 0x3995630>]\n"
         )

      .def("remove_all",&Map::remove_all,
           "Remove all Mapnik Styles and Layers from the Map.\n"
           "\n"
           "Usage:\n"
           ">>> m.remove_all()\n"
         )
        
      .def("remove_style",&Map::remove_style,
           "Remove a Mapnik Style from the map.\n"
           "\n"
           "Usage:\n"
           ">>> m.remove_style('Style Name')\n"
         )

      .def("resize",&Map::resize,
           "Resize a Mapnik Map.\n"
           "\n"
           "Usage:\n"
           ">>> m.resize(64,64)\n"
         )
        
      .def("scale", &Map::scale,
           "Return the Map Scale.\n"
           "Usage:\n"
           "\n"
           ">>> m.scale()\n"
         )

      .def("scale_denominator", &Map::scale_denominator,
           "Return the Map Scale Denominator.\n"
           "Usage:\n"
           "\n"
           ">>> m.scale_denominator\n"
         )
      
      .def("view_transform",&Map::view_transform,
                   "Map CoordinateTransform object.\n"
                   "\n"
                   "Usage:\n"
                   ">>> m.view_transform()\n"         
         )
      
      .def("zoom",&Map::zoom,
           "Zoom in by a given factor.\n"
           "Usage:\n"
           "\n"
           ">>> m.zoom(0.25)\n"
         )
        
      .def("zoom_all",&Map::zoom_all,
           "Set the geographical extent of the map\n"
           "to the combined extents of all active layers.\n"
           "\n"
           "Usage:\n"
           ">>> m.zoom_all()\n"
         )
        
      .def("zoom_to_box",&Map::zoomToBox,
           "Set the geographical extent of the map\n"
           "by specifying a Mapnik Envelope.\n"
           "\n"
           "Usage:\n"
           ">>> extext = Envelope(-180.0, -90.0, 180.0, 90.0)\n"
           ">>> m.zoom_to_box(extent)\n"
         )   

      .add_property("aspect_fix_mode",
                    &Map::getAspectFixMode,
                    &Map::setAspectFixMode,
                    "Get/Set aspect fix mode.\n"
                    "Usage:\n"
                    "\n"
                    ">>> m.aspect_fix_mode = aspect_fix_mode.GROW_BBOX\n"
         )      
        
      .add_property("background",make_function
                    (&Map::background,return_value_policy<copy_const_reference>()),
                    &Map::set_background,
                    "The background color of the map.\n"
                    "\n"
                    "Usage:\n"
                    ">>> m.background = Color('steelblue')\n"
         )
        
      .add_property("buffer_size",
                    &Map::buffer_size,
                    &Map::set_buffer_size,
                    "Get/Set the size of buffer around map in pixels.\n"
                    "\n"
                    "Usage:\n"
                    ">>> m.buffer_size\n"
                    "0 # zero by default\n"
                    ">>> m.buffer_size = 2\n"
                    ">>> m.buffer_size\n"
                    "2\n"
         )
         
      .add_property("height",
                    &Map::getHeight,
                    &Map::setHeight,
                    "Get/Set the height of the map in pixels.\n"
                    "Minimum settable size is 16 pixels.\n"
                    "\n"
                    "Usage:\n"
                    ">>> m.height\n"
                    "400\n"
                    ">>> m.height = 600\n"
                    ">>> m.height\n"
                    "600\n"
         )
        
      .add_property("layers",make_function
                    (layers_nonconst,return_value_policy<reference_existing_object>()), 
                    "The list of map layers.\n"
                    "\n"
                    "Usage:\n"
                    ">>> m.layers\n"
                    "<mapnik._mapnik.Layers object at 0x6d458>"
                    ">>> m.layers[0]\n"
                    "<mapnik._mapnik.Layer object at 0x5fe130>\n"
         )

      .add_property("srs",
                    make_function(&Map::srs,return_value_policy<copy_const_reference>()),
                    &Map::set_srs,
                    "Spatial reference in Proj.4 format.\n"
                    "Either an epsg code or proj literal.\n"
                    "For example, a proj literal:\n"
                    "\t'+proj=latlong +datum=WGS84'\n"
                    "and a proj epsg code:\n"
                    "\t'+init=epsg:4326'\n"
                    "\n"
                    "Note: using epsg codes requires the installation of\n"
                    "the Proj.4 'epsg' data file normally found in '/usr/local/share/proj'\n"
                    "\n"
                    "Usage:\n"
                    ">>> m.srs\n"
                    "'+proj=latlong +datum=WGS84' # The default srs if not initialized with custom srs\n"
                    ">>> # set to google mercator with Proj.4 literal\n"
                    "... \n"
                    ">>> m.srs = '+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over'\n"
         )
        
      .add_property("width",
                    &Map::getWidth,
                    &Map::setWidth,
                    "Get/Set the width of the map in pixels.\n"
                    "Minimum settable size is 16 pixels.\n"
                    "\n"
                    "Usage:\n"
                    ">>> m.width\n"
                    "600\n"
                    ">>> m.width = 800\n"
                    ">>> m.width\n"
                    "800\n"
         )
      ;
}
