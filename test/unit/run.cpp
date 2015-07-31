#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <mapnik/datasource_cache.hpp>

#include "cleanup.hpp" // run_cleanup()

int main (int argc, char* const argv[])
{
    mapnik::datasource_cache::instance().register_datasources("plugins/input/");

    int result = Catch::Session().run( argc, argv );

    testing::run_cleanup();

    return result;
}
