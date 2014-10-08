#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "utils.hpp"

#include <mapnik/layer.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/wkt/wkt_grammar_impl.hpp>
#include <mapnik/wkt/wkt_generator_grammar.hpp>
#include <mapnik/wkt/wkt_generator_grammar_impl.hpp>
#include <mapnik/simplify.hpp>
#include <mapnik/simplify_converter.hpp>

// stl
#include <stdexcept>

// Convenience method for test cases
void simplify(const std::string& wkt_in, double tolerance, const std::string& method, const std::string& expected)
{
    //grab the geom
    mapnik::geometry_container multi_input;
    if (!mapnik::from_wkt(wkt_in , multi_input))
    {
        throw std::runtime_error("Failed to parse WKT");
    }
    //setup the generalization
    mapnik::simplify_converter<mapnik::geometry_type> generalizer(multi_input.front());
    generalizer.set_simplify_algorithm(mapnik::simplify_algorithm_from_string(method).get());
    generalizer.set_simplify_tolerance(tolerance);
    //suck the vertices back out of it
    mapnik::geometry_type* output = new mapnik::geometry_type(multi_input.front().type());
    mapnik::CommandType cmd;
    double x, y;
    while((cmd = (mapnik::CommandType)generalizer.vertex(&x, &y)) != mapnik::SEG_END)
    {
        output->push_vertex(x, y, cmd);
    }
    //construct the answer
    mapnik::geometry_container multi_out;
    multi_out.push_back(output);
    std::string wkt_out;
    BOOST_TEST(mapnik::to_wkt(multi_out, wkt_out));
    BOOST_TEST_EQ(wkt_out, expected);
}

int main(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i=1;i<argc;++i)
    {
        args.push_back(argv[i]);
    }
    bool quiet = std::find(args.begin(), args.end(), "-q")!=args.end();

    BOOST_TEST(set_working_dir(args));

    simplify(	std::string("LineString(0 0,2 2,3 5,4 1,5 0,6 7,7 0)"),
				4, "douglas-peucker",
				std::string("LineString(0 0,6 7,7 0)"));

    simplify(  	std::string("LineString(0 0,2 2,3 5,4 1,5 0,6 7,7 0)"),
				2, "douglas-peucker",
				std::string("LineString(0 0,3 5,5 0,6 7,7 0)"));

    simplify(  	std::string("LineString(10 0,9 -4,7 -7,4 -9,0 -10,-4 -9,-7 -7,-9 -4,-10 0,-9 4,-7 7,-4 9,0 10,4 9,7 7,9 4)"),
				4, "douglas-peucker",
				std::string("LineString(10 0,0 -10,-10 0,0 10,9 4)"));

	simplify(  	std::string("LineString(0 0,1 1,2 2,0 10,0 0)"),
				10, "douglas-peucker",
				std::string("LineString(0 0,0 0)"));

    simplify(  	std::string("LineString(0 0,1 1,2 2,0 10,0 0)"),
				8, "douglas-peucker",
				std::string("LineString(0 0,0 10,0 0)"));

    simplify(  	std::string("LineString(0 0,1 1,2 2,0 10,0 0)"),
				1, "douglas-peucker",
				std::string("LineString(0 0,2 2,0 10,0 0)"));

    simplify(  	std::string("LineString(0 0, 1 -1, 2 2, 0 -10, 0 0, -5 7, 4 6)"),
				3, "douglas-peucker",
				std::string("LineString(0 0,0 -10,-5 7,4 6)"));

    if (!::boost::detail::test_errors())
    {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ simplify conversions: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    }
    else
    {
        return ::boost::report_errors();
    }
}
