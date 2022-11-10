#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mapnik/datasource_cache.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/util/fs.hpp>

#include <iostream>
#include <vector>
#include <algorithm>

TEST_CASE("datasource_cache")
{
    SECTION("registration")
    {
        try
        {
            mapnik::logger logger;
            mapnik::logger::severity_type original_severity = logger.get_severity();
            bool success = false;
            auto& cache = mapnik::datasource_cache::instance();

            // registering a directory without any plugins should return false
            success = cache.register_datasources("test/data/vrt");
            CHECK(success == false);

            // use existence of shape.input as proxy for whether any datasource plugins are available
            std::string shape_plugin("./plugins/input/shape.input");
            if (mapnik::util::exists(shape_plugin))
            {
                // registering a directory for the first time should return true
                success = cache.register_datasources("plugins/input");
                REQUIRE(success == true);

                // registering the same directory again should now return false
                success = cache.register_datasources("plugins/input");
                CHECK(success == false);

                // registering the same directory again, but recursively should
                // still return false - even though there are subdirectories, they
                // do not contain any more plugins.
                success = cache.register_datasources("plugins/input", true);
                CHECK(success == false);
            }
        }
        catch (std::exception const& ex)
        {
            std::clog << ex.what() << "\n";
            REQUIRE(false);
        }
    }
}
