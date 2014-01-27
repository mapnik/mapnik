#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <mapnik/projection.hpp>
#include <mapnik/markers_symbolizer.hpp>
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
#include <mapnik/util/fs.hpp>
#include <vector>
#include <algorithm>

#include "utils.hpp"

int main(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i=1;i<argc;++i)
    {
        args.push_back(argv[i]);
    }
    bool quiet = std::find(args.begin(), args.end(), "-q")!=args.end();

    try {
        BOOST_TEST(set_working_dir(args));
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
    r.append(mapnik::markers_symbolizer());
    mapnik::feature_type_style style;
    style.add_rule(r);
    map.insert_style("style",style);

    std::string csv_plugin("./plugins/input/csv.input");
    if (mapnik::util::exists(csv_plugin)) {
        try {
            mapnik::datasource_cache::instance().register_datasource(csv_plugin);
            mapnik::parameters p;
            p["type"]="csv";
            p["inline"]="x,y\n0,0";
            mapnik::datasource_ptr ds = mapnik::datasource_cache::instance().create(p);
            mapnik::layer l("layer");
            l.set_datasource(ds);
            l.add_style("style");
            mapnik::Map m = map;
            m.add_layer(l);
            m.zoom_all();
            mapnik::image_32 im(m.width(),m.height());
            mapnik::agg_renderer<mapnik::image_32> ren(m,im);
            //std::clog << mapnik::save_map_to_string(m) << "\n";
            BOOST_TEST(true);
            // should throw here with "CSV Plugin: no attribute 'foo'. Valid attributes are: x,y."
            ren.apply();
            BOOST_TEST(false);
        } catch (...) {
            BOOST_TEST(true);
        }
    }

    std::string shape_plugin("./plugins/input/shape.input");
    if (mapnik::util::exists(shape_plugin)) {
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

    if (!::boost::detail::test_errors()) {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ exceptions: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    } else {
        return ::boost::report_errors();
    }
}
