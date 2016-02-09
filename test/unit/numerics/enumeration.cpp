#include "catch.hpp"
#include <mapnik/enumeration.hpp>
#include <sstream>

namespace mapnik {

    enum _test_enumeration_enum : std::uint8_t
    {
        TEST_ONE,
        TEST_TWO,
        _test_enumeration_enum_MAX
    };

    DEFINE_ENUM( _test_enumeration_e, _test_enumeration_enum );

    static const char * _test_enumeration_strings[] = {
        "test_one",
        "test_two",
        ""
    };

    IMPLEMENT_ENUM( _test_enumeration_e, _test_enumeration_strings )

}

TEST_CASE("enumeration") {

        mapnik::_test_enumeration_e e(mapnik::TEST_ONE);
        CHECK( e.as_string() == "test_one" );
        // test the << operator, which calls `as_string` internally
        // this is not used in mapnik, but kept for back compat
        std::stringstream s;
        s << e;
        CHECK( s.str() == "test_one" );

}