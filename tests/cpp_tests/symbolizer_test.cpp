#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <mapnik/symbolizer.hpp>
#include <vector>
#include <algorithm>

using namespace mapnik;

int main(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i=1;i<argc;++i)
    {
        args.push_back(argv[i]);
    }
    bool quiet = std::find(args.begin(), args.end(), "-q")!=args.end();

    try {
        marker_multi_policy_enum policy_in = MARKER_WHOLE_MULTI;
        BOOST_TEST_EQ(policy_in,MARKER_WHOLE_MULTI);
        markers_symbolizer sym;
        put(sym, keys::markers_multipolicy, policy_in);
        BOOST_TEST_EQ(sym.properties.count(keys::markers_multipolicy),static_cast<unsigned long>(1));
        marker_multi_policy_enum policy_out = get<mapnik::marker_multi_policy_enum>(sym, keys::markers_multipolicy);
        BOOST_TEST_EQ(policy_out,MARKER_WHOLE_MULTI);
    }
    catch (std::exception const & ex)
    {
        std::clog << ex.what() << std::endl;
        BOOST_TEST(false);
    }

    if (!::boost::detail::test_errors()) {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ symbolizer test: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    } else {
        return ::boost::report_errors();
    }
}
