#include "catch.hpp"

#include <mapnik/agg_renderer.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/map.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/save_map.hpp>

#if defined(HAVE_CAIRO)
#include <mapnik/cairo/cairo_image_util.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#endif

#include <string>

namespace {

mapnik::Map dash_map(std::string const& symbolizers, double scale_factor, double offset = 0.0)
{
    std::string const srs = "epsg:3857";
    mapnik::Map parsed(200 * scale_factor, 80 * scale_factor, srs);
    mapnik::load_map_string(parsed,
                            "<Map background-color='white'><Style name='line'><Rule>" + symbolizers +
                              "</Rule></Style></Map>",
                            true);
    // Exercise serialization as well as XML parsing, including expressions.
    mapnik::Map map(parsed.width(), parsed.height(), srs);
    mapnik::load_map_string(map, mapnik::save_map_to_string(parsed), true);

    mapnik::parameters params;
    params["type"] = "memory";
    auto datasource = std::make_shared<mapnik::memory_datasource>(params);
    auto context = std::make_shared<mapnik::context_type>();
    context->push("phase");
    auto feature = mapnik::feature_factory::create(context, 1);
    feature->put("phase", offset);
    mapnik::geometry::multi_line_string<double> lines;
    // Unequal lengths ensure that the phase restarts on each subpath.
    lines.push_back({{10, 20}, {181, 20}});
    lines.push_back({{10, 60}, {166, 60}});
    feature->set_geometry(std::move(lines));
    datasource->push(feature);

    mapnik::layer layer("lines", srs);
    layer.set_datasource(datasource);
    layer.add_style("line");
    map.add_layer(layer);
    map.zoom_to_box({0, 0, 200, 80});
    return map;
}

mapnik::image_rgba8 render_dashes(mapnik::Map const& map, double scale_factor, std::string const& renderer)
{
    mapnik::image_rgba8 image(map.width(), map.height());
#if defined(HAVE_CAIRO)
    if (renderer == "cairo")
    {
        mapnik::cairo_surface_ptr surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, map.width(), map.height()),
                                          mapnik::cairo_surface_closer());
        mapnik::cairo_ptr context(mapnik::create_context(surface));
        mapnik::cairo_renderer<mapnik::cairo_ptr> ren(map, context, scale_factor);
        ren.apply();
        mapnik::cairo_image_to_rgba8(image, surface);
        return image;
    }
#endif
    mapnik::agg_renderer<mapnik::image_rgba8> ren(map, image, scale_factor);
    ren.apply();
    return image;
}

std::string line(std::string const& attributes)
{
    return "<LineSymbolizer stroke='red' stroke-width='4' clip='false' " + attributes + "/>";
}

} // namespace

TEST_CASE("LineSymbolizer dash offsets", "[renderer][dashoffset]")
{
    auto renderer = GENERATE(std::string("agg")
#if defined(HAVE_CAIRO)
                               ,
                             std::string("cairo")
#endif
    );
    double scale = GENERATE(1.0, 2.0);
    bool expression = GENERATE(false, true);
    struct example
    {
        double offset;
        char const* pattern;
    };
    // Each character describes one pixel of an 8-pixel dash / 8-pixel gap.
    auto example = GENERATE(values<struct example>({{0, "rrrrrrrr........"},
                                                    {4, "rrrr........rrrr"},
                                                    {8, "........rrrrrrrr"},
                                                    {12, "....rrrrrrrr...."},
                                                    {-4, "....rrrrrrrr...."},
                                                    {-8, "........rrrrrrrr"},
                                                    {16, "rrrrrrrr........"},
                                                    {-16, "rrrrrrrr........"},
                                                    {24, "........rrrrrrrr"},
                                                    {-20, "....rrrrrrrr...."},
                                                    {1000000000008.0, "........rrrrrrrr"},
                                                    {-1000000000004.0, "....rrrrrrrr...."}}));
    CAPTURE(renderer, scale, expression, example.offset);
    auto map = dash_map(line("stroke-dasharray='8,8' stroke-dashoffset='" +
                             (expression ? "[phase]" : std::to_string(example.offset)) + "'"),
                        scale,
                        example.offset);
    auto image = render_dashes(map, scale, renderer);
    for (int y : {20, 60})
    {
        for (int x = 0; x < 144 * scale; ++x)
        {
            CAPTURE(x, y);
            auto expected = example.pattern[(x / static_cast<int>(scale)) % 16] == 'r' ? mapnik::color("red").rgba()
                                                                                       : mapnik::color("white").rgba();
            REQUIRE(image(10 * scale + x, y * scale) == expected);
        }
    }
}

TEST_CASE("LineSymbolizer dash offset edge cases", "[renderer][dashoffset]")
{
    auto renderer = GENERATE(std::string("agg")
#if defined(HAVE_CAIRO)
                               ,
                             std::string("cairo")
#endif
    );
    double scale = GENERATE(1.0, 2.0);
    CAPTURE(renderer, scale);
    auto red = mapnik::color("red").rgba();
    auto white = mapnik::color("white").rgba();

    SECTION("starting in a gap with round caps leaves the start unpainted")
    {
        auto map = dash_map(line("stroke-dasharray='8,8' stroke-dashoffset='8' stroke-linecap='round'"), scale);
        auto image = render_dashes(map, scale, renderer);
        CHECK(image(10 * scale, 20 * scale) == white);
        CHECK(image(14 * scale, 20 * scale) == white);
        CHECK(image(18 * scale, 20 * scale) == red);
    }

    SECTION("fractional offsets are scaled with the dash lengths")
    {
        auto map = dash_map(line("stroke-dasharray='8,8' stroke-dashoffset='8.5'"), scale);
        auto image = render_dashes(map, scale, renderer);
        CHECK(image(16 * scale, 20 * scale) == white);
        CHECK(image(18 * scale, 20 * scale) == red);
        if (scale == 2.0)
        {
            CHECK(image(34, 40) == white);
            CHECK(image(35, 40) == red);
        }
    }

    SECTION("offset without a dash array leaves a solid line")
    {
        auto map = dash_map(line("stroke-dashoffset='8'"), scale);
        auto image = render_dashes(map, scale, renderer);
        CHECK(image(12 * scale, 20 * scale) == red);
        CHECK(image(20 * scale, 20 * scale) == red);
    }

    SECTION("an empty dash array with an offset does not hang")
    {
        auto map = dash_map(line("stroke-dasharray='0,0' stroke-dashoffset='8'"), scale);
        CHECK_NOTHROW(render_dashes(map, scale, renderer));
    }

    SECTION("omitted offset starts with the first dash")
    {
        auto map = dash_map(line("stroke-dasharray='8,8'"), scale);
        auto image = render_dashes(map, scale, renderer);
        CHECK(image(12 * scale, 20 * scale) == red);
        CHECK(image(20 * scale, 20 * scale) == white);
    }

    SECTION("offsets count through every pair of a dash array")
    {
        auto map = dash_map(line("stroke-dasharray='4,4,8,8' stroke-dashoffset='10'"), scale);
        auto image = render_dashes(map, scale, renderer);
        CHECK(image(14 * scale, 20 * scale) == red);
        CHECK(image(17 * scale, 20 * scale) == white);
        CHECK(image(25 * scale, 20 * scale) == red);
        CHECK(image(29 * scale, 20 * scale) == white);
    }

    SECTION("two colours can occupy complementary dashes")
    {
        auto map =
          dash_map(line("stroke-dasharray='8,8'") + "<LineSymbolizer stroke='blue' stroke-width='4' clip='false' "
                                                    "stroke-dasharray='8,8' stroke-dashoffset='8'/>",
                   scale);
        auto image = render_dashes(map, scale, renderer);
        CHECK(image(12 * scale, 20 * scale) == red);
        CHECK(image(20 * scale, 20 * scale) == mapnik::color("blue").rgba());
        CHECK(image(28 * scale, 20 * scale) == red);
    }
}
