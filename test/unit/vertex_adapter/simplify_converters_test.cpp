
#include "catch.hpp"

//#include <mapnik/wkt/wkt_factory.hpp>
//#include <mapnik/wkt/wkt_generator_grammar.hpp>
//#include <mapnik/simplify.hpp>
//#include <mapnik/simplify_converter.hpp>

// stl
#include <stdexcept>

// Convenience method for test cases
void simplify(std::string const& wkt_in, double tolerance, std::string const& method, std::string const& expected)
{
#if 0 // FIXME
    //grab the geom
    mapnik::geometry_container multi_input;
    if (!mapnik::from_wkt(wkt_in , multi_input))
    {
        throw std::runtime_error("Failed to parse WKT");
    }
    //setup the generalization
    mapnik::vertex_adapter va(multi_input.front());
    mapnik::simplify_converter<mapnik::vertex_adapter> generalizer(va);
    generalizer.set_simplify_algorithm(mapnik::simplify_algorithm_from_string(method).get());
    generalizer.set_simplify_tolerance(tolerance);
    //suck the vertices back out of it
    mapnik::geometry_type* output = new mapnik::geometry_type(multi_input.front().type());
    mapnik::CommandType cmd;
    double x, y;
    while ((cmd = (mapnik::CommandType)generalizer.vertex(&x, &y)) != mapnik::SEG_END)
    {
        output->push_vertex(x, y, cmd);
    }
    //construct the answer
    mapnik::geometry_container multi_out;
    multi_out.push_back(output);
    std::string wkt_out;
    REQUIRE(mapnik::to_wkt(multi_out, wkt_out));
    REQUIRE(wkt_out == expected);
#endif
}

TEST_CASE("converters")
{
    SECTION("simplify")
    {
        simplify(std::string("LineString(0 0,2 2,3 5,4 1,5 0,6 7,7 0)"),
                 4,
                 "douglas-peucker",
                 std::string("LineString(0 0,6 7,7 0)"));

        simplify(std::string("LineString(0 0,2 2,3 5,4 1,5 0,6 7,7 0)"),
                 2,
                 "douglas-peucker",
                 std::string("LineString(0 0,3 5,5 0,6 7,7 0)"));

        simplify(
          std::string("LineString(10 0,9 -4,7 -7,4 -9,0 -10,-4 -9,-7 -7,-9 -4,-10 0,-9 4,-7 7,-4 9,0 10,4 9,7 7,9 4)"),
          4,
          "douglas-peucker",
          std::string("LineString(10 0,0 -10,-10 0,0 10,9 4)"));

        simplify(std::string("LineString(0 0,1 1,2 2,0 10,0 0)"),
                 10,
                 "douglas-peucker",
                 std::string("LineString(0 0,0 0)"));

        simplify(std::string("LineString(0 0,1 1,2 2,0 10,0 0)"),
                 8,
                 "douglas-peucker",
                 std::string("LineString(0 0,0 10,0 0)"));

        simplify(std::string("LineString(0 0,1 1,2 2,0 10,0 0)"),
                 1,
                 "douglas-peucker",
                 std::string("LineString(0 0,2 2,0 10,0 0)"));

        simplify(std::string("LineString(0 0, 1 -1, 2 2, 0 -10, 0 0, -5 7, 4 6)"),
                 3,
                 "douglas-peucker",
                 std::string("LineString(0 0,0 -10,-5 7,4 6)"));
    }
}
