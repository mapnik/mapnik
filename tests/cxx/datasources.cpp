
#include "catch.hpp"

#include <mapnik/datasource_cache.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/util/fs.hpp>

TEST_CASE("datasources") {

SECTION("hello world") {

    std::string plugin("./plugins/input/templates/hello.input");
    if (mapnik::util::exists(plugin))
    {
        try
        {
            mapnik::datasource_cache::instance().register_datasource(plugin);
            mapnik::parameters p;
            p["type"]="hello";
            mapnik::datasource_ptr ds = mapnik::datasource_cache::instance().create(p);
            mapnik::box2d<double> bbox = ds->envelope();
            mapnik::query q(bbox);
            mapnik::featureset_ptr fs = ds->features(q);
            REQUIRE( fs != mapnik::featureset_ptr() );
            mapnik::feature_ptr feat1 = fs->next();
            REQUIRE( feat1 != mapnik::feature_ptr() );
            mapnik::feature_ptr feat2 = fs->next();
            REQUIRE( feat2 != mapnik::feature_ptr() );
            REQUIRE( fs->next() == mapnik::feature_ptr() );
            REQUIRE( feat1->id() == static_cast<mapnik::value_integer>(1) );
            REQUIRE( feat2->id() == static_cast<mapnik::value_integer>(2) );
            auto const& geom1 = feat1->get_geometry();
            REQUIRE( geom1.is<mapnik::new_geometry::point>() );
            auto const& point = mapnik::util::get<mapnik::new_geometry::point>(geom1);
            REQUIRE( point.x == bbox.center().x );
            REQUIRE( point.y == bbox.center().y );
            auto const& geom2 = feat2->get_geometry();
            REQUIRE( geom2.is<mapnik::new_geometry::line_string>() );
            auto const& line = mapnik::util::get<mapnik::new_geometry::line_string>(geom2);
            REQUIRE( line.size() == 4 );
            REQUIRE( line[0].x ==  bbox.minx() );
            REQUIRE( line[0].y ==  bbox.maxy() );
        }
        catch (std::exception const& ex)
        {
            FAIL(ex.what());
        }
    }
    else
    {
        WARN( std::string("could not register ") + plugin );
    }

}

}
