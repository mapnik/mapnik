#include "catch.hpp"
#include "geometry_equal.hpp"

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/geometry_reprojection.hpp>
#include <mapnik/geometry_correct.hpp>

// std
#include <iostream>

TEST_CASE("geometry reprojection") {

SECTION("test_projection_4326_3857 - Empty Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
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
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans(source, dest);
    {
        geometry geom(std::move(geometry_empty()));
        unsigned int err = 0;
        // Test Standard Transform
        geometry new_geom = reproject_copy(geom, proj_trans, err);
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
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    point geom1(-97.552175, 35.522895);
    point geom2(-10859458.446776, 4235169.496066);
    unsigned int err = 0;
    {
        // Test Standard Transform
        point new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        point new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        point new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        point new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        point geom3(-97.552175, 35.522895);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse - back
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        point geom3(-97.552175, 35.522895);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Point Geometry Variant Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    double x1 = -97.552175;
    double y1 = 35.522895;
    double x2 = -10859458.446776;
    double y2 = 4235169.496066;
    geometry geom1(point(x1, y1));
    geometry geom2(point(x2, y2));
    unsigned int err = 0;
    {
        // Test Standard Transform
        geometry new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        geometry new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        geometry new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        geometry new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        geometry geom3(point(-97.552175, 35.522895));
        // Transform in place 
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse - back
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        geometry geom3(point(-97.552175, 35.522895));
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
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    line_string geom1;
    geom1.emplace_back(point(-97.48872756958008, 35.360286150765084));
    geom1.emplace_back(point(-97.48065948486328, 35.34894577151337));
    geom1.emplace_back(point(-97.47267723083496, 35.36224605490395));
    geom1.emplace_back(point(-97.46323585510252, 35.34523530173256));
    geom1.emplace_back(point(-97.45963096618651, 35.36329598397908));
    geom1.emplace_back(point(-97.47550964355469, 35.369245324153866));
    line_string geom2;
    geom2.emplace_back(point(-10852395.511130, 4212951.024108));
    geom2.emplace_back(point(-10851497.376047, 4211403.174286));
    geom2.emplace_back(point(-10850608.795594, 4213218.553707));
    geom2.emplace_back(point(-10849557.786455, 4210896.778973));
    geom2.emplace_back(point(-10849156.492056, 4213361.873135));
    geom2.emplace_back(point(-10850924.098335, 4214174.016561));
    unsigned int err = 0;
    {
        // Test Standard Transform
        line_string new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        line_string new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        line_string new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        line_string new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        line_string geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        line_string geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Line_String Geometry Variant Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    line_string geom1_;
    geom1_.emplace_back(point(-97.48872756958008, 35.360286150765084));
    geom1_.emplace_back(point(-97.48065948486328, 35.34894577151337));
    geom1_.emplace_back(point(-97.47267723083496, 35.36224605490395));
    geom1_.emplace_back(point(-97.46323585510252, 35.34523530173256));
    geom1_.emplace_back(point(-97.45963096618651, 35.36329598397908));
    geom1_.emplace_back(point(-97.47550964355469, 35.369245324153866));
    line_string geom2_;
    geom2_.emplace_back(point(-10852395.511130, 4212951.024108));
    geom2_.emplace_back(point(-10851497.376047, 4211403.174286));
    geom2_.emplace_back(point(-10850608.795594, 4213218.553707));
    geom2_.emplace_back(point(-10849557.786455, 4210896.778973));
    geom2_.emplace_back(point(-10849156.492056, 4213361.873135));
    geom2_.emplace_back(point(-10850924.098335, 4214174.016561));
    line_string geom0_;
    geometry geom0(geom0_);
    geometry geom1(geom1_);
    geometry geom2(geom2_);
    unsigned int err = 0;
    {
        // Reprojecting empty line string will return a geometry_empty
        geometry new_geom = reproject_copy(geom0, proj_trans1, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
    }
    {
        // Test Standard Transform
        geometry new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        geometry new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        geometry new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        geometry new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        geometry geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        geometry geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section
    
SECTION("test_projection_4326_3857 - Polygon Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    polygon geom1;
    geom1.exterior_ring.emplace_back(point(-97.62588500976562, 35.62939577711732));
    geom1.exterior_ring.emplace_back(point(-97.79067993164062, 35.43941441533686));
    geom1.exterior_ring.emplace_back(point(-97.60391235351562, 35.34425514918409));
    geom1.exterior_ring.emplace_back(point(-97.42813110351562, 35.48191987272801));
    geom1.exterior_ring.emplace_back(point(-97.62588500976562, 35.62939577711732));
    geom1.interior_rings.emplace_back();
    geom1.interior_rings.back().emplace_back(point(-97.66571044921875, 35.46849952318069));
    geom1.interior_rings.back().emplace_back(point(-97.61489868164062, 35.54116627999813));
    geom1.interior_rings.back().emplace_back(point(-97.53799438476562, 35.459551379037606));
    geom1.interior_rings.back().emplace_back(point(-97.62451171875, 35.42598697382711));
    geom1.interior_rings.back().emplace_back(point(-97.66571044921875, 35.46849952318069));
    polygon geom2;
    geom2.exterior_ring.emplace_back(point(-10867663.807530, 4249745.898599));
    geom2.exterior_ring.emplace_back(point(-10886008.694318, 4223757.308982));
    geom2.exterior_ring.emplace_back(point(-10865217.822625, 4210763.014174));
    geom2.exterior_ring.emplace_back(point(-10845649.943384, 4229566.523132));
    geom2.exterior_ring.emplace_back(point(-10867663.807530, 4249745.898599));
    geom2.interior_rings.emplace_back();
    geom2.interior_rings.back().emplace_back(point(-10872097.155170, 4227732.034453));
    geom2.interior_rings.back().emplace_back(point(-10866440.815077, 4237668.848130));
    geom2.interior_rings.back().emplace_back(point(-10857879.867909, 4226509.042001));
    geom2.interior_rings.back().emplace_back(point(-10867510.933473, 4221922.820303));
    geom2.interior_rings.back().emplace_back(point(-10872097.155170, 4227732.034453));
    unsigned int err = 0;
    {
        // Test Standard Transform
        // Add extra vector to outer ring.
        geom1.interior_rings.emplace_back();
        REQUIRE(geom1.interior_rings.size() == 2);
        polygon new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        // Should remove the empty ring added to back of geom1
        REQUIRE(new_geom.interior_rings.size() == 1);
        assert_g_equal(new_geom, geom2);
        // Remove extra ring for future validity tests.
        geom1.interior_rings.pop_back();
        REQUIRE(geom1.interior_rings.size() == 1);
    }
    {
        // Transform in reverse
        polygon new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        polygon new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        polygon new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        polygon geom3(geom1);
        geom3.interior_rings.emplace_back();
        REQUIRE(reproject(geom3, proj_trans1));
        // Should NOT remove the empty ring added to back of geom1
        REQUIRE(geom3.interior_rings.size() == 2);
        // Remove so asserts that geometries are the same
        geom3.interior_rings.pop_back();
        REQUIRE(geom3.interior_rings.size() == 1);
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        polygon geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Polygon Geometry Variant Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    polygon geom1_;
    geom1_.exterior_ring.emplace_back(point(-97.62588500976562, 35.62939577711732));
    geom1_.exterior_ring.emplace_back(point(-97.79067993164062, 35.43941441533686));
    geom1_.exterior_ring.emplace_back(point(-97.60391235351562, 35.34425514918409));
    geom1_.exterior_ring.emplace_back(point(-97.42813110351562, 35.48191987272801));
    geom1_.exterior_ring.emplace_back(point(-97.62588500976562, 35.62939577711732));
    geom1_.interior_rings.emplace_back();
    geom1_.interior_rings.back().emplace_back(point(-97.66571044921875, 35.46849952318069));
    geom1_.interior_rings.back().emplace_back(point(-97.61489868164062, 35.54116627999813));
    geom1_.interior_rings.back().emplace_back(point(-97.53799438476562, 35.459551379037606));
    geom1_.interior_rings.back().emplace_back(point(-97.62451171875, 35.42598697382711));
    geom1_.interior_rings.back().emplace_back(point(-97.66571044921875, 35.46849952318069));
    polygon geom2_;
    geom2_.exterior_ring.emplace_back(point(-10867663.807530, 4249745.898599));
    geom2_.exterior_ring.emplace_back(point(-10886008.694318, 4223757.308982));
    geom2_.exterior_ring.emplace_back(point(-10865217.822625, 4210763.014174));
    geom2_.exterior_ring.emplace_back(point(-10845649.943384, 4229566.523132));
    geom2_.exterior_ring.emplace_back(point(-10867663.807530, 4249745.898599));
    geom2_.interior_rings.emplace_back();
    geom2_.interior_rings.back().emplace_back(point(-10872097.155170, 4227732.034453));
    geom2_.interior_rings.back().emplace_back(point(-10866440.815077, 4237668.848130));
    geom2_.interior_rings.back().emplace_back(point(-10857879.867909, 4226509.042001));
    geom2_.interior_rings.back().emplace_back(point(-10867510.933473, 4221922.820303));
    geom2_.interior_rings.back().emplace_back(point(-10872097.155170, 4227732.034453));
    polygon geom0_;
    geometry geom0(geom0_);
    geometry geom1(geom1_);
    geometry geom2(geom2_);
    unsigned int err = 0;
    {
        // Reprojecting empty poly will return a geometry_empty
        geometry new_geom = reproject_copy(geom0, proj_trans1, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
    }
    {
        // Test Standard Transform
        geometry new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        geometry new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        geometry new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        geometry new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        geometry geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        geometry geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // END SECTION

SECTION("test_projection_4326_3857 - Multi_Point Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    multi_point geom1;
    geom1.emplace_back(point(-97.48872756958008, 35.360286150765084));
    geom1.emplace_back(point(-97.48065948486328, 35.34894577151337));
    geom1.emplace_back(point(-97.47267723083496, 35.36224605490395));
    geom1.emplace_back(point(-97.46323585510252, 35.34523530173256));
    geom1.emplace_back(point(-97.45963096618651, 35.36329598397908));
    geom1.emplace_back(point(-97.47550964355469, 35.369245324153866));
    multi_point geom2;
    geom2.emplace_back(point(-10852395.511130, 4212951.024108));
    geom2.emplace_back(point(-10851497.376047, 4211403.174286));
    geom2.emplace_back(point(-10850608.795594, 4213218.553707));
    geom2.emplace_back(point(-10849557.786455, 4210896.778973));
    geom2.emplace_back(point(-10849156.492056, 4213361.873135));
    geom2.emplace_back(point(-10850924.098335, 4214174.016561));
    unsigned int err = 0;
    {
        // Test Standard Transform
        multi_point new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        multi_point new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        multi_point new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        multi_point new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        multi_point geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        multi_point geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section

SECTION("test_projection_4326_3857 - Multi_Point Geometry Variant Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans1(source, dest);
    mapnik::proj_transform proj_trans2(dest, source);
    multi_point geom1_;
    geom1_.emplace_back(point(-97.48872756958008, 35.360286150765084));
    geom1_.emplace_back(point(-97.48065948486328, 35.34894577151337));
    geom1_.emplace_back(point(-97.47267723083496, 35.36224605490395));
    geom1_.emplace_back(point(-97.46323585510252, 35.34523530173256));
    geom1_.emplace_back(point(-97.45963096618651, 35.36329598397908));
    geom1_.emplace_back(point(-97.47550964355469, 35.369245324153866));
    multi_point geom2_;
    geom2_.emplace_back(point(-10852395.511130, 4212951.024108));
    geom2_.emplace_back(point(-10851497.376047, 4211403.174286));
    geom2_.emplace_back(point(-10850608.795594, 4213218.553707));
    geom2_.emplace_back(point(-10849557.786455, 4210896.778973));
    geom2_.emplace_back(point(-10849156.492056, 4213361.873135));
    geom2_.emplace_back(point(-10850924.098335, 4214174.016561));
    multi_point geom0_;
    geometry geom0(geom0_);
    geometry geom1(geom1_);
    geometry geom2(geom2_);
    unsigned int err = 0;
    {
        // Reprojecting empty multi point will return a geometry_empty
        geometry new_geom = reproject_copy(geom0, proj_trans1, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
    }
    {
        // Test Standard Transform
        geometry new_geom = reproject_copy(geom1, proj_trans1, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform in reverse
        geometry new_geom = reproject_copy(geom2, proj_trans2, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform providing projections not transfrom
        geometry new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom2);
    }
    {
        // Transform providing projections in reverse
        geometry new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        assert_g_equal(new_geom, geom1);
    }
    {
        // Transform in place
        geometry geom3(geom1);
        REQUIRE(reproject(geom3, proj_trans1));
        assert_g_equal(geom3, geom2);
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans2));
        assert_g_equal(geom3, geom1);
    }
    {
        // Transform in place providing projections
        geometry geom3(geom1);
        REQUIRE(reproject(geom3, source, dest));
        assert_g_equal(geom3, geom2);
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        assert_g_equal(geom3, geom1);
    }
} // End Section


/*
        for (auto const& p : new_geom)
        {
            std::clog << "geom2.emplace_back(point(" << std::fixed << p.x << ", " << std::fixed << p.y << "));" << std::endl;
        }
    {
        line_string line;
        line.add_coord(0,0);
        line.add_coord(1,1);
        line.add_coord(2,2);
        geometry geom(line);
        REQUIRE( mapnik::hit_test(geom,0,0,1.5) );
    }
    {
        line_string line;
        line.add_coord(0,0);
        line.add_coord(1,1);
        line.add_coord(2,2);
        multi_line_string multi_line;
        multi_line.emplace_back(std::move(line));
        geometry geom(multi_line);
        REQUIRE( mapnik::hit_test(geom,0,0,1.5) );
    }
    {
        polygon poly;
        linear_ring ring;
        ring.add_coord(0,0);
        ring.add_coord(-10,0);
        ring.add_coord(-10,10);
        ring.add_coord(0,10);
        ring.add_coord(0,0);
        poly.set_exterior_ring(std::move(ring));
        geometry geom(poly);
        REQUIRE( mapnik::hit_test(geom,-5,5,0) );

        multi_polygon mp;
        mp.push_back(poly);
        geometry geom_mp(mp);
        REQUIRE( mapnik::hit_test(geom_mp,-5,5,0) );

        correct(geom);
        REQUIRE( mapnik::hit_test(geom,-5,5,0) );
        correct(geom_mp);
        REQUIRE( mapnik::hit_test(geom_mp,-5,5,0) );

        geometry_collection gc;
        REQUIRE( !mapnik::hit_test(geometry(gc),-5,5,0) );
        gc.push_back(geom_mp);
        REQUIRE( mapnik::hit_test(geometry(gc),-5,5,0) );
        REQUIRE( !mapnik::hit_test(geometry(gc),-50,-50,0) );
        gc.emplace_back(point(-50,-50));
        REQUIRE( mapnik::hit_test(geometry(gc),-50,-50,0) );
    }

    {
        // polygon with hole
        polygon poly;
        linear_ring ring;
        ring.add_coord(0,0);
        ring.add_coord(-10,0);
        ring.add_coord(-10,10);
        ring.add_coord(0,10);
        ring.add_coord(0,0);
        poly.set_exterior_ring(std::move(ring));
        linear_ring hole;
        hole.add_coord(-7,7);
        hole.add_coord(-7,3);
        hole.add_coord(-3,3);
        hole.add_coord(-3,7);
        hole.add_coord(-7,7);
        poly.add_hole(std::move(hole));
        geometry geom(poly);
        REQUIRE( !mapnik::hit_test(geom,-5,5,0) );
        // add another hole inside the first hole
        // which should be considered a hit
        linear_ring fill;
        fill.add_coord(-6,4);
        fill.add_coord(-6,6);
        fill.add_coord(-4,6);
        fill.add_coord(-4,4);
        fill.add_coord(-6,4);
        poly.add_hole(std::move(fill));
        REQUIRE( mapnik::hit_test(geometry(poly),-5,5,0) );
    }*/

} // End Testcase
