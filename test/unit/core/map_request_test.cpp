#include "catch.hpp"

#include <iostream>
#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#if defined(HAVE_CAIRO)
#include <mapnik/cairo/cairo_renderer.hpp>
#endif
#include <mapnik/image_util.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/scale_denominator.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/layer.hpp>

#include <vector>
#include <algorithm>

TEST_CASE("mapnik::request") {

SECTION("rendering") {

    try {

        mapnik::datasource_cache::instance().register_datasources("plugins/input/csv.input");
        mapnik::freetype_engine::register_fonts("./fonts", true );
        mapnik::Map m(256,256);
        mapnik::load_map(m,"./tests/data/good_maps/marker-text-line.xml",false);
        m.zoom_all();
        mapnik::image_rgba8 im(m.width(),m.height());
        double scale_factor = 1.2;

        // render normally with apply() and just map and image
        mapnik::agg_renderer<mapnik::image_rgba8> renderer1(m,im,scale_factor);
        renderer1.apply();

        // reset image
        mapnik::fill(im, 0);

        // set up a mapnik::request object
        mapnik::request req(m.width(),m.height(),m.get_current_extent());
        req.set_buffer_size(m.buffer_size());

        // render using apply() and mapnik::request
        mapnik::attributes vars;
        mapnik::agg_renderer<mapnik::image_rgba8> renderer2(m,req,vars,im,scale_factor);
        renderer2.apply();

        // reset image
        mapnik::fill(im, 0);

        // render with apply_to_layer api and mapnik::request params passed to apply_to_layer
        mapnik::agg_renderer<mapnik::image_rgba8> renderer3(m,req,vars,im,scale_factor);
        renderer3.start_map_processing(m);
        mapnik::projection map_proj(m.srs(),true);
        double scale_denom = mapnik::scale_denominator(req.scale(),map_proj.is_geographic());
        scale_denom *= scale_factor;
        for (mapnik::layer const& lyr : m.layers() )
        {
            if (lyr.visible(scale_denom))
            {
                std::set<std::string> names;
                renderer3.apply_to_layer(lyr,
                                         renderer3,
                                         map_proj,
                                         req.scale(),
                                         scale_denom,
                                         req.width(),
                                         req.height(),
                                         req.extent(),
                                         req.buffer_size(),
                                         names);

            }
        }
        renderer3.end_map_processing(m);

        // also test cairo
#if defined(HAVE_CAIRO)
        mapnik::cairo_surface_ptr image_surface(
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32,req.width(),req.height()),
            mapnik::cairo_surface_closer());
        mapnik::cairo_ptr image_context = (mapnik::create_context(image_surface));
        mapnik::cairo_renderer<mapnik::cairo_ptr> png_render(m,req,vars,image_context,scale_factor);
        png_render.apply();
        //cairo_surface_write_to_png(&*image_surface, expected_cairo.c_str());
        //std::string actual_cairo("/tmp/map-request-marker-text-line-actual4.png");
        //cairo_surface_write_to_png(&*image_surface, actual_cairo.c_str());
        // TODO - non visual way to test
#endif
        // TODO - test grid_renderer

    } catch (std::exception const& ex) {
        std::clog << ex.what() << "\n";
    }
}
}
