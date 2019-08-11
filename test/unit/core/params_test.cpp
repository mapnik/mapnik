#include "catch.hpp"

#include <iostream>
#include <mapnik/value/types.hpp>
#include <mapnik/params.hpp>
#include <mapnik/boolean.hpp>

#include <ostream>

namespace detail {

class string_holder {
 public:
    string_holder() :
     member_("member") {}
    std::string const& get_string() const
    {
        return member_;
    }
 private:
    std::string member_;
};

}

TEST_CASE("parameters") {

SECTION("get/set") {

    try
    {
        mapnik::parameters params;

        // true
        params["bool"] = mapnik::value_integer(true);
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == true));

        params["bool"] = "true";
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == true));

        params["bool"] = mapnik::value_integer(1);
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == true));

        params["bool"] = "1";
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == true));

        params["bool"] = "True";
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == true));

        params["bool"] = "on";
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == true));

        params["bool"] = "yes";
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == true));

        // false
        params["bool"] = mapnik::value_integer(false);
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == false) );

        params["bool"] = "false";
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == false) );

        params["bool"] = mapnik::value_integer(0);
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == false));

        params["bool"] = "0";
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == false));

        params["bool"] = "False";
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == false));

        params["bool"] = "off";
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == false));

        params["bool"] = "no";
        REQUIRE( (params.get<mapnik::boolean_type>("bool") && *params.get<mapnik::boolean_type>("bool") == false));

        // strings
        params["string"] = "hello";
        REQUIRE( (params.get<std::string>("string") && *params.get<std::string>("string") == "hello") );

        // int
        params["int"] = mapnik::value_integer(1);
        REQUIRE( (params.get<mapnik::value_integer>("int") && *params.get<mapnik::value_integer>("int") == 1) );

        // double
        params["double"] = 1.5;
        REQUIRE( (params.get<double>("double") && *params.get<double>("double") == 1.5) );
        // value_null
        params["null"] = mapnik::value_null();
        // https://github.com/mapnik/mapnik/issues/2471
        //REQUIRE( (params.get<mapnik::value_null>("null") && *params.get<mapnik::value_null>("null") == mapnik::value_null()) );

        std::string value("value");
        params["value"] = value;
        REQUIRE( (params.get<std::string>("value") == std::string("value")) ) ;
        REQUIRE(value == std::string("value"));

        // ensure that const member is not moved incorrectly when added to params
        detail::string_holder holder;
        std::string const& holder_member = holder.get_string();
        params["member"] = holder_member;
        REQUIRE( (params.get<std::string>("member") == std::string("member")) );
        REQUIRE( (holder_member == std::string("member")) );
    }
    catch (std::exception const& ex)
    {
        std::cerr << ex.what() << "\n";
        REQUIRE(false);
    }

}
}
