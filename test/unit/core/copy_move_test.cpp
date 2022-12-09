
#include <mapnik/layer.hpp>
#include <mapnik/map.hpp>
#include <mapnik/color.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/util/fs.hpp>

#include "catch.hpp"

TEST_CASE("copy")
{
    SECTION("layers")
    {
        try
        {
            mapnik::Map m0(100, 100);
            mapnik::Map m2(200, 100);
            std::string shape_plugin("./plugins/input/shape.input");
            if (mapnik::util::exists(shape_plugin))
            {
                mapnik::parameters p;
                p["type"] = "shape";
                p["file"] = "demo/data/boundaries";
                p["encoding"] = "latin1";
                auto ds0 = mapnik::datasource_cache::instance().create(p);

                auto ds1 = ds0; // shared ptr copy
                REQUIRE((ds1 == ds0));
                REQUIRE(!(*ds1 != *ds0));
                REQUIRE((ds1.get() == ds0.get()));
                ds1 = mapnik::datasource_cache::instance().create(p); // new with the same parameters
                REQUIRE((ds1 != ds0));
                REQUIRE((*ds1 == *ds0));
                auto ds2 = std::move(ds1);
                REQUIRE((ds2 != ds0));
                REQUIRE((*ds2 == *ds0));

                // mapnik::layer
                mapnik::layer l0("test-layer");
                l0.set_datasource(ds0);

                mapnik::layer l1 = l0; // copy assignment
                REQUIRE((l1 == l0));
                mapnik::layer l2(l0); // copy ctor
                REQUIRE((l2 == l0));
                mapnik::layer l3(mapnik::layer("test-layer")); // move ctor
                l3.set_datasource(ds2);

                REQUIRE((l3 == l0));
                mapnik::layer l4 = std::move(l3);
                REQUIRE((l4 == l0)); // move assignment

                m0.add_layer(l4);
                m0.set_background(mapnik::color("skyblue"));
                m2.set_background(mapnik::color("skyblue"));

                auto m1 = m0; // copy

                REQUIRE((m0 == m1));
                REQUIRE((m0 != m2));

                m2 = m1; // copy
                REQUIRE((m2 == m1));
                m2 = std::move(m1);
                REQUIRE((m2 == m0));
                REQUIRE((m1 != m0));

                REQUIRE((m0 == m2));
            }
        }
        catch (std::exception const& ex)
        {
            std::clog << ex.what() << "\n";
            REQUIRE(false);
        }
    }
}
