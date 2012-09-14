#include <boost/version.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <mapnik/geometry.hpp>
#include <mapnik/geom_util.hpp>


int main( int, char*[] )
{
    // reused these for simplicity
    double x,y;

    // single point
    mapnik::geometry_type pt(mapnik::Point);
    pt.move_to(10,10);
    BOOST_TEST( mapnik::label::centroid(pt, x, y) );
    BOOST_TEST( x == 10 );
    BOOST_TEST( y == 10 );
    
    // two points
    pt.move_to(20,20);
    BOOST_TEST( mapnik::label::centroid(pt, x, y) );
    BOOST_TEST_EQ( x, 15 );
    BOOST_TEST_EQ( y, 15 );

    // line with two verticies
    mapnik::geometry_type line(mapnik::LineString);
    line.move_to(0,0);
    line.move_to(50,50);
    BOOST_TEST( mapnik::label::centroid(line, x, y) );
    BOOST_TEST( x == 25 );
    BOOST_TEST( y == 25 );
    
    if (!::boost::detail::test_errors()) {
        std::clog << "C++ label algorithms: \x1b[1;32m✓ \x1b[0m\n";
#if BOOST_VERSION >= 104600
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
#endif
    } else {
        return ::boost::report_errors();
    }
}
