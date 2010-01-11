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
//$Id: mapnik_python.cc 27 2005-03-30 21:45:40Z pavlenko $

#include <boost/python.hpp>
#include <boost/get_pointer.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <boost/python/exception_translator.hpp>

void register_cairo();
void export_color();
void export_coord();
void export_layer();
void export_parameters();
void export_envelope();
void export_query();
void export_geometry();
void export_image();
void export_image_view();
void export_map();
void export_python();
void export_filter();
void export_rule();
void export_style();
void export_stroke();
void export_feature();
void export_featureset();
void export_datasource();
void export_datasource_cache();
void export_symbolizer();
void export_point_symbolizer();
void export_line_symbolizer();
void export_line_pattern_symbolizer();
void export_polygon_symbolizer();
void export_polygon_pattern_symbolizer();
void export_raster_symbolizer();
void export_text_symbolizer();
void export_shield_symbolizer();
void export_font_engine();
void export_projection();
void export_proj_transform();
void export_view_transform();

#include <mapnik/version.hpp>
#include <mapnik/map.hpp>
#include <mapnik/agg_renderer.hpp>
#ifdef HAVE_CAIRO
#include <mapnik/cairo_renderer.hpp>
#endif
#include <mapnik/graphics.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/save_map.hpp>

#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
#include <pycairo.h>
static Pycairo_CAPI_t *Pycairo_CAPI;
#endif

void render(const mapnik::Map& map,mapnik::Image32& image, unsigned offset_x = 0, unsigned offset_y = 0)
{
    Py_BEGIN_ALLOW_THREADS
    try
    {
        mapnik::agg_renderer<mapnik::Image32> ren(map,image,offset_x, offset_y);
        ren.apply();
    }
    catch (...)
    {
        Py_BLOCK_THREADS
        throw;
    }
    Py_END_ALLOW_THREADS
}

void render2(const mapnik::Map& map,mapnik::Image32& image)
{
    Py_BEGIN_ALLOW_THREADS
    try
    {
        mapnik::agg_renderer<mapnik::Image32> ren(map,image);
        ren.apply();
    }
    catch (...)
    {
        Py_BLOCK_THREADS
        throw;
    }
    Py_END_ALLOW_THREADS
}

#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)

void render3(const mapnik::Map& map,PycairoSurface* surface, unsigned offset_x = 0, unsigned offset_y = 0)
{
    Py_BEGIN_ALLOW_THREADS
    try
    {
        Cairo::RefPtr<Cairo::Surface> s(new Cairo::Surface(surface->surface));
        mapnik::cairo_renderer<Cairo::Surface> ren(map,s,offset_x, offset_y);
        ren.apply();
    }
    catch (...)
    {
        Py_BLOCK_THREADS
        throw;
    }
    Py_END_ALLOW_THREADS
}

void render4(const mapnik::Map& map,PycairoSurface* surface)
{
    Py_BEGIN_ALLOW_THREADS
    try
    {
        Cairo::RefPtr<Cairo::Surface> s(new Cairo::Surface(surface->surface));
        mapnik::cairo_renderer<Cairo::Surface> ren(map,s);
        ren.apply();
    }
    catch (...)
    {
        Py_BLOCK_THREADS
        throw;
    }
    Py_END_ALLOW_THREADS
}

void render5(const mapnik::Map& map,PycairoContext* context, unsigned offset_x = 0, unsigned offset_y = 0)
{
    Py_BEGIN_ALLOW_THREADS
    try
    {
        Cairo::RefPtr<Cairo::Context> c(new Cairo::Context(context->ctx));
        mapnik::cairo_renderer<Cairo::Context> ren(map,c,offset_x, offset_y);
        ren.apply();
    }
    catch (...)
    {
        Py_BLOCK_THREADS
        throw;
    }
    Py_END_ALLOW_THREADS
}

void render6(const mapnik::Map& map,PycairoContext* context)
{
    Py_BEGIN_ALLOW_THREADS
    try
    {
        Cairo::RefPtr<Cairo::Context> c(new Cairo::Context(context->ctx));
        mapnik::cairo_renderer<Cairo::Context> ren(map,c);
        ren.apply();
    }
    catch (...)
    {
        Py_BLOCK_THREADS
        throw;
    }
    Py_END_ALLOW_THREADS
}

#endif

void render_tile_to_file(const mapnik::Map& map, 
                         unsigned offset_x, unsigned offset_y,
                         unsigned width, unsigned height,
                         const std::string& file,
                         const std::string& format)
{
    mapnik::Image32 image(width,height);
    render(map,image,offset_x, offset_y);
    mapnik::save_to_file(image.data(),file,format);
}

void render_to_file1(const mapnik::Map& map,
                    const std::string& filename,
                    const std::string& format)
{
    if (format == "pdf" || format == "svg" || format =="ps" || format == "ARGB32" || format == "RGB24")
    {
#if defined(HAVE_CAIRO)
        mapnik::save_to_cairo_file(map,filename,format);
#else
        throw mapnik::ImageWriterException("Cairo backend not available, cannot write to format: " + format);
#endif
    }
    else 
    {
        mapnik::Image32 image(map.getWidth(),map.getHeight());
        render(map,image,0,0);
        mapnik::save_to_file(image,filename,format); 
    }
}

void render_to_file2(const mapnik::Map& map,const std::string& filename)
{
    std::string format = mapnik::guess_type(filename);
    if (format == "pdf" || format == "svg" || format =="ps")
    {
#if defined(HAVE_CAIRO)
        mapnik::save_to_cairo_file(map,filename,format);
#else
        throw mapnik::ImageWriterException("Cairo backend not available, cannot write to format: " + format);
#endif
    }
    else 
    {
        mapnik::Image32 image(map.getWidth(),map.getHeight());
        render(map,image,0,0);
        mapnik::save_to_file(image,filename); 
    }
}

double scale_denominator(mapnik::Map const &map, bool geographic)
{
    return mapnik::scale_denominator(map, geographic);
}

void translator(mapnik::config_error const & ex) {
    PyErr_SetString(PyExc_RuntimeError, ex.what());
}

unsigned mapnik_version()
{
    return MAPNIK_VERSION;
}

unsigned mapnik_svn_revision()
{
#if defined(SVN_REVISION)
  return SVN_REVISION;
#else
  return 0;
#endif
}

// indicator for cairo rendering support inside libmapnik
bool has_cairo()
{
#if defined(HAVE_CAIRO)
  return true;
#else
  return false;
#endif
}

// indicator for pycairo support in the python bindings
bool has_pycairo()
{
#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
   Pycairo_IMPORT;
   /*!
   Case where pycairo support has been compiled into
   mapnik but at runtime the cairo python module 
   is unable to be imported and therefore Pycairo surfaces 
   and contexts cannot be passed to mapnik.render() 
   */ 
   if (Pycairo_CAPI == NULL) return false;
   return true;
#else
   return false;
#endif
}


BOOST_PYTHON_FUNCTION_OVERLOADS(load_map_overloads, load_map, 2, 3);
BOOST_PYTHON_FUNCTION_OVERLOADS(load_map_string_overloads, load_map_string, 2, 4);
BOOST_PYTHON_FUNCTION_OVERLOADS(save_map_overloads, save_map, 2, 3);
BOOST_PYTHON_FUNCTION_OVERLOADS(save_map_to_string_overloads, save_map_to_string, 1, 2);

BOOST_PYTHON_MODULE(_mapnik)
{
 
    using namespace boost::python;

    using mapnik::load_map;
    using mapnik::load_map_string;
    using mapnik::save_map;
    using mapnik::save_map_to_string;

    register_exception_translator<mapnik::config_error>(translator);
    register_cairo();
    export_query();
    export_geometry();
    export_feature();
    export_featureset();
    export_datasource();
    export_parameters();
    export_color(); 
    export_envelope();   
    export_image();
    export_image_view();
    export_filter();
    export_rule();
    export_style();    
    export_layer();
    export_stroke();
    export_datasource_cache();
    export_symbolizer();
    export_point_symbolizer();
    export_line_symbolizer();
    export_line_pattern_symbolizer();
    export_polygon_symbolizer();
    export_polygon_pattern_symbolizer();
    export_raster_symbolizer();
    export_text_symbolizer();
    export_shield_symbolizer();
    export_font_engine();
    export_projection();
    export_proj_transform();
    export_view_transform();
    export_coord();
    export_map();

    def("render_to_file",&render_to_file1,
        "\n"
        "Render Map to file using explicit image type.\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, render_to_file, load_map\n"
        ">>> m = Map(256,256)\n"
        ">>> load_map(m,'mapfile.xml')\n"
        ">>> render_to_file(m,'image32bit.png','png')\n"
        "\n"
        "8 bit (paletted) PNG can be requested with 'png256':\n"
        ">>> render_to_file(m,'8bit_image.png','png256')\n"
        "\n"
        "JPEG quality can be controlled by adding a suffix to\n"
        "'jpeg' between 0 and 100 (default is 85):\n"
        ">>> render_to_file(m,'top_quality.jpeg','jpeg100')\n"
        ">>> render_to_file(m,'medium_quality.jpeg','jpeg50')\n"
        );

    def("render_to_file",&render_to_file2,
        "\n"
        "Render Map to file (type taken from file extension)\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, render_to_file, load_map\n"
        ">>> m = Map(256,256)\n"
        ">>> render_to_file(m,'image.jpeg')\n"
        "\n"
        );

    def("render_tile_to_file",&render_tile_to_file,
        "\n"
        "TODO\n"
        "\n"
        ); 

    
    def("render",&render,
        "\n"
        "Render Map to an AGG Image32 using offsets\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, Image, render, load_map\n"
        ">>> m = Map(256,256)\n"
        ">>> load_map(m,'mapfile.xml')\n"
        ">>> im = Image(m.width,m.height)\n"
        ">>> render(m,im,1,1)\n"
        "\n"
        ); 

    def("render",&render2,
        "\n"
        "Render Map to an AGG Image32\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, Image, render, load_map\n"
        ">>> m = Map(256,256)\n"
        ">>> load_map(m,'mapfile.xml')\n"
        ">>> im = Image(m.width,m.height)\n"
        ">>> render(m,im)\n"
        "\n"
        );

#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
    def("render",&render3,
        "\n"
        "Render Map to Cairo Surface using offsets\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, render, load_map\n"
        ">>> from cairo import SVGSurface\n"
        ">>> m = Map(256,256)\n"
        ">>> load_map(m,'mapfile.xml')\n"
        ">>> surface = SVGSurface('image.svg', m.width, m.height)\n"
        ">>> render(m,surface,1,1)\n"
        "\n"
        );

    def("render",&render4,
        "\n"
        "Render Map to Cairo Surface\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, render, load_map\n"
        ">>> from cairo import SVGSurface\n"
        ">>> m = Map(256,256)\n"
        ">>> load_map(m,'mapfile.xml')\n"
        ">>> surface = SVGSurface('image.svg', m.width, m.height)\n"
        ">>> render(m,surface)\n"
        "\n"
        );

    def("render",&render5,
        "\n"
        "Render Map to Cairo Context using offsets\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, render, load_map\n"
        ">>> from cairo import SVGSurface, Context\n"
        ">>> surface = SVGSurface('image.svg', m.width, m.height)\n"
        ">>> ctx = Context(surface)\n"
        ">>> load_map(m,'mapfile.xml')\n"  
        ">>> render(m,context,1,1)\n"
        "\n"
        );

    def("render",&render6,
        "\n"
        "Render Map to Cairo Context\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, render, load_map\n"
        ">>> from cairo import SVGSurface, Context\n"
        ">>> surface = SVGSurface('image.svg', m.width, m.height)\n"
        ">>> ctx = Context(surface)\n"
        ">>> load_map(m,'mapfile.xml')\n"
        ">>> render(m,context)\n"
        "\n"
        );
#endif

    def("scale_denominator", &scale_denominator,
        "\n"
        "Return the Map Scale Denominator.\n"
        "Also available as Map.scale_denominator()\n"
        "\n"
        "Usage:\n"
        "\n"
        ">>> from mapnik import Map, Projection, scale_denominator, load_map\n"
        ">>> m = Map(256,256)\n"
        ">>> load_map(m,'mapfile.xml')\n"
        ">>> scale_denominator(m,Projection(m.srs).geographic)\n"
        "\n"
        );
   
    def("load_map", & load_map, load_map_overloads());

    def("load_map_from_string", & load_map_string, load_map_string_overloads());

    def("save_map", & save_map, save_map_overloads());
/*
        "\n"
        "Save Map object to XML file\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, load_map, save_map\n"
        ">>> m = Map(256,256)\n"
        ">>> load_map(m,'mapfile_wgs84.xml')\n"
        ">>> m.srs\n"
        "'+proj=latlong +datum=WGS84'\n"
        ">>> m.srs = '+init=espg:3395'\n"
        ">>> save_map(m,'mapfile_mercator.xml')\n"
        "\n"
        );
*/
    
    def("save_map_to_string", & save_map_to_string, save_map_to_string_overloads());
    def("mapnik_version", &mapnik_version,"Get the Mapnik version number");
    def("mapnik_svn_revision", &mapnik_svn_revision,"Get the Mapnik svn revision");
    def("has_cairo", &has_cairo, "Get cairo library status");
    def("has_pycairo", &has_pycairo, "Get pycairo module status");

    register_ptr_to_python<mapnik::filter_ptr>();
}
