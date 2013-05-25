#include <boost/version.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include "plugins/input/csv/csv_datasource.hpp"
#include <mapnik/params.hpp>
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

    // test of directly instanciating a datasource
    try {
        mapnik::parameters params;
        params["type"]="csv";
        params["file"]="./tests/data/csv/wkt.csv";
        csv_datasource ds(params);
        BOOST_TEST(true);
    } catch (std::exception const& ex) {
        BOOST_TEST(false);
        std::clog << "threw: " << ex.what() << "\n";
    }

    if (!::boost::detail::test_errors()) {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ CSV parse: \x1b[1;32m✓ \x1b[0m\n";
#if BOOST_VERSION >= 104600
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
#endif
    } else {
        return ::boost::report_errors();
    }
}
