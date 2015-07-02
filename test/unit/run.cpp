#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "cleanup.hpp" // run_cleanup()

int main (int argc, char* const argv[])
{
    int result = Catch::Session().run( argc, argv );

    testing::run_cleanup();

    return result;
}
