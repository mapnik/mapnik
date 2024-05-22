#include "catch.hpp"

#include <mapnik/geometry/geometry_type.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/wkt/wkt_generator_grammar.hpp>
#include <mapnik/simplify.hpp>
#include <mapnik/simplify_converter.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/util/geometry_to_wkt.hpp>

// stl
#include <stdexcept>

// Convenience method for test cases
void simplify(std::string const& wkt_in, double tolerance, std::string const& method, std::string const& expected)
{
    // grab the geom
    mapnik::geometry::geometry<double> input;
    if (!mapnik::from_wkt(wkt_in, input))
    {
        throw std::runtime_error("Failed to parse WKT");
    }
    // setup the generalization for LineStings
    if (mapnik::geometry::geometry_type(input) == mapnik::geometry::geometry_types::LineString)
    {
        mapnik::geometry::line_string<double> line = input.get<mapnik::geometry::line_string<double>>();
        mapnik::geometry::line_string_vertex_adapter<double> va(line);
        mapnik::simplify_converter<mapnik::geometry::line_string_vertex_adapter<double>> generalizer(va);
        generalizer.set_simplify_algorithm(mapnik::simplify_algorithm_from_string(method).value());
        generalizer.set_simplify_tolerance(tolerance);
        mapnik::geometry::line_string<double> output;
        mapnik::CommandType cmd;
        double x, y;

        while ((cmd = (mapnik::CommandType)generalizer.vertex(&x, &y)) != mapnik::SEG_END)
        {
            output.emplace_back(x, y);
        }
        std::string wkt_out;
        REQUIRE(mapnik::util::to_wkt(wkt_out, mapnik::geometry::geometry<double>{output}));
        REQUIRE(wkt_out == expected);
    }
}

TEST_CASE("converters")
{
    SECTION("simplify-douglas-peucker")
    {
        simplify(std::string("LINESTRING(0 0,2 2,3 5,4 1,5 0,6 7,7 0)"),
                 4,
                 "douglas-peucker",
                 std::string("LINESTRING(0 0,6 7,7 0)"));

        simplify(std::string("LINESTRING(0 0,2 2,3 5,4 1,5 0,6 7,7 0)"),
                 2,
                 "douglas-peucker",
                 std::string("LINESTRING(0 0,3 5,5 0,6 7,7 0)"));

        simplify(
          std::string("LINESTRING(10 0,9 -4,7 -7,4 -9,0 -10,-4 -9,-7 -7,-9 -4,-10 0,-9 4,-7 7,-4 9,0 10,4 9,7 7,9 4)"),
          4,
          "douglas-peucker",
          std::string("LINESTRING(10 0,0 -10,-10 0,0 10,9 4)"));

        simplify(std::string("LINESTRING(0 0,1 1,2 2,0 10,0 0)"),
                 10,
                 "douglas-peucker",
                 std::string("LINESTRING(0 0,0 0)"));

        simplify(std::string("LINESTRING(0 0,1 1,2 2,0 10,0 0)"),
                 8,
                 "douglas-peucker",
                 std::string("LINESTRING(0 0,0 10,0 0)"));

        simplify(std::string("LINESTRING(0 0,1 1,2 2,0 10,0 0)"),
                 1,
                 "douglas-peucker",
                 std::string("LINESTRING(0 0,2 2,0 10,0 0)"));

        simplify(std::string("LINESTRING(0 0, 1 -1, 2 2, 0 -10, 0 0, -5 7, 4 6)"),
                 3,
                 "douglas-peucker",
                 std::string("LINESTRING(0 0,0 -10,-5 7,4 6)"));

        simplify(
          std::string(
            "LINESTRING(19.1425676643848 48.3706356666503,19.1425502300262 48.3706623938922,19.1425006091595 "
            "48.3706410120998,19.1425113379955 48.3706784302306,19.1424550116062 48.3706632848,19.1424348950386 "
            "48.3707069392642,19.1423343122005 48.3706632848,19.1422645747662 48.3706677393389,19.1411232948303 "
            "48.3712040629718,19.1395568847656 48.3707336664687,19.1382908821106 48.3711042822585,19.1368532180786 "
            "48.3705055938728,19.1365367174149 48.3702775202553,19.1362524032593 48.3704022481414,19.1361290216446 "
            "48.3702917748863,19.1359841823578 48.3703594843292,19.1358715295792 48.3702953385434,19.1357481479645 "
            "48.3703844298907,19.1356247663498 48.3703452297171,19.1356556117535 48.3704075936154,19.1355912387371 "
            "48.3704102663522,19.1355885565281 48.3704432300945)"),
          0.0001,
          "douglas-peucker",
          std::string("LINESTRING(19.1425676643848 48.3706356666503,19.1411232948303 48.3712040629718,19.1395568847656 "
                      "48.3707336664687,19.1382908821106 48.3711042822585,19.1365367174149 "
                      "48.3702775202553,19.1355885565281 48.3704432300945)"));
    }
    SECTION("simplify-radial-distance")
    {
        simplify(std::string("LINESTRING(0 0,2 2,3 5,4 1,5 0,6 7,7 0)"),
                 40,
                 "radial-distance",
                 std::string("LINESTRING(0 0,6 7,7 0)"));

        simplify(std::string("LINESTRING(0 0,2 2,3 5,4 1,5 0,6 7,7 0)"),
                 20,
                 "radial-distance",
                 std::string("LINESTRING(0 0,3 5,5 0,6 7,7 0)"));

        simplify(
          std::string("LINESTRING(10 0,9 -4,7 -7,4 -9,0 -10,-4 -9,-7 -7,-9 -4,-10 0,-9 4,-7 7,-4 9,0 10,4 9,7 7,9 4)"),
          40,
          "radial-distance",
          std::string("LINESTRING(10 0,7 -7,0 -10,-7 -7,-10 0,-7 7,0 10,7 7,9 4)"));

        simplify(std::string("LINESTRING(0 0,1 1,2 2,0 10,0 0)"),
                 100,
                 "radial-distance",
                 std::string("LINESTRING(0 0,0 0)"));
        simplify(std::string("LINESTRING(0 0,1 1,2 2,0 10,0 0)"),
                 10,
                 "radial-distance",
                 std::string("LINESTRING(0 0,0 10,0 0)"));

        simplify(std::string("LINESTRING(0 0, 1 -1, 2 2, 0 -10, 0 0, -5 7, 4 6)"),
                 30,
                 "radial-distance",
                 std::string("LINESTRING(0 0,0 -10,0 0,-5 7,4 6)"));

        simplify(
          std::string(
            "LINESTRING(19.1425676643848 48.3706356666503,19.1425502300262 48.3706623938922,19.1425006091595 "
            "48.3706410120998,19.1425113379955 48.3706784302306,19.1424550116062 48.3706632848,19.1424348950386 "
            "48.3707069392642,19.1423343122005 48.3706632848,19.1422645747662 48.3706677393389,19.1411232948303 "
            "48.3712040629718,19.1395568847656 48.3707336664687,19.1382908821106 48.3711042822585,19.1368532180786 "
            "48.3705055938728,19.1365367174149 48.3702775202553,19.1362524032593 48.3704022481414,19.1361290216446 "
            "48.3702917748863,19.1359841823578 48.3703594843292,19.1358715295792 48.3702953385434,19.1357481479645 "
            "48.3703844298907,19.1356247663498 48.3703452297171,19.1356556117535 48.3704075936154,19.1355912387371 "
            "48.3704102663522,19.1355885565281 48.3704432300945)"),
          0.000001,
          "radial-distance",
          std::string("LINESTRING(19.1425676643848 48.3706356666503,19.1411232948303 48.3712040629718,19.1395568847656 "
                      "48.3707336664687,19.1382908821106 48.3711042822585,19.1368532180786 "
                      "48.3705055938728,19.1358715295792 48.3702953385434,19.1355885565281 48.3704432300945)"));
    }
}
