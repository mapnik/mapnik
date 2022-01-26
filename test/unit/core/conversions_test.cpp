#include "catch.hpp"

#include <mapnik/value/types.hpp>
#include <mapnik/value.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/util/conversions.hpp>

#include <iostream>
#include <unordered_map>
#include <sstream>

#if defined(_MSC_VER) && _MSC_VER < 1900
#include <cstdio>
#endif

TEST_CASE("conversions")
{
    SECTION("to string")
    {
#if defined(_MSC_VER) && _MSC_VER < 1900
        unsigned int old = _set_output_format(_TWO_DIGIT_EXPONENT);
#endif

        using mapnik::util::string2bool;
        using mapnik::util::to_string;

        try
        {
            std::string out;

            // Test double
            to_string(out, double(0));
            REQUIRE(out == "0");
            out.clear();

            to_string(out, double(1));
            REQUIRE(out == "1");
            out.clear();

            to_string(out, double(-1));
            REQUIRE(out == "-1");
            out.clear();

            to_string(out, double(0.1));
            REQUIRE(out == "0.1");
            out.clear();

            to_string(out, double(-0.1));
            REQUIRE(out == "-0.1");
            out.clear();

            to_string(out, double(0.123));
            REQUIRE(out == "0.123");
            out.clear();

            to_string(out, double(-0.123));
            REQUIRE(out == "-0.123");
            out.clear();

            to_string(out, double(1e-06));
            REQUIRE(out == "1e-06");
            out.clear();

            to_string(out, double(-1e-06));
            REQUIRE(out == "-1e-06");
            out.clear();

            to_string(out, double(1e-05));
            REQUIRE(out == "1e-05");
            out.clear();

            to_string(out, double(-1e-05));
            REQUIRE(out == "-1e-05");
            out.clear();

            to_string(out, double(0.0001));
            REQUIRE(out == "0.0001");
            out.clear();

            to_string(out, double(-0.0001));
            REQUIRE(out == "-0.0001");
            out.clear();

            to_string(out, double(0.0001));
            REQUIRE(out == "0.0001");
            out.clear();

            to_string(out, double(0.00001));
            REQUIRE(out == "1e-05");
            out.clear();

            to_string(out, double(0.000001));
            REQUIRE(out == "1e-06");
            out.clear();

            to_string(out, double(0.0000001));
            REQUIRE(out == "1e-07");
            out.clear();

            to_string(out, double(0.00000001));
            REQUIRE(out == "1e-08");
            out.clear();

            to_string(out, double(0.000000001));
            REQUIRE(out == "1e-09");
            out.clear();

            to_string(out, double(0.0000000001));
            REQUIRE(out == "1e-10");
            out.clear();

            to_string(out, double(-1.234e+16));
            REQUIRE(out == "-1.234e+16");
            out.clear();

            // critical failure when karam is used
            // https://github.com/mapnik/mapnik/issues/1741
            // https://github.com/mapbox/tilemill/issues/1456
            to_string(out, double(8.3));
            REQUIRE(out == "8.3");
            out.clear();

            // non-critical failures if karma is used
            to_string(out, double(0.0001234567890123456));
            // TODO: https://github.com/mapnik/mapnik/issues/1676
            REQUIRE(out == "0.000123457");
            out.clear();

            to_string(out, double(0.00000000001));
            REQUIRE(out == "1e-11");
            out.clear();

            to_string(out, double(0.000000000001));
            REQUIRE(out == "1e-12");
            out.clear();

            to_string(out, double(0.0000000000001));
            REQUIRE(out == "1e-13");
            out.clear();

            to_string(out, double(0.00000000000001));
            REQUIRE(out == "1e-14");
            out.clear();

            to_string(out, double(0.000000000000001));
            REQUIRE(out == "1e-15");
            out.clear();

            to_string(out, double(100000));
            REQUIRE(out == "100000");
            out.clear();

            to_string(out, double(1000000));
            REQUIRE(out == "1e+06");
            out.clear();

            to_string(out, double(10000000));
            REQUIRE(out == "1e+07");
            out.clear();

            to_string(out, double(100000000));
            REQUIRE(out == "1e+08");
            out.clear();

            to_string(out, double(1000000000));
            REQUIRE(out == "1e+09");
            out.clear();

            to_string(out, double(10000000000));
            REQUIRE(out == "1e+10");
            out.clear();

            to_string(out, double(100000000000));
            REQUIRE(out == "1e+11");
            out.clear();

            to_string(out, double(1000000000000));
            REQUIRE(out == "1e+12");
            out.clear();

            to_string(out, double(10000000000000));
            REQUIRE(out == "1e+13");
            out.clear();

            to_string(out, double(100000000000000));
            REQUIRE(out == "1e+14");
            out.clear();

            to_string(out, double(1000000000000005));
            REQUIRE(out == "1e+15");
            out.clear();

            to_string(out, double(-1000000000000000));
            REQUIRE(out == "-1e+15");
            out.clear();

            to_string(out, double(100000000000000.1));
            REQUIRE(out == "1e+14");
            out.clear();

            to_string(out, double(1.00001));
            REQUIRE(out == "1.00001");
            out.clear();

            to_string(out, double(67.65));
            REQUIRE(out == "67.65");
            out.clear();

            to_string(out, double(67.35));
            REQUIRE(out == "67.35");
            out.clear();

            to_string(out, double(1234000000000000));
            REQUIRE(out == "1.234e+15");
            out.clear();

            to_string(out, double(1e+16));
            REQUIRE(out == "1e+16");
            out.clear();

            to_string(out, double(1.234e+16));
            REQUIRE(out == "1.234e+16");
            out.clear();

            // int
            to_string(out, int(2));
            REQUIRE(out == "2");
            out.clear();

            to_string(out, int(0));
            REQUIRE(out == "0");
            out.clear();

            to_string(out, int(-2));
            REQUIRE(out == "-2");
            out.clear();

            to_string(out, int(2147483647));
            REQUIRE(out == "2147483647");
            out.clear();

            to_string(out, int(-2147483648));
            REQUIRE(out == "-2147483648");
            out.clear();

            // unsigned
            to_string(out, unsigned(4294967295));
            REQUIRE(out == "4294967295");
            out.clear();

#ifdef BIGINT
            // long long
            to_string(out, mapnik::value_integer(-0));
            REQUIRE(out == "0");
            out.clear();

            to_string(out, mapnik::value_integer(-2));
            REQUIRE(out == "-2");
            out.clear();

            to_string(out, mapnik::value_integer(9223372036854775807));
            REQUIRE(out == "9223372036854775807");
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
            REQUIRE(out == "true");
            out.clear();

            to_string(out, false);
            REQUIRE(out == "false");
            out.clear();

            bool val = false;
            REQUIRE(!string2bool("this is invalid", val));
            REQUIRE(val == false);
            REQUIRE(string2bool("true", val));
            REQUIRE(val == true);

            // mapnik::value hash() and operator== works for all T in value<Types...>
            mapnik::transcoder tr("utf8");
            using values_container = std::unordered_map<mapnik::value, mapnik::value>;
            values_container vc;
            mapnik::value keys[5] = {true, 123456789, 3.14159f, tr.transcode("Мапник"), mapnik::value_null()};
            for (auto const& k : keys)
            {
                vc.insert({k, k});
                REQUIRE(vc[k] == k);
            }

            // mapnik::value << to ostream
            std::stringstream s;
            mapnik::value_unicode_string ustr = tr.transcode("hello world!");
            mapnik::value streamable(ustr);
            s << streamable;
            CHECK(s.str() == std::string("hello world!"));

        } catch (std::exception const& ex)
        {
            std::clog << ex.what() << "\n";
            REQUIRE(false);
        }
    }
}
