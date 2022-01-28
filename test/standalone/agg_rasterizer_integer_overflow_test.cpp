#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mapnik/map.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/json/feature_parser.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/debug.hpp>

#include <iostream>

// geojson box of the world
const std::string
  geojson("{ \"type\": \"Feature\", \"properties\": { }, \"geometry\": { \"type\": \"Polygon\", \"coordinates\": [ [ [ "
          "-17963313.143242701888084, -6300857.11560364998877 ], [ -17963313.143242701888084, 13071343.332991421222687 "
          "], [ 7396658.353099936619401, 13071343.332991421222687 ], [ 7396658.353099936619401, "
          "-6300857.11560364998877 ], [ -17963313.143242701888084, -6300857.11560364998877 ] ] ] } }");

TEST_CASE("agg_rasterizer_integer_overflow")
{
    SECTION("coordinates_do_not_overflow_and_polygon_is_rendered")
    {
        auto expected_color = mapnik::color("white");

        mapnik::Map m(256, 256);
        m.set_background(mapnik::color("black"));

        mapnik::feature_type_style s;
        {
            mapnik::rule r;
            mapnik::polygon_symbolizer sym;
            mapnik::put(sym, mapnik::keys::fill, expected_color);
            mapnik::put(sym, mapnik::keys::clip, false);
            r.append(std::move(sym));
            s.add_rule(std::move(r));
        }
        m.insert_style("style", std::move(s));

        mapnik::layer lyr("Layer");
        lyr.styles().emplace_back("style");
        {
            auto ds = std::make_shared<mapnik::memory_datasource>(mapnik::parameters());
            auto context = std::make_shared<mapnik::context_type>();
            auto f = std::make_shared<mapnik::feature_impl>(context, 0);
            REQUIRE(mapnik::json::from_geojson(geojson, *f));
            ds->push(f);
            lyr.set_datasource(ds);
        }
        m.add_layer(std::move(lyr));

        // 17/20864/45265.png
        m.zoom_to_box(
          mapnik::box2d<double>(-13658379.710221574, 6197514.253362091, -13657768.213995293, 6198125.749588372));

        // works 15/5216/11316.png
        // m.zoom_to_box(mapnik::box2d<double>(-13658379.710221574,6195679.764683247,-13655933.72531645,6198125.749588372));

        mapnik::image_rgba8 im(256, 256);
        {
            mapnik::agg_renderer<mapnik::image_rgba8> ren(m, im);
            ren.apply();
        }

        REQUIRE(im(128, 128) == expected_color.rgba());
    } // SECTION
} // TEST_CASE
