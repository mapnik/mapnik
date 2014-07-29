#include <mapnik/value_types.hpp>
#include <mapnik/util/conversions.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

#if defined(_MSC_VER)
#include <cstdio>
#endif

int main(int argc, char** argv)
{
    #if defined(_MSC_VER)
    unsigned int old = _set_output_format(_TWO_DIGIT_EXPONENT);
    #endif
    std::vector<std::string> args;
    for (int i=1;i<argc;++i)
    {
        args.push_back(argv[i]);
    }
    bool quiet = std::find(args.begin(), args.end(), "-q")!=args.end();

    using mapnik::util::to_string;

    try
    {
        std::string out;

        // Test double
        to_string(out, double(0));
        BOOST_TEST_EQ( out,  "0" );
        out.clear();

        to_string(out, double(1));
        BOOST_TEST_EQ( out,  "1" );
        out.clear();

        to_string(out, double(-1));
        BOOST_TEST_EQ( out,  "-1" );
        out.clear();

        to_string(out, double(0.1));
        BOOST_TEST_EQ( out,  "0.1" );
        out.clear();

        to_string(out, double(-0.1));
        BOOST_TEST_EQ( out,  "-0.1" );
        out.clear();

        to_string(out, double(0.123));
        BOOST_TEST_EQ( out,  "0.123" );
        out.clear();

        to_string(out, double(-0.123));
        BOOST_TEST_EQ( out,  "-0.123" );
        out.clear();

        to_string(out, double(1e-06));
        BOOST_TEST_EQ( out,  "1e-06" );
        out.clear();

        to_string(out, double(-1e-06));
        BOOST_TEST_EQ( out,  "-1e-06" );
        out.clear();

        to_string(out, double(1e-05));
        BOOST_TEST_EQ( out,  "1e-05" );
        out.clear();

        to_string(out, double(-1e-05));
        BOOST_TEST_EQ( out,  "-1e-05" );
        out.clear();

        to_string(out, double(0.0001));
        BOOST_TEST_EQ( out,  "0.0001" );
        out.clear();

        to_string(out, double(-0.0001));
        BOOST_TEST_EQ( out,  "-0.0001" );
        out.clear();

        to_string(out, double(0.0001));
        BOOST_TEST_EQ( out,  "0.0001" );
        out.clear();

        to_string(out, double(0.00001));
        BOOST_TEST_EQ( out,  "1e-05" );
        out.clear();

        to_string(out, double(0.000001));
        BOOST_TEST_EQ( out,  "1e-06" );
        out.clear();

        to_string(out, double(0.0000001));
        BOOST_TEST_EQ( out,  "1e-07" );
        out.clear();

        to_string(out, double(0.00000001));
        BOOST_TEST_EQ( out,  "1e-08" );
        out.clear();

        to_string(out, double(0.000000001));
        BOOST_TEST_EQ( out,  "1e-09" );
        out.clear();

        to_string(out, double(0.0000000001));
        BOOST_TEST_EQ( out,  "1e-10" );
        out.clear();

        to_string(out, double(-1.234e+16));
        BOOST_TEST_EQ( out,  "-1.234e+16" );
        out.clear();

        // critical failure when karam is used
        // https://github.com/mapnik/mapnik/issues/1741
        // https://github.com/mapbox/tilemill/issues/1456
        to_string(out, double(8.3));
        BOOST_TEST_EQ( out,  "8.3" );
        out.clear();

        // non-critical failures if karma is used
        to_string(out, double(0.0001234567890123456));
        // TODO: https://github.com/mapnik/mapnik/issues/1676
        BOOST_TEST_EQ( out,  "0.000123457" );
        out.clear();

        to_string(out, double(0.00000000001));
        BOOST_TEST_EQ( out,  "1e-11" );
        out.clear();

        to_string(out, double(0.000000000001));
        BOOST_TEST_EQ( out,  "1e-12" );
        out.clear();

        to_string(out, double(0.0000000000001));
        BOOST_TEST_EQ( out,  "1e-13" );
        out.clear();

        to_string(out, double(0.00000000000001));
        BOOST_TEST_EQ( out,  "1e-14" );
        out.clear();

        to_string(out, double(0.000000000000001));
        BOOST_TEST_EQ( out,  "1e-15" );
        out.clear();

        to_string(out, double(100000));
        BOOST_TEST_EQ( out,  "100000" );
        out.clear();

        to_string(out, double(1000000));
        BOOST_TEST_EQ( out,  "1e+06" );
        out.clear();

        to_string(out, double(10000000));
        BOOST_TEST_EQ( out,  "1e+07" );
        out.clear();

        to_string(out, double(100000000));
        BOOST_TEST_EQ( out,  "1e+08" );
        out.clear();

        to_string(out, double(1000000000));
        BOOST_TEST_EQ( out,  "1e+09" );
        out.clear();

        to_string(out, double(10000000000));
        BOOST_TEST_EQ( out,  "1e+10" );
        out.clear();

        to_string(out, double(100000000000));
        BOOST_TEST_EQ( out,  "1e+11" );
        out.clear();

        to_string(out, double(1000000000000));
        BOOST_TEST_EQ( out,  "1e+12" );
        out.clear();

        to_string(out, double(10000000000000));
        BOOST_TEST_EQ( out,  "1e+13" );
        out.clear();

        to_string(out, double(100000000000000));
        BOOST_TEST_EQ( out,  "1e+14" );
        out.clear();

        to_string(out, double(1000000000000005));
        BOOST_TEST_EQ( out,  "1e+15" );
        out.clear();

        to_string(out, double(-1000000000000000));
        BOOST_TEST_EQ( out,  "-1e+15" );
        out.clear();

        to_string(out, double(100000000000000.1));
        BOOST_TEST_EQ( out,  "1e+14" );
        out.clear();

        to_string(out, double(1.00001));
        BOOST_TEST_EQ( out,  "1.00001" );
        out.clear();

        to_string(out, double(67.65));
        BOOST_TEST_EQ( out,  "67.65" );
        out.clear();

        to_string(out, double(67.35));
        BOOST_TEST_EQ( out,  "67.35" );
        out.clear();

        to_string(out, double(1234000000000000));
        BOOST_TEST_EQ( out,  "1.234e+15" );
        out.clear();

        to_string(out, double(1e+16));
        BOOST_TEST_EQ( out,  "1e+16" );
        out.clear();

        to_string(out, double(1.234e+16));
        BOOST_TEST_EQ( out,  "1.234e+16" );
        out.clear();

        // int
        to_string(out, int(2));
        BOOST_TEST_EQ( out, "2" );
        out.clear();

        to_string(out, int(0));
        BOOST_TEST_EQ( out, "0" );
        out.clear();

        to_string(out, int(-2));
        BOOST_TEST_EQ( out, "-2" );
        out.clear();

        to_string(out, int(2147483647));
        BOOST_TEST_EQ( out, "2147483647" );
        out.clear();

        to_string(out, int(-2147483648));
        BOOST_TEST_EQ( out, "-2147483648" );
        out.clear();

        // unsigned
        to_string(out, unsigned(4294967295));
        BOOST_TEST_EQ( out, "4294967295" );
        out.clear();

#ifdef BIGINT
        // long long
        to_string(out,mapnik::value_integer(-0));
        BOOST_TEST_EQ( out, "0" );
        out.clear();

        to_string(out,mapnik::value_integer(-2));
        BOOST_TEST_EQ( out, "-2" );
        out.clear();

        to_string(out,mapnik::value_integer(9223372036854775807));
        BOOST_TEST_EQ( out, "9223372036854775807" );
        out.clear();
#else
  #ifdef _MSC_VER
    #pragma NOTE("BIGINT not defined so skipping large number conversion tests")
  #else
    #warning BIGINT not defined so skipping large number conversion tests
  #endif
#endif
        // bool
        to_string(out, true);
        BOOST_TEST_EQ( out, "true" );
        out.clear();

        to_string(out, false);
        BOOST_TEST_EQ( out, "false" );
        out.clear();
    }
    catch (std::exception const & ex)
    {
        std::clog << "C++ type conversions problem: " << ex.what() << "\n";
        BOOST_TEST(false);
    }

    if (!::boost::detail::test_errors()) {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ type conversions: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    } else {
        return ::boost::report_errors();
    }
}
