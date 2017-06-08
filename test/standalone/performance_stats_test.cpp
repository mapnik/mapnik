#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <unistd.h>
#include <iostream>
#include <mapnik/performance_stats.hpp>


TEST_CASE("performance_stats") {
SECTION("collect_and_flush") {
    CHECK(mapnik::timer_stats::instance().flush().length() == 0);
    {
        mapnik::stats_timer __stats__("dummy::sleep");
        usleep(10000);
    }

    {
        mapnik::stats_timer __stats__("dummy::sleep");
        usleep(10000);
    }

    mapnik::stats_timer __stats__("busy::wait");
    unsigned long int counter = 0;
    for(unsigned int i=0; i<10000; i++) {
        for(unsigned int j=0; j<10000; j++) {
            counter++;
        }
    }
    __stats__.stop();

    std::string out = mapnik::timer_stats::instance().flush();
    CHECK(out.length() > 0);
    std::cout << out;
    CHECK(mapnik::timer_stats::instance().flush().length() == 0);
}
}
