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
void export_palette();
void export_image();
void export_image_view();
void export_gamma_method();
void export_scaling_method();
void export_grid();
void export_grid_view();
void export_map();
void export_python();
void export_expression();
void export_rule();
void export_style();
void export_stroke();
void export_feature();
void export_featureset();
void export_fontset();
void export_datasource();
void export_datasource_cache();
void export_symbolizer();
void export_markers_symbolizer();
void export_point_symbolizer();
void export_line_symbolizer();
void export_line_pattern_symbolizer();
void export_polygon_symbolizer();
void export_building_symbolizer();
void export_polygon_pattern_symbolizer();
void export_raster_symbolizer();
void export_text_placement();
void export_shield_symbolizer();
void export_font_engine();
void export_projection();
void export_proj_transform();
void export_view_transform();
void export_raster_colorizer();
void export_label_collision_detector();
void export_logger();

#include <mapnik/version.hpp>
#include <mapnik/value_error.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/map.hpp>
#include <mapnik/agg_renderer.hpp>
#ifdef HAVE_CAIRO
#include <mapnik/cairo_renderer.hpp>
#endif
#include <mapnik/graphics.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/scale_denominator.hpp>
#include <mapnik/value_error.hpp>
#include <mapnik/save_map.hpp>
#include <mapnik/scale_denominator.hpp>
#include "python_grid_utils.hpp"
#include "mapnik_value_converter.hpp"
#include "mapnik_threads.hpp"
#include "python_optional.hpp"
#include <mapnik/marker_cache.hpp>
#include <mapnik/mapped_memory_cache.hpp>


void clear_cache()
{
    mapnik::marker_cache::instance().clear();
    mapnik::mapped_memory_cache::instance().clear();
}

#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
#include <pycairo.h>
static Pycairo_CAPI_t *Pycairo_CAPI;
#endif

using mapnik::python_thread;
using mapnik::python_unblock_auto_block;
#ifdef MAPNIK_DEBUG
bool python_thread::thread_support = true;
#endif
boost::thread_specific_ptr<PyThreadState> python_thread::state;

void render(const mapnik::Map& map,
            mapnik::image_32& image,
            double scale_factor = 1.0,
            unsigned offset_x = 0u,
            unsigned offset_y = 0u)
{
    python_unblock_auto_block b;
    mapnik::agg_renderer<mapnik::image_32> ren(map,image,scale_factor,offset_x, offset_y);
    ren.apply();

}

void render_with_detector(
    const mapnik::Map &map,
    mapnik::image_32 &image,
    boost::shared_ptr<mapnik::label_collision_detector4> detector,
    double scale_factor = 1.0,
    unsigned offset_x = 0u,
    unsigned offset_y = 0u)
{
    python_unblock_auto_block b;
    mapnik::agg_renderer<mapnik::image_32> ren(map,image,detector);
    ren.apply();
}

void render_layer2(const mapnik::Map& map,
                   mapnik::image_32& image,
                   unsigned layer_idx)
{
    std::vector<mapnik::layer> const& layers = map.layers();
    std::size_t layer_num = layers.size();
    if (layer_idx >= layer_num) {
        std::ostringstream s;
        s << "Zero-based layer index '" << layer_idx << "' not valid, only '"
          << layer_num << "' layers are in map\n";
        throw std::runtime_error(s.str());
    }

    python_unblock_auto_block b;
    mapnik::layer const& layer = layers[layer_idx];
    mapnik::agg_renderer<mapnik::image_32> ren(map,image,1.0,0,0);
    std::set<std::string> names;
    ren.apply(layer,names);
}

#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)

void render3(const mapnik::Map& map,
             PycairoSurface* surface,
             double scale_factor = 1.0,
             unsigned offset_x = 0,
             unsigned offset_y = 0)
{
    python_unblock_auto_block b;
    Cairo::RefPtr<Cairo::Surface> s(new Cairo::Surface(surface->surface));
    mapnik::cairo_renderer<Cairo::Surface> ren(map,s,scale_factor,offset_x,offset_y);
    ren.apply();
}

void render4(const mapnik::Map& map, PycairoSurface* surface)
{
    python_unblock_auto_block b;
    Cairo::RefPtr<Cairo::Surface> s(new Cairo::Surface(surface->surface));
    mapnik::cairo_renderer<Cairo::Surface> ren(map,s);
    ren.apply();
}

void render5(const mapnik::Map& map,
             PycairoContext* context,
             double scale_factor = 1.0,
             unsigned offset_x = 0,
             unsigned offset_y = 0)
{
    python_unblock_auto_block b;
    Cairo::RefPtr<Cairo::Context> c(new Cairo::Context(context->ctx));
    mapnik::cairo_renderer<Cairo::Context> ren(map,c,scale_factor,offset_x, offset_y);
    ren.apply();
}

void render6(const mapnik::Map& map, PycairoContext* context)
{
    python_unblock_auto_block b;
    Cairo::RefPtr<Cairo::Context> c(new Cairo::Context(context->ctx));
    mapnik::cairo_renderer<Cairo::Context> ren(map,c);
    ren.apply();
}

void render_with_detector2(
    const mapnik::Map& map,
    PycairoContext* context,
    boost::shared_ptr<mapnik::label_collision_detector4> detector)
{
    python_unblock_auto_block b;
    Cairo::RefPtr<Cairo::Context> c(new Cairo::Context(context->ctx));
    mapnik::cairo_renderer<Cairo::Context> ren(map,c,detector);
    ren.apply();
}

void render_with_detector3(
    const mapnik::Map& map,
    PycairoContext* context,
    boost::shared_ptr<mapnik::label_collision_detector4> detector,
    double scale_factor = 1.0,
    unsigned offset_x = 0u,
    unsigned offset_y = 0u)
{
    python_unblock_auto_block b;
    Cairo::RefPtr<Cairo::Context> c(new Cairo::Context(context->ctx));
    mapnik::cairo_renderer<Cairo::Context> ren(map,c,detector,scale_factor,offset_x,offset_y);
    ren.apply();
}

void render_with_detector4(
    const mapnik::Map& map,
    PycairoSurface* surface,
    boost::shared_ptr<mapnik::label_collision_detector4> detector)
{
    python_unblock_auto_block b;
    Cairo::RefPtr<Cairo::Surface> s(new Cairo::Surface(surface->surface));
    mapnik::cairo_renderer<Cairo::Surface> ren(map,s,detector);
    ren.apply();
}

void render_with_detector5(
    const mapnik::Map& map,
    PycairoSurface* surface,
    boost::shared_ptr<mapnik::label_collision_detector4> detector,
    double scale_factor = 1.0,
    unsigned offset_x = 0u,
    unsigned offset_y = 0u)
{
    python_unblock_auto_block b;
    Cairo::RefPtr<Cairo::Surface> s(new Cairo::Surface(surface->surface));
    mapnik::cairo_renderer<Cairo::Surface> ren(map,s,detector,scale_factor,offset_x,offset_y);
    ren.apply();
}

#endif


void render_tile_to_file(const mapnik::Map& map,
                         unsigned offset_x, unsigned offset_y,
                         unsigned width, unsigned height,
                         std::string const& file,
                         std::string const& format)
{
    mapnik::image_32 image(width,height);
    render(map,image,1.0,offset_x, offset_y);
    mapnik::save_to_file(image.data(),file,format);
}

void render_to_file1(const mapnik::Map& map,
                     std::string const& filename,
                     std::string const& format)
{
    if (format == "pdf" || format == "svg" || format =="ps" || format == "ARGB32" || format == "RGB24")
    {
#if defined(HAVE_CAIRO)
        mapnik::save_to_cairo_file(map,filename,format,1.0);
#else
        throw mapnik::ImageWriterException("Cairo backend not available, cannot write to format: " + format);
#endif
    }
    else
    {
        mapnik::image_32 image(map.width(),map.height());
        render(map,image,1.0,0,0);
        mapnik::save_to_file(image,filename,format);
    }
}

void render_to_file2(const mapnik::Map& map,std::string const& filename)
{
    std::string format = mapnik::guess_type(filename);
    if (format == "pdf" || format == "svg" || format =="ps")
    {
#if defined(HAVE_CAIRO)
        mapnik::save_to_cairo_file(map,filename,format,1.0);
#else
        throw mapnik::ImageWriterException("Cairo backend not available, cannot write to format: " + format);
#endif
    }
    else
    {
        mapnik::image_32 image(map.width(),map.height());
        render(map,image,1.0,0,0);
        mapnik::save_to_file(image,filename);
    }
}

void render_to_file3(const mapnik::Map& map,
                     std::string const& filename,
                     std::string const& format,
                     double scale_factor = 1.0
    )
{
    if (format == "pdf" || format == "svg" || format =="ps" || format == "ARGB32" || format == "RGB24")
    {
#if defined(HAVE_CAIRO)
        mapnik::save_to_cairo_file(map,filename,format,scale_factor);
#else
        throw mapnik::ImageWriterException("Cairo backend not available, cannot write to format: " + format);
#endif
    }
    else
    {
        mapnik::image_32 image(map.width(),map.height());
        render(map,image,scale_factor,0,0);
        mapnik::save_to_file(image,filename,format);
    }
}

double scale_denominator(mapnik::Map const &map, bool geographic)
{
    return mapnik::scale_denominator(map, geographic);
}

// http://docs.python.org/c-api/exceptions.html#standard-exceptions
void config_error_translator(mapnik::config_error const & ex)
{
    PyErr_SetString(PyExc_RuntimeError, ex.what());
}

void value_error_translator(mapnik::value_error const & ex)
{
    PyErr_SetString(PyExc_ValueError, ex.what());
}

void runtime_error_translator(std::runtime_error const & ex)
{
    PyErr_SetString(PyExc_RuntimeError, ex.what());
}

unsigned mapnik_version()
{
    return MAPNIK_VERSION;
}

std::string mapnik_version_string()
{
    return MAPNIK_VERSION_STRING;
}

// indicator for jpeg read/write support within libmapnik
bool has_jpeg()
{
#if defined(HAVE_JPEG)
    return true;
#else
    return false;
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
    Pycairo_CAPI = (Pycairo_CAPI_t*) PyCObject_Import(const_cast<char *>("cairo"), const_cast<char *>("CAPI"));
    if (Pycairo_CAPI == NULL){
        /*
          Case where pycairo support has been compiled into
          mapnik but at runtime the cairo python module
          is unable to be imported and therefore Pycairo surfaces
          and contexts cannot be passed to mapnik.render()
        */
        return false;
    }
    return true;
#else
    return false;
#endif
}


BOOST_PYTHON_FUNCTION_OVERLOADS(load_map_overloads, load_map, 2, 3)
BOOST_PYTHON_FUNCTION_OVERLOADS(load_map_string_overloads, load_map_string, 2, 4)
BOOST_PYTHON_FUNCTION_OVERLOADS(save_map_overloads, save_map, 2, 3)
BOOST_PYTHON_FUNCTION_OVERLOADS(save_map_to_string_overloads, save_map_to_string, 1, 2)
BOOST_PYTHON_FUNCTION_OVERLOADS(render_overloads, render, 2, 5)
BOOST_PYTHON_FUNCTION_OVERLOADS(render_with_detector_overloads, render_with_detector, 3, 6)

BOOST_PYTHON_MODULE(_mapnik)
{

    using namespace boost::python;

    using mapnik::load_map;
    using mapnik::load_map_string;
    using mapnik::save_map;
    using mapnik::save_map_to_string;
    using mapnik::render_grid;

    register_exception_translator<mapnik::config_error>(&config_error_translator);
    register_exception_translator<mapnik::value_error>(&value_error_translator);
    register_exception_translator<std::runtime_error>(&runtime_error_translator);
    register_cairo();
    export_query();
    export_geometry();
    export_feature();
    export_featureset();
    export_fontset();
    export_datasource();
    export_parameters();
    export_color();
    export_envelope();
    export_palette();
    export_image();
    export_image_view();
    export_gamma_method();
    export_scaling_method();
    export_grid();
    export_grid_view();
    export_expression();
    export_rule();
    export_style();
    export_layer();
    export_stroke();
    export_datasource_cache();
    export_symbolizer();
    export_markers_symbolizer();
    export_point_symbolizer();
    export_line_symbolizer();
    export_line_pattern_symbolizer();
    export_polygon_symbolizer();
    export_building_symbolizer();
    export_polygon_pattern_symbolizer();
    export_raster_symbolizer();
    export_text_placement();
    export_shield_symbolizer();
    export_font_engine();
    export_projection();
    export_proj_transform();
    export_view_transform();
    export_coord();
    export_map();
    export_raster_colorizer();
    export_label_collision_detector();
    export_logger();

    def("clear_cache", &clear_cache,
        "\n"
        "Clear all global caches of markers and mapped memory regions.\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import clear_cache\n"
        ">>> clear_cache()\n"
        );

    def("render_grid",&render_grid,
        ( arg("map"),
          arg("layer"),
          args("key")="__id__",
          arg("resolution")=4,
          arg("fields")=boost::python::list()
            )
        );

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

    def("render_to_file",&render_to_file3,
        "\n"
        "Render Map to file using explicit image type and scale factor.\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, render_to_file, load_map\n"
        ">>> m = Map(256,256)\n"
        ">>> scale_factor = 4\n"
        ">>> render_to_file(m,'image.jpeg',scale_factor)\n"
        "\n"
        );

    def("render_tile_to_file",&render_tile_to_file,
        "\n"
        "TODO\n"
        "\n"
        );


    def("render", &render, render_overloads(
            "\n"
            "Render Map to an AGG image_32 using offsets\n"
            "\n"
            "Usage:\n"
            ">>> from mapnik import Map, Image, render, load_map\n"
            ">>> m = Map(256,256)\n"
            ">>> load_map(m,'mapfile.xml')\n"
            ">>> im = Image(m.width,m.height)\n"
            ">>> scale_factor=2.0\n"
            ">>> offset = [100,50]\n"
            ">>> render(m,im)\n"
            ">>> render(m,im,scale_factor)\n"
            ">>> render(m,im,scale_factor,offset[0],offset[1])\n"
            "\n"
            ));

    def("render_with_detector", &render_with_detector, render_with_detector_overloads(
            "\n"
            "Render Map to an AGG image_32 using a pre-constructed detector.\n"
            "\n"
            "Usage:\n"
            ">>> from mapnik import Map, Image, LabelCollisionDetector, render_with_detector, load_map\n"
            ">>> m = Map(256,256)\n"
            ">>> load_map(m,'mapfile.xml')\n"
            ">>> im = Image(m.width,m.height)\n"
            ">>> detector = LabelCollisionDetector(m)\n"
            ">>> render_with_detector(m, im, detector)\n"
            ));

    def("render_layer", &render_layer2,
        (arg("map"),arg("image"),args("layer"))
        );

    def("render_layer", &mapnik::render_layer_for_grid,
        (arg("map"),arg("grid"),args("layer"),arg("fields")=boost::python::list())
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

    def("render_with_detector", &render_with_detector2,
        "\n"
        "Render Map to Cairo Context using a pre-constructed detector.\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, LabelCollisionDetector, render_with_detector, load_map\n"
        ">>> from cairo import SVGSurface, Context\n"
        ">>> surface = SVGSurface('image.svg', m.width, m.height)\n"
        ">>> ctx = Context(surface)\n"
        ">>> m = Map(256,256)\n"
        ">>> load_map(m,'mapfile.xml')\n"
        ">>> detector = LabelCollisionDetector(m)\n"
        ">>> render_with_detector(m, ctx, detector)\n"
        );

    def("render_with_detector", &render_with_detector3,
        "\n"
        "Render Map to Cairo Context using a pre-constructed detector, scale and offsets.\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, LabelCollisionDetector, render_with_detector, load_map\n"
        ">>> from cairo import SVGSurface, Context\n"
        ">>> surface = SVGSurface('image.svg', m.width, m.height)\n"
        ">>> ctx = Context(surface)\n"
        ">>> m = Map(256,256)\n"
        ">>> load_map(m,'mapfile.xml')\n"
        ">>> detector = LabelCollisionDetector(m)\n"
        ">>> render_with_detector(m, ctx, detector, 1, 1, 1)\n"
        );

    def("render_with_detector", &render_with_detector4,
        "\n"
        "Render Map to Cairo Surface using a pre-constructed detector.\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, LabelCollisionDetector, render_with_detector, load_map\n"
        ">>> from cairo import SVGSurface, Context\n"
        ">>> surface = SVGSurface('image.svg', m.width, m.height)\n"
        ">>> m = Map(256,256)\n"
        ">>> load_map(m,'mapfile.xml')\n"
        ">>> detector = LabelCollisionDetector(m)\n"
        ">>> render_with_detector(m, surface, detector)\n"
        );

    def("render_with_detector", &render_with_detector5,
        "\n"
        "Render Map to Cairo Surface using a pre-constructed detector, scale and offsets.\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import Map, LabelCollisionDetector, render_with_detector, load_map\n"
        ">>> from cairo import SVGSurface, Context\n"
        ">>> surface = SVGSurface('image.svg', m.width, m.height)\n"
        ">>> m = Map(256,256)\n"
        ">>> load_map(m,'mapfile.xml')\n"
        ">>> detector = LabelCollisionDetector(m)\n"
        ">>> render_with_detector(m, surface, detector, 1, 1, 1)\n"
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

    def("load_map", &load_map, load_map_overloads());

    def("load_map_from_string", &load_map_string, load_map_string_overloads());

    def("save_map", &save_map, save_map_overloads());
/*
  "\n"
  "Save Map object to XML file\n"
  "\n"
  "Usage:\n"
  ">>> from mapnik import Map, load_map, save_map\n"
  ">>> m = Map(256,256)\n"
  ">>> load_map(m,'mapfile_wgs84.xml')\n"
  ">>> m.srs\n"
  "'+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs'\n"
  ">>> m.srs = '+init=espg:3395'\n"
  ">>> save_map(m,'mapfile_mercator.xml')\n"
  "\n"
  );
*/

    def("save_map_to_string", &save_map_to_string, save_map_to_string_overloads());
    def("mapnik_version", &mapnik_version,"Get the Mapnik version number");
    def("mapnik_version_string", &mapnik_version_string,"Get the Mapnik version string");
    def("has_jpeg", &has_jpeg, "Get jpeg read/write support status");
    def("has_cairo", &has_cairo, "Get cairo library status");
    def("has_pycairo", &has_pycairo, "Get pycairo module status");

    python_optional<mapnik::stroke>();
    python_optional<mapnik::font_set>();
    python_optional<mapnik::color>();
    python_optional<mapnik::box2d<double> >();
    python_optional<mapnik::composite_mode_e>();
    python_optional<mapnik::datasource::geometry_t>();
    python_optional<std::string>();
    python_optional<unsigned>();
    python_optional<double>();
    python_optional<float>();
    python_optional<bool>();
    python_optional<mapnik::text_transform_e>();
    register_ptr_to_python<mapnik::expression_ptr>();
    register_ptr_to_python<mapnik::path_expression_ptr>();
    to_python_converter<mapnik::value_holder,mapnik_param_to_python>();
    to_python_converter<mapnik::value,mapnik_value_to_python>();
}
