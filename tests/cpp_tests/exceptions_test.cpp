#include <boost/version.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/make_shared.hpp>
#include <iostream>
#include <mapnik/projection.hpp>
#include <mapnik/map.hpp>
#include <mapnik/save_map.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/params.hpp>

extern "C" {
#include <sqlite3.h>
}

int main( int, char*[] )
{
    try {
        mapnik::projection srs("foo");
        // to avoid unused variable warning
        srs.params();
        BOOST_TEST(false);
    } catch (...) {
        BOOST_TEST(true);
    }

    mapnik::Map map(256,256);
    mapnik::rule r;
    r.set_filter(mapnik::parse_expression("[foo]='bar'"));
    mapnik::feature_type_style style;
    style.add_rule(r);
    map.insert_style("style",style);

    std::string csv_plugin("./plugins/input/csv.input");
    if (boost::filesystem::exists(csv_plugin)) {
        try {
            mapnik::datasource_cache::instance().register_datasource(csv_plugin);
            mapnik::parameters p;
            p["type"]="csv";
            p["inline"]="x,y\n0,0";
            mapnik::datasource_ptr ds = mapnik::datasource_cache::instance().create(p);
            //mapnik::datasource_ptr ds = boost::make_shared<mapnik::memory_datasource>();
            //mapnik::context_ptr ctx = boost::make_shared<mapnik::context_type>();
            //mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, 1));
            //mapnik::memory_datasource *mem_ds = dynamic_cast<mapnik::memory_datasource *>(ds.get());
            //mem_ds->push(feature);
            mapnik::layer l("layer");
            l.set_datasource(ds);
            l.add_style("style");
            mapnik::Map m = map;
            m.addLayer(l);
            m.zoom_all();
            mapnik::image_32 im(m.width(),m.height());
            mapnik::agg_renderer<mapnik::image_32> ren(m,im);
            //std::clog << mapnik::save_map_to_string(m) << "\n";
            BOOST_TEST(true);
            // should throw here
            ren.apply();
            BOOST_TEST(false);
        } catch (...) {
            BOOST_TEST(true);
        }
    }

    std::string shape_plugin("./plugins/input/shape.input");
    if (boost::filesystem::exists(shape_plugin)) {
        try {
            mapnik::datasource_cache::instance().register_datasource(shape_plugin);
            mapnik::parameters p2;
            p2["type"]="shape";
            p2["file"]="foo";
            mapnik::datasource_cache::instance().create(p2);
            BOOST_TEST(false);
        } catch (...) {
            BOOST_TEST(true);
        }
    }

    /*
    // not working, oddly segfaults valgrind
    try {
        sqlite3_initialize();
        // http://stackoverflow.com/questions/11107703/sqlite3-sigsegvs-with-valgrind
        sqlite3_config(SQLITE_CONFIG_HEAP, malloc (1024*1024), 1024*1024, 64);
        mapnik::datasource_cache::instance().register_datasource("./plugins/input/sqlite.input");
        mapnik::parameters p;
        p["type"]="sqlite";
        p["file"]="tests/data/sqlite/world.sqlite";
        p["table"]="world_merc";
        mapnik::datasource_cache::instance().create(p);
        sqlite3_shutdown();
        BOOST_TEST(true);
    } catch (...) {
        BOOST_TEST(false);
    }
    */

    if (!::boost::detail::test_errors()) {
        std::clog << "C++ exceptions: \x1b[1;32m✓ \x1b[0m\n";
#if BOOST_VERSION >= 104600
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
#endif
    } else {
        return ::boost::report_errors();
    }
}
