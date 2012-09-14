#include <boost/version.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <mapnik/params.hpp>
#include <mapnik/boolean.hpp>

int main( int, char*[] )
{

    mapnik::parameters params;

    // true
    params["bool"] = true;
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == true));

    params["bool"] = "true";
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == true));

    params["bool"] = 1;
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == true));

    params["bool"] = "1";
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == true));

    params["bool"] = "True";
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == true));

    params["bool"] = "on";
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == true));

    params["bool"] = "yes";
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == true));

    // false
    params["bool"] = false;
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == false) );

    params["bool"] = "false";
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == false) );

    params["bool"] = 0;
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == false));

    params["bool"] = "0";
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == false));

    params["bool"] = "False";
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == false));

    params["bool"] = "off";
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == false));

    params["bool"] = "no";
    BOOST_TEST( (params.get<mapnik::boolean>("bool") && *params.get<mapnik::boolean>("bool") == false));

    // strings
    params["string"] = "hello";
    BOOST_TEST( (params.get<std::string>("string") && *params.get<std::string>("string") == "hello") );

    // int
    params["int"] = 1;
    BOOST_TEST( (params.get<int>("int") && *params.get<int>("int") == 1) );

    // double
    params["double"] = 1.5;
    BOOST_TEST( (params.get<double>("double") && *params.get<double>("double") == 1.5) );

    // value_null
    params["null"] = mapnik::value_null();
    //BOOST_TEST( (params.get<mapnik::value_null>("null")/* && *params.get<mapnik::value_null>("null") == mapnik::value_null()*/) );

    if (!::boost::detail::test_errors()) {
        std::clog << "C++ parameters: \x1b[1;32m✓ \x1b[0m\n";
#if BOOST_VERSION >= 104600
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
#endif
    } else {
        return ::boost::report_errors();
    }
}
