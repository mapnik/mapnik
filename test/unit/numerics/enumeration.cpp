
#include "catch.hpp"
#include <mapnik/enumeration.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <sstream>

TEST_CASE("enumeration")
{
    mapnik::line_cap_e e(mapnik::ROUND_CAP);
    CHECK(e.as_string() == "round");
    // note: test the << operator, which calls `as_string` internally
    // is not used in mapnik, but kept for back compat
    std::stringstream s;
    s << e;
    CHECK(s.str() == "round");
}
