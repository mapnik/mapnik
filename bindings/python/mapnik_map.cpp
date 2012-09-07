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
#include <boost/python/detail/api_placeholder.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

// mapnik
#include <mapnik/layer.hpp>
#include <mapnik/map.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/feature_type_style.hpp>
//#include <mapnik/util/deepcopy.hpp>
#include "mapnik_enumeration.hpp"

using mapnik::color;
using mapnik::coord;
using mapnik::box2d;
using mapnik::layer;
using mapnik::Map;

std::vector<layer>& (Map::*layers_nonconst)() =  &Map::layers;
std::vector<layer> const& (Map::*layers_const)() const =  &Map::layers;
mapnik::parameters& (Map::*params_nonconst)() =  &Map::get_extra_parameters;

mapnik::feature_type_style find_style(mapnik::Map const& m, std::string const& name)
{
    boost::optional<mapnik::feature_type_style const&> style = m.find_style(name);
    if (!style)
    {
        PyErr_SetString(PyExc_KeyError, "Invalid style name");
        boost::python::throw_error_already_set();
    }
    return *style;
}

mapnik::font_set find_fontset(mapnik::Map const& m, std::string const& name)
{
    boost::optional<mapnik::font_set const&> fontset = m.find_fontset(name);
    if (!fontset)
    {
        PyErr_SetString(PyExc_KeyError, "Invalid font_set name");
        boost::python::throw_error_already_set();
    }
    return *fontset;
}

// TODO - we likely should allow indexing by negative number from python
// for now, protect against negative values and kindly throw
mapnik::featureset_ptr query_point(mapnik::Map const& m, int index, double x, double y)
{
    if (index < 0){
        PyErr_SetString(PyExc_IndexError, "Please provide a layer index >= 0");
        boost::python::throw_error_already_set();
    }
    unsigned idx = index;
    return m.query_point(idx, x, y);
}

mapnik::featureset_ptr query_map_point(mapnik::Map const& m, int index, double x, double y)
{
    if (index < 0){
        PyErr_SetString(PyExc_IndexError, "Please provide a layer index >= 0");
        boost::python::throw_error_already_set();
    }
    unsigned idx = index;
    return m.query_map_point(idx, x, y);
}

// deepcopy
/*
mapnik::Map map_deepcopy(mapnik::Map & m, boost::python::dict memo)
{
    // FIXME: ignore memo for now
    mapnik::Map result;
    mapnik::util::deepcopy(m, result);
    return result;
}
*/

void set_maximum_extent(mapnik::Map & m, boost::optional<mapnik::box2d<double> > const& box)
{
    if (box)
    {
        m.set_maximum_extent(*box);
    }
    else
    {
        m.reset_maximum_extent();
    }
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

    class_<std::vector<layer> >("Layers")
        .def(vector_indexing_suite<std::vector<layer> >())
        ;

    class_<Map>("Map","The map object.",init<int,int,optional<std::string const&> >(
                    ( arg("width"),arg("height"),arg("srs") ),
                    "Create a Map with a width and height as integers and, optionally,\n"
                    "an srs string either with a Proj.4 epsg code ('+init=epsg:<code>')\n"
                    "or with a Proj.4 literal ('+proj=<literal>').\n"
                    "If no srs is specified the map will default to '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs'\n"
                    "\n"
                    "Usage:\n"
                    ">>> from mapnik import Map\n"
                    ">>> m = Map(600,400)\n"
                    ">>> m\n"
                    "<mapnik._mapnik.Map object at 0x6a240>\n"
                    ">>> m.srs\n"
                    "'+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs'\n"
                    ))

        .def("append_style",&Map::insert_style,
             (arg("style_name"),arg("style_object")),
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

        .def("append_fontset",&Map::insert_fontset,
             (arg("fontset")),
             "Add a FontSet to the map."
            )

        .def("buffered_envelope",
             &Map::get_buffered_extent,
             "Get the Box2d() of the Map given\n"
             "the Map.buffer_size.\n"
             "\n"
             "Usage:\n"
             ">>> m = Map(600,400)\n"
             ">>> m.envelope()\n"
             "Box2d(-1.0,-1.0,0.0,0.0)\n"
             ">>> m.buffered_envelope()\n"
             "Box2d(-1.0,-1.0,0.0,0.0)\n"
             ">>> m.buffer_size = 1\n"
             ">>> m.buffered_envelope()\n"
             "Box2d(-1.02222222222,-1.02222222222,0.0222222222222,0.0222222222222)\n"
            )

        .def("envelope",
             make_function(&Map::get_current_extent,
                           return_value_policy<copy_const_reference>()),
             "Return the Map Box2d object\n"
             "and print the string representation\n"
             "of the current extent of the map.\n"
             "\n"
             "Usage:\n"
             ">>> m.envelope()\n"
             "Box2d(-0.185833333333,-0.96,0.189166666667,-0.71)\n"
             ">>> dir(m.envelope())\n"
             "...'center', 'contains', 'expand_to_include', 'forward',\n"
             "...'height', 'intersect', 'intersects', 'inverse', 'maxx',\n"
             "...'maxy', 'minx', 'miny', 'width'\n"
            )

        .def("find_fontset",find_fontset,
             (arg("name")),
             "Find a fontset by name."
            )

        .def("find_style",
             find_style,
             (arg("name")),
             "Query the Map for a style by name and return\n"
             "a style object if found or raise KeyError\n"
             "style if not found.\n"
             "\n"
             "Usage:\n"
             ">>> m.find_style('Style Name')\n"
             "<mapnik._mapnik.Style object at 0x654f0>\n"
            )

        .def("pan",&Map::pan,
             (arg("x"),arg("y")),
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
             (arg("x"),arg("y"),arg("factor")),
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

        .def("query_map_point",query_map_point,
             (arg("layer_idx"),arg("pixel_x"),arg("pixel_y")),
             "Query a Map Layer (by layer index) for features \n"
             "intersecting the given x,y location in the pixel\n"
             "coordinates of the rendered map image.\n"
             "Layer index starts at 0 (first layer in map).\n"
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

        .def("query_point",query_point,
             (arg("layer idx"),arg("x"),arg("y")),
             "Query a Map Layer (by layer index) for features \n"
             "intersecting the given x,y location in the coordinates\n"
             "of map projection.\n"
             "Layer index starts at 0 (first layer in map).\n"
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
             "Remove all Mapnik Styles and layers from the Map.\n"
             "\n"
             "Usage:\n"
             ">>> m.remove_all()\n"
            )

        .def("remove_style",&Map::remove_style,
             (arg("style_name")),
             "Remove a Mapnik Style from the map.\n"
             "\n"
             "Usage:\n"
             ">>> m.remove_style('Style Name')\n"
            )

        .def("resize",&Map::resize,
             (arg("width"),arg("height")),
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
             ">>> m.scale_denominator()\n"
            )

        .def("view_transform",&Map::view_transform,
             "Return the map ViewTransform object\n"
             "which is used internally to convert between\n"
             "geographic coordinates and screen coordinates.\n"
             "\n"
             "Usage:\n"
             ">>> m.view_transform()\n"
            )

        .def("zoom",&Map::zoom,
             (arg("factor")),
             "Zoom in or out by a given factor.\n"
             "Positive number zooms in, negative number\n"
             "zooms out.\n"
             "\n"
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

        .def("zoom_to_box",&Map::zoom_to_box,
             (arg("Boxd2")),
             "Set the geographical extent of the map\n"
             "by specifying a Mapnik Box2d.\n"
             "\n"
             "Usage:\n"
             ">>> extext = Box2d(-180.0, -90.0, 180.0, 90.0)\n"
             ">>> m.zoom_to_box(extent)\n"
            )

        //.def("__deepcopy__",&map_deepcopy)
        .add_property("parameters",make_function(params_nonconst,return_value_policy<reference_existing_object>()),"TODO")

        .add_property("aspect_fix_mode",
                      &Map::get_aspect_fix_mode,
                      &Map::set_aspect_fix_mode,
                      // TODO - how to add arg info to properties?
                      //(arg("aspect_fix_mode")),
                      "Get/Set aspect fix mode.\n"
                      "Usage:\n"
                      "\n"
                      ">>> m.aspect_fix_mode = aspect_fix_mode.GROW_BBOX\n"
            )

        .add_property("background",make_function
                      (&Map::background,return_value_policy<copy_const_reference>()),
                      &Map::set_background,
                      "The background color of the map (same as background_color property).\n"
                      "\n"
                      "Usage:\n"
                      ">>> m.background = Color('steelblue')\n"
            )

        .add_property("background_color",make_function
                      (&Map::background,return_value_policy<copy_const_reference>()),
                      &Map::set_background,
                      "The background color of the map.\n"
                      "\n"
                      "Usage:\n"
                      ">>> m.background_color = Color('steelblue')\n"
            )

        .add_property("background_image",make_function
                      (&Map::background_image,return_value_policy<copy_const_reference>()),
                      &Map::set_background_image,
                      "The optional background image of the map.\n"
                      "\n"
                      "Usage:\n"
                      ">>> m.background_image = '/path/to/image.png'\n"
            )

        .add_property("base",
                      make_function(&Map::base_path,return_value_policy<copy_const_reference>()),
                      &Map::set_base_path,
                      "The base path of the map where any files using relative \n"
                      "paths will be interpreted as relative to.\n"
                      "\n"
                      "Usage:\n"
                      ">>> m.base_path = '.'\n"
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
                      &Map::height,
                      &Map::set_height,
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
                      "<mapnik._mapnik.layers object at 0x6d458>"
                      ">>> m.layers[0]\n"
                      "<mapnik._mapnik.layer object at 0x5fe130>\n"
            )

        .add_property("maximum_extent",make_function
                      (&Map::maximum_extent,return_value_policy<copy_const_reference>()),
                      &set_maximum_extent,
                      "The maximum extent of the map.\n"
                      "\n"
                      "Usage:\n"
                      ">>> m.maximum_extent = Box2d(-180,-90,180,90)\n"
            )

        .add_property("srs",
                      make_function(&Map::srs,return_value_policy<copy_const_reference>()),
                      &Map::set_srs,
                      "Spatial reference in Proj.4 format.\n"
                      "Either an epsg code or proj literal.\n"
                      "For example, a proj literal:\n"
                      "\t'+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs'\n"
                      "and a proj epsg code:\n"
                      "\t'+init=epsg:4326'\n"
                      "\n"
                      "Note: using epsg codes requires the installation of\n"
                      "the Proj.4 'epsg' data file normally found in '/usr/local/share/proj'\n"
                      "\n"
                      "Usage:\n"
                      ">>> m.srs\n"
                      "'+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs' # The default srs if not initialized with custom srs\n"
                      ">>> # set to google mercator with Proj.4 literal\n"
                      "... \n"
                      ">>> m.srs = '+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over'\n"
            )

        .add_property("width",
                      &Map::width,
                      &Map::set_width,
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
