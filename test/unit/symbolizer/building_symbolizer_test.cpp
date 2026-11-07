#include "catch.hpp"

#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image.hpp>
#include <mapnik/color.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/geometry.hpp>

#include <string>

using namespace mapnik;

namespace {

// Renders one 32x32 px square building, centered on a 64x64 px map, with the
// given symbolizer attributes, and returns the color in its middle. With
// height="0" only the roof covers that pixel: the walls have no area and the
// outlines run along the edges of the square.
color render_building(std::string const& background, std::string const& attrs, bool& premultiplied)
{
    Map m(64, 64);
    load_map_string(m,
                    R"xml(<Map background-color=")xml" + background +
                      R"xml("><Style name="buildings"><Rule><BuildingSymbolizer height="0" )xml" + attrs +
                      R"xml(/></Rule></Style></Map>)xml");
    context_ptr ctx = std::make_shared<context_type>();
    feature_ptr feature = std::make_shared<feature_impl>(ctx, 1);
    geometry::polygon<double> square;
    square.push_back(geometry::linear_ring<double>{{16, 16}, {48, 16}, {48, 48}, {16, 48}, {16, 16}});
    feature->set_geometry(std::move(square));
    parameters params;
    params["type"] = "memory";
    auto ds = std::make_shared<memory_datasource>(params);
    ds->push(feature);
    layer lyr("buildings");
    lyr.set_datasource(ds);
    lyr.add_style("buildings");
    m.add_layer(lyr);
    m.zoom_to_box(box2d<double>(0, 0, 64, 64));
    image_rgba8 im(64, 64);
    agg_renderer<image_rgba8> ren(m, im);
    ren.apply();
    premultiplied = im.get_premultiplied();
    return color(im(32, 32), false);
}

} // namespace

TEST_CASE("building_symbolizer")
{
    bool premultiplied = true;

    SECTION("without comp-op the roof is drawn over the background")
    {
        color const c = render_building("rgb(64,96,255)", R"attrs(fill="rgb(255,128,0)")attrs", premultiplied);
        REQUIRE(int(c.red()) == 255);
        REQUIRE(int(c.green()) == 128);
        REQUIRE(int(c.blue()) == 0);
        REQUIRE(int(c.alpha()) == 255);
    }

    SECTION("comp-op is applied")
    {
        // multiply: 255*64/255, 128*96/255, 0*255/255
        color const c =
          render_building("rgb(64,96,255)", R"attrs(fill="rgb(255,128,0)" comp-op="multiply")attrs", premultiplied);
        REQUIRE(int(c.red()) == Approx(64).margin(1));
        REQUIRE(int(c.green()) == Approx(48).margin(1));
        REQUIRE(int(c.blue()) == 0);
        REQUIRE(int(c.alpha()) == 255);
    }

    SECTION("a comp-op that lowers the alpha of an opaque map is demultiplied")
    {
        // the renderer skips demultiplying a map that stayed opaque, so it must learn of a
        // blend that can make it translucent
        color const c = render_building("rgb(255,0,0)",
                                        R"attrs(fill="black" fill-opacity="0.5" comp-op="dst-out")attrs",
                                        premultiplied);
        REQUIRE_FALSE(premultiplied);
        // half of the red background is cut out: red at half alpha, not half red
        REQUIRE(int(c.alpha()) == Approx(128).margin(2));
        REQUIRE(int(c.red()) >= 253);
        REQUIRE(int(c.green()) == 0);
        REQUIRE(int(c.blue()) == 0);
    }
}
