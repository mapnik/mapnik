#include "catch.hpp"
#include "geometry_equal.hpp"

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/geometry/reprojection.hpp>

TEST_CASE("geometry reprojection") {

SECTION("test_projection_4326_3857 - Empty Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans(source, dest);
    {
        geometry_empty geom;
        unsigned int err = 0;
        // Test Standard Transform
        geometry_empty new_geom = reproject_copy(geom, proj_trans, err);
        REQUIRE(err == 0);
        // Transform providing projections not transfrom
        new_geom = reproject_copy(geom, source, dest, err);
        REQUIRE(err == 0);
        // Transform providing projections in reverse
        new_geom = reproject_copy(geom, dest, source, err);
        REQUIRE(err == 0);
        // Transform in place
        REQUIRE(reproject(new_geom, proj_trans));
        // Transform in place providing projections
        REQUIRE(reproject(new_geom, source, dest));
        // Transform in place provoding projections reversed
        REQUIRE(reproject(new_geom, dest, source));
    }
} // End Section

SECTION("test_projection_4326_3857 - Empty Geometry in Geometry Variant") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans(source, dest);
    {
        geometry<double> geom = geometry_empty();
        unsigned int err = 0;
        // Test Standard Transform
        geometry<double> new_geom = reproject_copy(geom, proj_trans, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
        // Transform providing projections not transfrom
        new_geom = reproject_copy(geom, source, dest, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
        // Transform providing projections in reverse
        new_geom = reproject_copy(geom, dest, source, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
        // Transform in place
        REQUIRE(reproject(new_geom, proj_trans));
        // Transform in place providing projections
        REQUIRE(reproject(new_geom, source, dest));
        // Transform in place provoding projections reversed
        REQUIRE(reproject(new_geom, dest, source));
    }
} // End Section

SECTION("test_projection_4326_3857 - Point Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    point<double> geom1(-97.552175, 35.522895);
    point<double> geom2(-10859458.446776, 4235169.496066);
    unsigned int err = 0;
    {
        // Test Standard Transform
        point<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        point<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        point<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        point<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        point<double> geom3(-97.552175, 35.522895);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse - back
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        point<double> geom3(-97.552175, 35.522895);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Point Geometry Variant Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    double x1 = -97.552175;
    double y1 = 35.522895;
    double x2 = -10859458.446776;
    double y2 = 4235169.496066;
    geometry<double> geom1(point<double>(x1, y1));
    geometry<double> geom2(point<double>(x2, y2));
    unsigned int err = 0;
    {
        // Test Standard Transform
        geometry<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        geometry<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        geometry<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        geometry<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        geometry<double> geom3(point<double>(-97.552175, 35.522895));
        // Transform in place
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse - back
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        geometry<double> geom3(point<double>(-97.552175, 35.522895));
        // Transform in place providing projections
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section */

SECTION("test_projection_4326_3857 - Line_String Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    line_string<double> geom1;
    geom1.emplace_back(point<double>(-97.48872756958008, 35.360286150765084));
    geom1.emplace_back(point<double>(-97.48065948486328, 35.34894577151337));
    geom1.emplace_back(point<double>(-97.47267723083496, 35.36224605490395));
    geom1.emplace_back(point<double>(-97.46323585510252, 35.34523530173256));
    geom1.emplace_back(point<double>(-97.45963096618651, 35.36329598397908));
    geom1.emplace_back(point<double>(-97.47550964355469, 35.369245324153866));
    line_string<double> geom2;
    geom2.emplace_back(point<double>(-10852395.511130, 4212951.024108));
    geom2.emplace_back(point<double>(-10851497.376047, 4211403.174286));
    geom2.emplace_back(point<double>(-10850608.795594, 4213218.553707));
    geom2.emplace_back(point<double>(-10849557.786455, 4210896.778973));
    geom2.emplace_back(point<double>(-10849156.492056, 4213361.873135));
    geom2.emplace_back(point<double>(-10850924.098335, 4214174.016561));
    unsigned int err = 0;
    {
        // Test Standard Transform
        line_string<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        line_string<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        line_string<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        line_string<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        line_string<double> geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        line_string<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Line_String Geometry Variant Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    line_string<double> geom1_;
    geom1_.emplace_back(point<double>(-97.48872756958008, 35.360286150765084));
    geom1_.emplace_back(point<double>(-97.48065948486328, 35.34894577151337));
    geom1_.emplace_back(point<double>(-97.47267723083496, 35.36224605490395));
    geom1_.emplace_back(point<double>(-97.46323585510252, 35.34523530173256));
    geom1_.emplace_back(point<double>(-97.45963096618651, 35.36329598397908));
    geom1_.emplace_back(point<double>(-97.47550964355469, 35.369245324153866));
    line_string<double> geom2_;
    geom2_.emplace_back(point<double>(-10852395.511130, 4212951.024108));
    geom2_.emplace_back(point<double>(-10851497.376047, 4211403.174286));
    geom2_.emplace_back(point<double>(-10850608.795594, 4213218.553707));
    geom2_.emplace_back(point<double>(-10849557.786455, 4210896.778973));
    geom2_.emplace_back(point<double>(-10849156.492056, 4213361.873135));
    geom2_.emplace_back(point<double>(-10850924.098335, 4214174.016561));
    line_string<double> geom0_;
    geometry<double> geom0(geom0_);
    geometry<double> geom1(geom1_);
    geometry<double> geom2(geom2_);
    unsigned int err = 0;
    {
        // Reprojecting empty line string will return a geometry_empty
        geometry<double> new_geom = reproject_copy(geom0, proj_trans1, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
    }
    {
        // Test Standard Transform
        geometry<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        geometry<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        geometry<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        geometry<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        geometry<double> geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        geometry<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Polygon Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    polygon<double> geom1;
    // exterior
    geom1.emplace_back();
    geom1.back().emplace_back(point<double>(-97.62588500976562, 35.62939577711732));
    geom1.back().emplace_back(point<double>(-97.79067993164062, 35.43941441533686));
    geom1.back().emplace_back(point<double>(-97.60391235351562, 35.34425514918409));
    geom1.back().emplace_back(point<double>(-97.42813110351562, 35.48191987272801));
    geom1.back().emplace_back(point<double>(-97.62588500976562, 35.62939577711732));
    // interior
    geom1.emplace_back();
    geom1.back().emplace_back(point<double>(-97.66571044921875, 35.46849952318069));
    geom1.back().emplace_back(point<double>(-97.61489868164062, 35.54116627999813));
    geom1.back().emplace_back(point<double>(-97.53799438476562, 35.459551379037606));
    geom1.back().emplace_back(point<double>(-97.62451171875, 35.42598697382711));
    geom1.back().emplace_back(point<double>(-97.66571044921875, 35.46849952318069));
    polygon<double> geom2;
    // interior
    geom2.emplace_back();
    geom2.back().emplace_back(point<double>(-10867663.807530, 4249745.898599));
    geom2.back().emplace_back(point<double>(-10886008.694318, 4223757.308982));
    geom2.back().emplace_back(point<double>(-10865217.822625, 4210763.014174));
    geom2.back().emplace_back(point<double>(-10845649.943384, 4229566.523132));
    geom2.back().emplace_back(point<double>(-10867663.807530, 4249745.898599));
    // exterior
    geom2.emplace_back();
    geom2.back().emplace_back(point<double>(-10872097.155170, 4227732.034453));
    geom2.back().emplace_back(point<double>(-10866440.815077, 4237668.848130));
    geom2.back().emplace_back(point<double>(-10857879.867909, 4226509.042001));
    geom2.back().emplace_back(point<double>(-10867510.933473, 4221922.820303));
    geom2.back().emplace_back(point<double>(-10872097.155170, 4227732.034453));
    unsigned int err = 0;
    {
        // Test Standard Transform
        // Add extra vector to outer ring.
        geom1.emplace_back();
        REQUIRE(geom1.size() == 3);
        polygon<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        // Should remove the empty ring added to back of geom1
        REQUIRE(new_geom.size() == 2);
        assert_g_equal(new_geom, geom2);
        // Remove extra ring for future validity tests.
        geom1.pop_back();
        REQUIRE(geom1.size() == 2);
    }
    {
        // Transform in reverse
        polygon<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        polygon<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        polygon<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        polygon<double> geom3(geom1);
        geom3.emplace_back();
        REQUIRE(reproject(geom3, proj_trans1));
        // Should NOT remove the empty ring added to back of geom1
        REQUIRE(geom3.size() == 3);
        // Remove so asserts that geometries are the same
        geom3.pop_back();
        REQUIRE(geom3.size() == 2);
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        polygon<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Polygon Geometry Variant Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    polygon<double> geom1_;
    // exterior
    geom1_.emplace_back();
    geom1_.back().emplace_back(point<double>(-97.62588500976562, 35.62939577711732));
    geom1_.back().emplace_back(point<double>(-97.79067993164062, 35.43941441533686));
    geom1_.back().emplace_back(point<double>(-97.60391235351562, 35.34425514918409));
    geom1_.back().emplace_back(point<double>(-97.42813110351562, 35.48191987272801));
    geom1_.back().emplace_back(point<double>(-97.62588500976562, 35.62939577711732));
    // interior
    geom1_.emplace_back();
    geom1_.back().emplace_back(point<double>(-97.66571044921875, 35.46849952318069));
    geom1_.back().emplace_back(point<double>(-97.61489868164062, 35.54116627999813));
    geom1_.back().emplace_back(point<double>(-97.53799438476562, 35.459551379037606));
    geom1_.back().emplace_back(point<double>(-97.62451171875, 35.42598697382711));
    geom1_.back().emplace_back(point<double>(-97.66571044921875, 35.46849952318069));

    polygon<double> geom2_;
    // exterior
    geom2_.emplace_back();
    geom2_.back().emplace_back(point<double>(-10867663.807530, 4249745.898599));
    geom2_.back().emplace_back(point<double>(-10886008.694318, 4223757.308982));
    geom2_.back().emplace_back(point<double>(-10865217.822625, 4210763.014174));
    geom2_.back().emplace_back(point<double>(-10845649.943384, 4229566.523132));
    geom2_.back().emplace_back(point<double>(-10867663.807530, 4249745.898599));
    // interior
    geom2_.emplace_back();
    geom2_.back().emplace_back(point<double>(-10872097.155170, 4227732.034453));
    geom2_.back().emplace_back(point<double>(-10866440.815077, 4237668.848130));
    geom2_.back().emplace_back(point<double>(-10857879.867909, 4226509.042001));
    geom2_.back().emplace_back(point<double>(-10867510.933473, 4221922.820303));
    geom2_.back().emplace_back(point<double>(-10872097.155170, 4227732.034453));
    polygon<double> geom0_;
    geometry<double> geom0(geom0_);
    geometry<double> geom1(geom1_);
    geometry<double> geom2(geom2_);
    unsigned int err = 0;
    {
        // Reprojecting empty poly will return a geometry_empty
        geometry<double> new_geom = reproject_copy(geom0, proj_trans1, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
    }
    {
        // Test Standard Transform
        geometry<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        geometry<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        geometry<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        geometry<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        geometry<double> geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        geometry<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // END SECTION

SECTION("test_projection_4326_3857 - Multi_Point Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    multi_point<double> geom1;
    geom1.emplace_back(point<double>(-97.48872756958008, 35.360286150765084));
    geom1.emplace_back(point<double>(-97.48065948486328, 35.34894577151337));
    geom1.emplace_back(point<double>(-97.47267723083496, 35.36224605490395));
    geom1.emplace_back(point<double>(-97.46323585510252, 35.34523530173256));
    geom1.emplace_back(point<double>(-97.45963096618651, 35.36329598397908));
    geom1.emplace_back(point<double>(-97.47550964355469, 35.369245324153866));
    multi_point<double> geom2;
    geom2.emplace_back(point<double>(-10852395.511130, 4212951.024108));
    geom2.emplace_back(point<double>(-10851497.376047, 4211403.174286));
    geom2.emplace_back(point<double>(-10850608.795594, 4213218.553707));
    geom2.emplace_back(point<double>(-10849557.786455, 4210896.778973));
    geom2.emplace_back(point<double>(-10849156.492056, 4213361.873135));
    geom2.emplace_back(point<double>(-10850924.098335, 4214174.016561));
    unsigned int err = 0;
    {
        // Test Standard Transform
        multi_point<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        multi_point<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        multi_point<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        multi_point<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        multi_point<double> geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        multi_point<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Multi_Point Geometry Variant Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    multi_point<double> geom1_;
    geom1_.emplace_back(point<double>(-97.48872756958008, 35.360286150765084));
    geom1_.emplace_back(point<double>(-97.48065948486328, 35.34894577151337));
    geom1_.emplace_back(point<double>(-97.47267723083496, 35.36224605490395));
    geom1_.emplace_back(point<double>(-97.46323585510252, 35.34523530173256));
    geom1_.emplace_back(point<double>(-97.45963096618651, 35.36329598397908));
    geom1_.emplace_back(point<double>(-97.47550964355469, 35.369245324153866));
    multi_point<double> geom2_;
    geom2_.emplace_back(point<double>(-10852395.511130, 4212951.024108));
    geom2_.emplace_back(point<double>(-10851497.376047, 4211403.174286));
    geom2_.emplace_back(point<double>(-10850608.795594, 4213218.553707));
    geom2_.emplace_back(point<double>(-10849557.786455, 4210896.778973));
    geom2_.emplace_back(point<double>(-10849156.492056, 4213361.873135));
    geom2_.emplace_back(point<double>(-10850924.098335, 4214174.016561));
    multi_point<double> geom0_;
    geometry<double> geom0(geom0_);
    geometry<double> geom1(geom1_);
    geometry<double> geom2(geom2_);
    unsigned int err = 0;
    {
        // Reprojecting empty multi point will return a geometry_empty
        geometry<double> new_geom = reproject_copy(geom0, proj_trans1, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
    }
    {
        // Test Standard Transform
        geometry<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        geometry<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        geometry<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        geometry<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        geometry<double> geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        geometry<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Multi_Line_String Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    line_string<double> geom1a;
    geom1a.emplace_back(point<double>(-97.48872756958008, 35.360286150765084));
    geom1a.emplace_back(point<double>(-97.48065948486328, 35.34894577151337));
    geom1a.emplace_back(point<double>(-97.47267723083496, 35.36224605490395));
    geom1a.emplace_back(point<double>(-97.46323585510252, 35.34523530173256));
    geom1a.emplace_back(point<double>(-97.45963096618651, 35.36329598397908));
    geom1a.emplace_back(point<double>(-97.47550964355469, 35.369245324153866));
    multi_line_string<double> geom1;
    geom1.emplace_back(geom1a);
    line_string<double> geom2a;
    geom2a.emplace_back(point<double>(-10852395.511130, 4212951.024108));
    geom2a.emplace_back(point<double>(-10851497.376047, 4211403.174286));
    geom2a.emplace_back(point<double>(-10850608.795594, 4213218.553707));
    geom2a.emplace_back(point<double>(-10849557.786455, 4210896.778973));
    geom2a.emplace_back(point<double>(-10849156.492056, 4213361.873135));
    geom2a.emplace_back(point<double>(-10850924.098335, 4214174.016561));
    multi_line_string<double> geom2;
    geom2.emplace_back(geom2a);
    unsigned int err = 0;
    {
        // Prior to test add an empty line_string to the the multi_line_string
        // this should be removed.
        geom1.emplace_back();
        REQUIRE(geom1.size() == 2);
        // Test Standard Transform
        multi_line_string<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.size() == 1);
        // Restore geom1 removing empty for later tests.
        geom1.pop_back();
        REQUIRE(geom1.size() == 1);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        multi_line_string<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        multi_line_string<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        multi_line_string<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        multi_line_string<double> geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        multi_line_string<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Multi_Line_String Geometry Variant Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    line_string<double> geom1a_;
    geom1a_.emplace_back(point<double>(-97.48872756958008, 35.360286150765084));
    geom1a_.emplace_back(point<double>(-97.48065948486328, 35.34894577151337));
    geom1a_.emplace_back(point<double>(-97.47267723083496, 35.36224605490395));
    geom1a_.emplace_back(point<double>(-97.46323585510252, 35.34523530173256));
    geom1a_.emplace_back(point<double>(-97.45963096618651, 35.36329598397908));
    geom1a_.emplace_back(point<double>(-97.47550964355469, 35.369245324153866));
    multi_line_string<double> geom1_;
    geom1_.emplace_back(geom1a_);
    line_string<double> geom2a_;
    geom2a_.emplace_back(point<double>(-10852395.511130, 4212951.024108));
    geom2a_.emplace_back(point<double>(-10851497.376047, 4211403.174286));
    geom2a_.emplace_back(point<double>(-10850608.795594, 4213218.553707));
    geom2a_.emplace_back(point<double>(-10849557.786455, 4210896.778973));
    geom2a_.emplace_back(point<double>(-10849156.492056, 4213361.873135));
    geom2a_.emplace_back(point<double>(-10850924.098335, 4214174.016561));
    multi_line_string<double> geom2_;
    geom2_.emplace_back(geom2a_);
    multi_line_string<double> geom0_;
    geometry<double> geom0(geom0_);
    geometry<double> geom1(geom1_);
    geometry<double> geom2(geom2_);
    unsigned int err = 0;
    {
        // Reprojecting empty line string will return a geometry_empty
        geometry<double> new_geom = reproject_copy(geom0, proj_trans1, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
    }
    {
        // Test Standard Transform
        geometry<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        geometry<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        geometry<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        geometry<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        geometry<double> geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        geometry<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Multi_Polygon Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    polygon<double> geom1a;
    // exterior
    geom1a.emplace_back();
    geom1a.back().emplace_back(point<double>(-97.62588500976562, 35.62939577711732));
    geom1a.back().emplace_back(point<double>(-97.79067993164062, 35.43941441533686));
    geom1a.back().emplace_back(point<double>(-97.60391235351562, 35.34425514918409));
    geom1a.back().emplace_back(point<double>(-97.42813110351562, 35.48191987272801));
    geom1a.back().emplace_back(point<double>(-97.62588500976562, 35.62939577711732));
    // interior
    geom1a.emplace_back();
    geom1a.back().emplace_back(point<double>(-97.66571044921875, 35.46849952318069));
    geom1a.back().emplace_back(point<double>(-97.61489868164062, 35.54116627999813));
    geom1a.back().emplace_back(point<double>(-97.53799438476562, 35.459551379037606));
    geom1a.back().emplace_back(point<double>(-97.62451171875, 35.42598697382711));
    geom1a.back().emplace_back(point<double>(-97.66571044921875, 35.46849952318069));
    multi_polygon<double> geom1;
    geom1.emplace_back(geom1a);
    polygon<double> geom2a;
    // exterior
    geom2a.emplace_back();
    geom2a.back().emplace_back(point<double>(-10867663.807530, 4249745.898599));
    geom2a.back().emplace_back(point<double>(-10886008.694318, 4223757.308982));
    geom2a.back().emplace_back(point<double>(-10865217.822625, 4210763.014174));
    geom2a.back().emplace_back(point<double>(-10845649.943384, 4229566.523132));
    geom2a.back().emplace_back(point<double>(-10867663.807530, 4249745.898599));
    // interior
    geom2a.emplace_back();
    geom2a.back().emplace_back(point<double>(-10872097.155170, 4227732.034453));
    geom2a.back().emplace_back(point<double>(-10866440.815077, 4237668.848130));
    geom2a.back().emplace_back(point<double>(-10857879.867909, 4226509.042001));
    geom2a.back().emplace_back(point<double>(-10867510.933473, 4221922.820303));
    geom2a.back().emplace_back(point<double>(-10872097.155170, 4227732.034453));
    multi_polygon<double> geom2;
    geom2.emplace_back(geom2a);
    unsigned int err = 0;
    {
        // Test Standard Transform
        // Add extra polygon to outer ring.
        geom1.emplace_back();
        REQUIRE(geom1.size() == 2);
        multi_polygon<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        // Should remove the empty polygon added to back of geom1
        REQUIRE(new_geom.size() == 1);
        assert_g_equal(new_geom, geom2);
        // Remove extra ring for future validity tests.
        geom1.pop_back();
        REQUIRE(geom1.size() == 1);
    }
    {
        // Transform in reverse
        multi_polygon<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        multi_polygon<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        multi_polygon<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        multi_polygon<double> geom3(geom1);
        geom3.emplace_back();
        REQUIRE(reproject(geom3, proj_trans1));
        // Should NOT remove the empty ring added to back of geom1
        REQUIRE(geom3.size() == 2);
        // Remove so asserts that geometries are the same
        geom3.pop_back();
        REQUIRE(geom3.size() == 1);
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        multi_polygon<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Multi_Polygon Geometry Variant Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    polygon<double> geom1a_;
    // exterior
    geom1a_.emplace_back();
    geom1a_.back().emplace_back(point<double>(-97.62588500976562, 35.62939577711732));
    geom1a_.back().emplace_back(point<double>(-97.79067993164062, 35.43941441533686));
    geom1a_.back().emplace_back(point<double>(-97.60391235351562, 35.34425514918409));
    geom1a_.back().emplace_back(point<double>(-97.42813110351562, 35.48191987272801));
    geom1a_.back().emplace_back(point<double>(-97.62588500976562, 35.62939577711732));
    // interior
    geom1a_.emplace_back();
    geom1a_.back().emplace_back(point<double>(-97.66571044921875, 35.46849952318069));
    geom1a_.back().emplace_back(point<double>(-97.61489868164062, 35.54116627999813));
    geom1a_.back().emplace_back(point<double>(-97.53799438476562, 35.459551379037606));
    geom1a_.back().emplace_back(point<double>(-97.62451171875, 35.42598697382711));
    geom1a_.back().emplace_back(point<double>(-97.66571044921875, 35.46849952318069));
    multi_polygon<double> geom1_;
    geom1_.emplace_back(geom1a_);
    polygon<double> geom2a_;
    // exterior
    geom2a_.emplace_back();
    geom2a_.back().emplace_back(point<double>(-10867663.807530, 4249745.898599));
    geom2a_.back().emplace_back(point<double>(-10886008.694318, 4223757.308982));
    geom2a_.back().emplace_back(point<double>(-10865217.822625, 4210763.014174));
    geom2a_.back().emplace_back(point<double>(-10845649.943384, 4229566.523132));
    geom2a_.back().emplace_back(point<double>(-10867663.807530, 4249745.898599));
    // interior
    geom2a_.emplace_back();
    geom2a_.back().emplace_back(point<double>(-10872097.155170, 4227732.034453));
    geom2a_.back().emplace_back(point<double>(-10866440.815077, 4237668.848130));
    geom2a_.back().emplace_back(point<double>(-10857879.867909, 4226509.042001));
    geom2a_.back().emplace_back(point<double>(-10867510.933473, 4221922.820303));
    geom2a_.back().emplace_back(point<double>(-10872097.155170, 4227732.034453));
    multi_polygon<double> geom2_;
    geom2_.emplace_back(geom2a_);
    multi_polygon<double> geom0_;
    geometry<double> geom0(geom0_);
    geometry<double> geom1(geom1_);
    geometry<double> geom2(geom2_);
    unsigned int err = 0;
    {
        // Reprojecting empty poly will return a geometry_empty
        geometry<double> new_geom = reproject_copy(geom0, proj_trans1, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
    }
    {
        // Test Standard Transform
        geometry<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        geometry<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        geometry<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        geometry<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        geometry<double> geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        geometry<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // END SECTION

SECTION("test_projection_4326_3857 - Geometry Collection Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    polygon<double> geom1a;
    // exterior
    geom1a.emplace_back();
    geom1a.back().emplace_back(point<double>(-97.62588500976562, 35.62939577711732));
    geom1a.back().emplace_back(point<double>(-97.79067993164062, 35.43941441533686));
    geom1a.back().emplace_back(point<double>(-97.60391235351562, 35.34425514918409));
    geom1a.back().emplace_back(point<double>(-97.42813110351562, 35.48191987272801));
    geom1a.back().emplace_back(point<double>(-97.62588500976562, 35.62939577711732));
    // interior
    geom1a.emplace_back();
    geom1a.back().emplace_back(point<double>(-97.66571044921875, 35.46849952318069));
    geom1a.back().emplace_back(point<double>(-97.61489868164062, 35.54116627999813));
    geom1a.back().emplace_back(point<double>(-97.53799438476562, 35.459551379037606));
    geom1a.back().emplace_back(point<double>(-97.62451171875, 35.42598697382711));
    geom1a.back().emplace_back(point<double>(-97.66571044921875, 35.46849952318069));
    geometry_collection<double> geom1;
    geom1.emplace_back(geometry<double>(geom1a));
    polygon<double> geom2a;
    // exerior
    geom2a.emplace_back();
    geom2a.back().emplace_back(point<double>(-10867663.807530, 4249745.898599));
    geom2a.back().emplace_back(point<double>(-10886008.694318, 4223757.308982));
    geom2a.back().emplace_back(point<double>(-10865217.822625, 4210763.014174));
    geom2a.back().emplace_back(point<double>(-10845649.943384, 4229566.523132));
    geom2a.back().emplace_back(point<double>(-10867663.807530, 4249745.898599));
    // interior
    geom2a.emplace_back();
    geom2a.back().emplace_back(point<double>(-10872097.155170, 4227732.034453));
    geom2a.back().emplace_back(point<double>(-10866440.815077, 4237668.848130));
    geom2a.back().emplace_back(point<double>(-10857879.867909, 4226509.042001));
    geom2a.back().emplace_back(point<double>(-10867510.933473, 4221922.820303));
    geom2a.back().emplace_back(point<double>(-10872097.155170, 4227732.034453));
    geometry_collection<double> geom2;
    geom2.emplace_back(geometry<double>(geom2a));
    unsigned int err = 0;
    {
        // Test Standard Transform
        // Add extra geometry to outer ring.
        geom1.emplace_back();
        REQUIRE(geom1.size() == 2);
        geometry_collection<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        // Should remove the empty geometry added to back of geom1
        REQUIRE(new_geom.size() == 1);
        assert_g_equal(new_geom, geom2);
        // Remove extra ring for future validity tests.
        geom1.pop_back();
        REQUIRE(geom1.size() == 1);
    }
    {
        // Transform in reverse
        geometry_collection<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        geometry_collection<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        geometry_collection<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        geometry_collection<double> geom3(geom1);
        geom3.emplace_back();
        REQUIRE(reproject(geom3, proj_trans1));
        // Should NOT remove the empty ring added to back of geom1
        REQUIRE(geom3.size() == 2);
        // Remove so asserts that geometries are the same
        geom3.pop_back();
        REQUIRE(geom3.size() == 1);
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        geometry_collection<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Geometry Collection Variant Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4326");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    polygon<double> geom1a_;
    // exterior
    geom1a_.emplace_back();
    geom1a_.back().emplace_back(point<double>(-97.62588500976562, 35.62939577711732));
    geom1a_.back().emplace_back(point<double>(-97.79067993164062, 35.43941441533686));
    geom1a_.back().emplace_back(point<double>(-97.60391235351562, 35.34425514918409));
    geom1a_.back().emplace_back(point<double>(-97.42813110351562, 35.48191987272801));
    geom1a_.back().emplace_back(point<double>(-97.62588500976562, 35.62939577711732));
    // interior
    geom1a_.emplace_back();
    geom1a_.back().emplace_back(point<double>(-97.66571044921875, 35.46849952318069));
    geom1a_.back().emplace_back(point<double>(-97.61489868164062, 35.54116627999813));
    geom1a_.back().emplace_back(point<double>(-97.53799438476562, 35.459551379037606));
    geom1a_.back().emplace_back(point<double>(-97.62451171875, 35.42598697382711));
    geom1a_.back().emplace_back(point<double>(-97.66571044921875, 35.46849952318069));
    geometry_collection<double> geom1_;
    geom1_.emplace_back(geometry<double>(geom1a_));
    polygon<double> geom2a_;
    // exterior
    geom2a_.emplace_back();
    geom2a_.back().emplace_back(point<double>(-10867663.807530, 4249745.898599));
    geom2a_.back().emplace_back(point<double>(-10886008.694318, 4223757.308982));
    geom2a_.back().emplace_back(point<double>(-10865217.822625, 4210763.014174));
    geom2a_.back().emplace_back(point<double>(-10845649.943384, 4229566.523132));
    geom2a_.back().emplace_back(point<double>(-10867663.807530, 4249745.898599));
    // interior
    geom2a_.emplace_back();
    geom2a_.back().emplace_back(point<double>(-10872097.155170, 4227732.034453));
    geom2a_.back().emplace_back(point<double>(-10866440.815077, 4237668.848130));
    geom2a_.back().emplace_back(point<double>(-10857879.867909, 4226509.042001));
    geom2a_.back().emplace_back(point<double>(-10867510.933473, 4221922.820303));
    geom2a_.back().emplace_back(point<double>(-10872097.155170, 4227732.034453));
    geometry_collection<double> geom2_;
    geom2_.emplace_back(geometry<double>(geom2a_));
    multi_polygon<double> geom0_;
    geometry<double> geom0(geom0_);
    geometry<double> geom1(geom1_);
    geometry<double> geom2(geom2_);
    unsigned int err = 0;
    {
        // Reprojecting empty poly will return a geometry_empty
        geometry<double> new_geom = reproject_copy(geom0, proj_trans1, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
    }
    {
        // Test Standard Transform
        geometry<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        geometry<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        geometry<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        geometry<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        geometry<double> geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        geometry<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // END SECTION

#ifdef MAPNIK_USE_PROJ4
SECTION("test_projection_4269_3857 - Line_String Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4269");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    line_string<double> geom1;
    geom1.emplace_back(point<double>(-97.48872756958008, 35.360286150765084));
    geom1.emplace_back(point<double>(-97.48065948486328, 35.34894577151337));
    geom1.emplace_back(point<double>(-97.47267723083496, 35.36224605490395));
    geom1.emplace_back(point<double>(-97.46323585510252, 35.34523530173256));
    geom1.emplace_back(point<double>(-97.45963096618651, 35.36329598397908));
    geom1.emplace_back(point<double>(-97.47550964355469, 35.369245324153866));
    line_string<double> geom2;
    geom2.emplace_back(point<double>(-10852395.511130, 4212951.024108));
    geom2.emplace_back(point<double>(-10851497.376047, 4211403.174286));
    geom2.emplace_back(point<double>(-10850608.795594, 4213218.553707));
    geom2.emplace_back(point<double>(-10849557.786455, 4210896.778973));
    geom2.emplace_back(point<double>(-10849156.492056, 4213361.873135));
    geom2.emplace_back(point<double>(-10850924.098335, 4214174.016561));
    unsigned int err = 0;
    {
        // Test Standard Transform
        line_string<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        line_string<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        line_string<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        line_string<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        line_string<double> geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        line_string<double> geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section


SECTION("test_projection_4269_3857 - Point Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("epsg:4269");
    mapnik::projection dest("epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    point<double> geom1(-97.552175, 35.522895);
    point<double> geom2(-10859458.446776, 4235169.496066);
    unsigned int err = 0;
    {
        // Test Standard Transform
        point<double> new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        point<double> new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        point<double> new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        point<double> new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        point<double> geom3(-97.552175, 35.522895);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse - back
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        point<double> geom3(-97.552175, 35.522895);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

#endif  // MAPNIK_USE_PROJ4

} // End Testcase
