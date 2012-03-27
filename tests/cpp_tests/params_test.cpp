
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

  return ::boost::report_errors();
}
