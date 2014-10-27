#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <mapnik/layer.hpp>
#include <mapnik/map.hpp>
#include <mapnik/color.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>

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

    try
    {
        mapnik::Map m0(100,100);
        mapnik::Map m2(200,100);

        // mapnik::datasource
        mapnik::datasource_cache::instance().register_datasources("plugins/input/shape.input");
        mapnik::parameters p;
        p["type"]="shape";
        p["file"]="demo/data/boundaries";
        p["encoding"]="latin1";
        auto ds0 = mapnik::datasource_cache::instance().create(p);

        auto ds1 = ds0; // shared ptr copy
        BOOST_TEST(ds1 == ds0);
        BOOST_TEST(*ds1 == *ds0);
        ds1 = mapnik::datasource_cache::instance().create(p); // new with the same parameters
        BOOST_TEST(ds1 != ds0);
        BOOST_TEST(*ds1 == *ds0);
        auto ds2 = std::move(ds1);
        BOOST_TEST(ds2 != ds0);
        BOOST_TEST(*ds2 == *ds0);

        // mapnik::layer
        mapnik::layer l0("test-layer");
        l0.set_datasource(ds0);

        mapnik::layer l1 = l0; // copy assignment
        BOOST_TEST(l1 == l0);
        mapnik::layer l2(l0); // copy ctor
        BOOST_TEST(l2 == l0);
        mapnik::layer l3(mapnik::layer("test-layer")); // move ctor
        l3.set_datasource(ds2);

        BOOST_TEST(l3 == l0);
        mapnik::layer l4 = std::move(l3);
        BOOST_TEST(l4 == l0); // move assignment

        m0.add_layer(l4);
        m0.set_background(mapnik::color("skyblue"));
        m2.set_background(mapnik::color("skyblue"));

        auto m1 = m0; //copy

        BOOST_TEST(m0 == m1);
        BOOST_TEST(m0 != m2);

        m2 = m1; // copy
        BOOST_TEST(m2 == m1);
        m2 = std::move(m1);
        BOOST_TEST(m2 == m0);
        BOOST_TEST(m1 != m0);

        BOOST_TEST(m0 == m2);
    }
    catch (std::exception const & ex)
    {
        std::clog << ex.what() << "\n";
        BOOST_TEST(false);
    }

    if (!::boost::detail::test_errors())
    {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ copy/move/assignment tests: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    }
    else
    {
        return ::boost::report_errors();
    }
}
