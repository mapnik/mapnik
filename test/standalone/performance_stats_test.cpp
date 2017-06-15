#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <unistd.h>
#include <iostream>
#include <mapnik/performance_stats.hpp>


TEST_CASE("performance_stats") {
SECTION("collect_and_flush") {
    CHECK(mapnik::timer_stats::instance().flush().size() == 0);
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

    auto metrics = mapnik::timer_stats::instance().flush();
    CHECK(metrics.size() == 2);
    CHECK(metrics["dummy::sleep"].cpu_elapsed <= metrics["dummy::sleep"].wall_clock_elapsed);
    CHECK(metrics["dummy::sleep"].cpu_elapsed <= metrics["busy::wait"].cpu_elapsed);

    CHECK(mapnik::timer_stats::instance().flush().size() == 0);
}
}
