#include "catch.hpp"
// mapnik
#include <mapnik/wkb.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_adapters.hpp>
#include <mapnik/geometry_is_empty.hpp>
#include <mapnik/util/geometry_to_wkt.hpp>
// bool
#include <boost/version.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/policies/compare.hpp>
// stl
#include "parse_hex.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#if BOOST_VERSION >= 105800
namespace  {

struct spatially_equal_visitor
{
    using result_type = bool;

    result_type operator() (mapnik::geometry::geometry_empty, mapnik::geometry::geometry_empty) const
    {
        return true;
    }

    result_type operator() (mapnik::geometry::geometry_collection<double> const& lhs, mapnik::geometry::geometry_collection<double> const& rhs) const
    {
        std::size_t size0 = lhs.size();
        std::size_t size1 = rhs.size();
        if (size0 != size1) return false;
        for (std::size_t index = 0; index < size0 ; ++index)
        {
            if (!mapnik::util::apply_visitor(*this, lhs[index], rhs[index]))
                return false;
        }
        return true;
    }

    result_type operator() (mapnik::geometry::multi_point<double> const& lhs, mapnik::geometry::multi_point<double> const& rhs) const
    {
        std::size_t size0 = lhs.size();
        std::size_t size1 = rhs.size();
        if (size0 != size1) return false;
        auto tmp0 = lhs;
        auto tmp1 = rhs;
        std::sort(tmp0.begin(), tmp0.end(), boost::geometry::less<mapnik::geometry::point<double>>());
        std::sort(tmp1.begin(), tmp1.end(), boost::geometry::less<mapnik::geometry::point<double>>());
        for (std::size_t index = 0; index < size0 ; ++index)
        {
            if (!boost::geometry::equals(tmp0[index], tmp1[index])) return false;
        }
        return true;
    }

    result_type operator()(mapnik::geometry::multi_line_string<double> const& lhs,
                           mapnik::geometry::multi_line_string<double> const& rhs) const
    {

        std::size_t size0 = lhs.size();
        std::size_t size1 = rhs.size();
        if (size0 != size1)
            return false;

        for (std::size_t index = 0; index < size0; ++index)
        {
            if (!boost::geometry::equals(lhs[index], rhs[index]))
                return false;
        }
        return true;
    }

    template <typename T>
    result_type operator() (T const& lhs, T const& rhs) const
    {
        if (mapnik::geometry::is_empty(lhs) && mapnik::geometry::is_empty(rhs))
            return true; //Empty geometries of the same type are considered to be spatially equal
        return boost::geometry::equals(lhs, rhs);
    }

    template <typename T0, typename T1>
    result_type operator() (T0 const& lhs, T1 const& rhs) const
    {
        return false;
    }

};

template <typename T>
bool spatially_equal(mapnik::geometry::geometry<T> const& g0, mapnik::geometry::geometry<T> const& g1)
{
    return mapnik::util::apply_visitor(spatially_equal_visitor(), g0, g1);
}

}
#endif

TEST_CASE("Well-known-geometries")
{
    SECTION("wkb")
    {
        std::string filename("test/unit/data/well-known-geometries.test");
        std::ifstream is(filename.c_str(),std::ios_base::in | std::ios_base::binary);
        if (!is) throw std::runtime_error("could not open: '" + filename + "'");

        for (std::string line; std::getline(is, line,'\n');)
        {
            std::vector<std::string> columns;
            boost::split(columns, line, boost::is_any_of(";"));
            REQUIRE(columns.size() == 3);
            std::vector<char> wkb, twkb;
            REQUIRE(mapnik::util::parse_hex(columns[1], wkb));
            REQUIRE(mapnik::util::parse_hex(columns[2], twkb));
            mapnik::geometry::geometry<double> geom_0 = mapnik::geometry_utils::from_wkb(wkb.data(), wkb.size(), mapnik::wkbAuto);
            mapnik::geometry::geometry<double> geom_1 = mapnik::geometry_utils::from_twkb(twkb.data(), twkb.size());
            // compare WKTs
            std::string wkt0, wkt1;
            REQUIRE(mapnik::util::to_wkt(wkt0, geom_0));
            REQUIRE(mapnik::util::to_wkt(wkt1, geom_1));
            if (!mapnik::geometry::is_empty(geom_0) && !mapnik::geometry::is_empty(geom_1))
            {
                REQUIRE(wkt0 == wkt1);
                // compare spatially (NOTE: GeometryCollection comparison also enforces strict order)
#if BOOST_VERSION >= 105800
                REQUIRE(spatially_equal(geom_0, geom_1));
#endif
            }
        }
    }
}
