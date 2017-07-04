
#include "catch.hpp"

#include <mapnik/map.hpp>
#include <mapnik/agg_renderer.hpp>

#if defined(HAVE_CAIRO)
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/cairo/cairo_image_util.hpp>
#endif

TEST_CASE("map") {

SECTION("set background - agg") {

    mapnik::Map map(256, 256);
    mapnik::image_rgba8 image(map.width(), map.height());
    const mapnik::color c1(255, 0, 0);
    mapnik::fill(image, c1);

    CHECK(image(0, 0) == c1.rgba());

    // Fully transparent black should replace red color
    const mapnik::color c2(0, 0, 0, 0);
    map.set_background(c2);
    mapnik::agg_renderer<mapnik::image_rgba8> ren(map, image);
    ren.apply();

    CHECK(image(0, 0) == c2.rgba());
}

#if defined(HAVE_CAIRO)
SECTION("set background - cairo") {

    mapnik::Map map(256, 256);
    mapnik::cairo_surface_ptr image_surface(
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, map.width(), map.height()),
        mapnik::cairo_surface_closer());
    mapnik::cairo_ptr image_context(mapnik::create_context(image_surface));

    cairo_set_source_rgba(image_context.get(), 1.0, 0.0, 0.0, 1.0);
    cairo_paint(image_context.get());

    {
        mapnik::image_rgba8 image(map.width(), map.height());
        mapnik::cairo_image_to_rgba8(image, image_surface);
        const mapnik::color c1(255, 0, 0);
        CHECK(image(0, 0) == c1.rgba());
    }

    // Fully transparent black should replace red color
    const mapnik::color c2(0, 0, 0, 0);
    map.set_background(c2);
    mapnik::cairo_renderer<mapnik::cairo_ptr> ren(map, image_context, 1.0);
    ren.apply();

    {
        mapnik::image_rgba8 image(map.width(), map.height());
        mapnik::cairo_image_to_rgba8(image, image_surface);
        CHECK(image(0, 0) == c2.rgba());
    }
}
#endif

}

