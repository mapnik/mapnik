#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <mapnik/geometry.hpp>
#include <mapnik/geom_util.hpp>
#include <vector>
#include <algorithm>

int main(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i=1;i<argc;++i)
    {
        args.push_back(argv[i]);
    }
    bool quiet = std::find(args.begin(), args.end(), "-q")!=args.end();

    // reused these for simplicity
    double x,y;

    // single point
    mapnik::geometry_type pt(mapnik::geometry_type::types::Point);
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
    mapnik::geometry_type line(mapnik::geometry_type::types::LineString);
    line.move_to(0,0);
    line.line_to(50,50);
    BOOST_TEST( mapnik::label::centroid(line, x, y) );
    BOOST_TEST( x == 25 );
    BOOST_TEST( y == 25 );

    // TODO - centroid and interior should be equal but they appear not to be (check largest)
    // MULTIPOLYGON(((-52 40,-60 32,-68 40,-60 48,-52 40)),((-60 50,-80 30,-100 49.9999999999999,-80.0000000000001 70,-60 50)),((-52 60,-60 52,-68 60,-60 68,-52 60)))

    // hit tests
    mapnik::geometry_type pt_hit(mapnik::geometry_type::types::Point);
    pt_hit.move_to(10,10);
    BOOST_TEST( mapnik::label::hit_test(pt_hit, 10, 10, 0.1) );
    BOOST_TEST( !mapnik::label::hit_test(pt_hit, 9, 9, 0) );
    BOOST_TEST( mapnik::label::hit_test(pt_hit, 9, 9, 1.5) );
    mapnik::geometry_type line_hit(mapnik::geometry_type::types::LineString);
    line_hit.move_to(0,0);
    line_hit.line_to(50,50);
    BOOST_TEST( mapnik::label::hit_test(line_hit, 0, 0, 0.001) );
    BOOST_TEST( !mapnik::label::hit_test(line_hit, 1, 1, 0) );
    BOOST_TEST( mapnik::label::hit_test(line_hit, 1, 1, 1.001) );

    if (!::boost::detail::test_errors()) {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ label algorithms: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    } else {
        return ::boost::report_errors();
    }
}
