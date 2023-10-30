#include "catch.hpp"

#include <iostream>
#include <mapnik/projection.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/map.hpp>
#include <mapnik/save_map.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/params.hpp>
#include <mapnik/util/fs.hpp>
#include <vector>
#include <algorithm>

TEST_CASE("exceptions")
{
    SECTION("handling")
    {
        try
        {
            mapnik::projection srs("FAIL");
            // to avoid unused variable warning
            srs.params();
            REQUIRE(false);
        }
        catch (...)
        {
            REQUIRE(true);
        }

        // https://github.com/mapnik/mapnik/issues/2170
        try
        {
            mapnik::projection srs("epsg:4326 foo", true);
            REQUIRE(srs.is_geographic());
            REQUIRE(true);
            srs.init_proj();
            // oddly init_proj does not throw with old proj/ubuntu precise
            // REQUIRE(false);
        }
        catch (...)
        {
            REQUIRE(true);
        }

        try
        {
            mapnik::transcoder tr("bogus encoding");
            REQUIRE(false);
        }
        catch (...)
        {
            REQUIRE(true);
        }

        mapnik::Map map(256, 256);
        mapnik::rule r;
        r.set_filter(mapnik::parse_expression("[foo]='bar'"));
        r.append(mapnik::markers_symbolizer());
        mapnik::feature_type_style style;
        style.add_rule(std::move(r));
        map.insert_style("style", std::move(style));

        std::string csv_plugin("./plugins/input/csv.input");
        if (mapnik::util::exists(csv_plugin))
        {
            try
            {
                mapnik::parameters p;
                p["type"] = "csv";
                p["inline"] = "x,y\n0,0";
                mapnik::datasource_ptr ds = mapnik::datasource_cache::instance().create(p);
                mapnik::layer l("layer");
                l.set_datasource(ds);
                l.add_style("style");
                mapnik::Map m = map;
                m.add_layer(l);
                m.zoom_all();
                mapnik::image_rgba8 im(m.width(), m.height());
                mapnik::agg_renderer<mapnik::image_rgba8> ren(m, im);
                // std::clog << mapnik::save_map_to_string(m) << "\n";
                REQUIRE(true);
                // should throw here with "CSV Plugin: no attribute 'foo'. Valid attributes are: x,y."
                ren.apply();
                REQUIRE(false);
            }
            catch (...)
            {
                REQUIRE(true);
            }
        }

        std::string shape_plugin("./plugins/input/shape.input");
        if (mapnik::util::exists(shape_plugin))
        {
            try
            {
                mapnik::parameters p2;
                p2["type"] = "shape";
                p2["file"] = "foo";
                mapnik::datasource_cache::instance().create(p2);
                REQUIRE(false);
            }
            catch (...)
            {
                REQUIRE(true);
            }
        }
    }
}
