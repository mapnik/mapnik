#include <boost/detail/lightweight_test.hpp>
#include <iostream>
//#include <mapnik/value_types.hpp>
#include <mapnik/map.hpp>
//#include <mapnik/params.hpp>
//#include <mapnik/boolean.hpp>

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

    mapnik::Map m0(100,100);
    mapnik::Map m1(100,100);
    mapnik::Map m2(200,100);

    BOOST_TEST(m0 == m1);
    BOOST_TEST(m0 != m2);

    m2 = m1;
    BOOST_TEST(m2 == m1);
    m2 = std::move(m1);
    BOOST_TEST(m2 == m0);
    BOOST_TEST(m1 != m0);

    if (!::boost::detail::test_errors())
    {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ parameters: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    }
    else
    {
        return ::boost::report_errors();
    }
}
