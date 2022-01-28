
#include "catch.hpp"

#include <iostream>
#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/util/fs.hpp>

TEST_CASE("image")
{
    SECTION("painting")
    {
        using namespace mapnik;

        try
        {
            std::string csv_plugin("./plugins/input/csv.input");
            if (mapnik::util::exists(csv_plugin))
            {
                Map m(256, 256);

                feature_type_style lines_style;
                {
                    rule r;
                    line_symbolizer line_sym;
                    r.append(std::move(line_sym));
                    lines_style.add_rule(std::move(r));
                }
                m.insert_style("lines", std::move(lines_style));

                feature_type_style markers_style;
                {
                    rule r;
                    r.set_filter(parse_expression("False"));
                    markers_symbolizer mark_sym;
                    r.append(std::move(mark_sym));
                    markers_style.add_rule(std::move(r));
                }
                m.insert_style("markers", std::move(markers_style));

                parameters p;
                p["type"] = "csv";
                p["separator"] = "|";
                p["inline"] = "wkt\nLINESTRING(-10  0, 0 20, 10 0, 15 5)";

                layer lyr("layer");
                lyr.set_datasource(datasource_cache::instance().create(p));
                lyr.add_style("lines");
                lyr.add_style("markers");
                m.add_layer(lyr);

                m.zoom_all();

                image_rgba8 image(m.width(), m.height());
                agg_renderer<image_rgba8> ren(m, image);
                ren.apply();

                REQUIRE(image.painted() == true);
            }
        } catch (std::exception const& ex)
        {
            std::clog << ex.what() << std::endl;
            REQUIRE(false);
        }
    }
}
