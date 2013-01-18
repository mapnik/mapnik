#include <boost/version.hpp>
#include <mapnik/util/conversions.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <iostream>

int main( int, char*[] )
{
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
    
        to_string(out, double(0.0001234567890123456));
        // TODO: https://github.com/mapnik/mapnik/issues/1676
        BOOST_TEST_EQ( out,  "0.000123457" );
        out.clear();
    
        to_string(out, double(1000000000000000));
        BOOST_TEST_EQ( out,  "1000000000000000" );
        out.clear();
    
        to_string(out, double(-1000000000000000));
        BOOST_TEST_EQ( out,  "-1000000000000000" );
        out.clear();
    
        to_string(out, double(100000000000000.1));
        BOOST_TEST_EQ( out,  "100000000000000.1" );
        out.clear();
    
        to_string(out, double(1.00001));
        BOOST_TEST_EQ( out,  "1.00001" );
        out.clear();
    
        to_string(out, double(1234000000000000));
        BOOST_TEST_EQ( out,  "1234000000000000" );
        out.clear();
    
        to_string(out, double(1.234e+16));
        BOOST_TEST_EQ( out,  "1.234e+16" );
        out.clear();
    
        to_string(out, double(-1.234e+16));
        BOOST_TEST_EQ( out,  "-1.234e+16" );
        out.clear();
    
        // Test int
    
        to_string(out,   int(2));
        BOOST_TEST_EQ( out, "2" );
        out.clear();
    }
    catch (std::exception const & ex)
    {
        std::clog << "C++ type conversions problem: " << ex.what() << "\n";
        BOOST_TEST(false);
    }

    if (!::boost::detail::test_errors()) {
        std::clog << "C++ type conversions: \x1b[1;32m✓ \x1b[0m\n";
#if BOOST_VERSION >= 104600
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
#endif
    } else {
        return ::boost::report_errors();
    }
}
