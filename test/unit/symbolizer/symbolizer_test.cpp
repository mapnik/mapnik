
#include "catch.hpp"

#include <iostream>
#include <mapnik/symbolizer.hpp>

using namespace mapnik;

TEST_CASE("symbolizer")
{
    SECTION("enums")
    {
        try
        {
            marker_multi_policy_enum policy_in = MARKER_WHOLE_MULTI;
            REQUIRE(policy_in == MARKER_WHOLE_MULTI);
            markers_symbolizer sym;
            put(sym, keys::markers_multipolicy, policy_in);
            REQUIRE(sym.properties.count(keys::markers_multipolicy) == static_cast<unsigned long>(1));
            marker_multi_policy_enum policy_out = get<mapnik::marker_multi_policy_enum>(sym, keys::markers_multipolicy);
            REQUIRE(policy_out == MARKER_WHOLE_MULTI);
        }
        catch (std::exception const& ex)
        {
            std::clog << ex.what() << std::endl;
            REQUIRE(false);
        }
    }
}
