#include "catch.hpp"
#include <mapnik/css_color_grammar.hpp>
#include <mapnik/css_color_grammar_impl.hpp>
#include <mapnik/safe_cast.hpp>
#include <sstream>

TEST_CASE("css color") {

    SECTION("conversions")
    {
        mapnik::percent_conv_impl conv;
        CHECK( conv(1.0) == 3 );
        CHECK( conv(60.0) == 153 );
        // should not overflow on invalid input
        CHECK( conv(100000.0) == 255 );
        CHECK( conv(-100000.0) == 0 );

        mapnik::alpha_conv_impl conv2;
        CHECK( conv2(0.5) == 128 );
        CHECK( conv2(1.0) == 255 );
        // should not overflow on invalid input
        CHECK( conv2(60.0) == 255 );
        CHECK( conv2(100000.0) == 255 );
        CHECK( conv2(-100000.0) == 0 );

        mapnik::hsl_conv_impl conv3;
        mapnik::color c;
        conv3(c, 1.0, 1.0, 1.0);
        CHECK( c.alpha() == 255 );
        CHECK( c.red() == 3 );
        CHECK( c.green() == 3 );
        CHECK( c.blue() == 3 );
        // invalid
        conv3(c, -1, -1, -1);
        CHECK( c.alpha() == 255 ); // should not be touched by hsl converter
        CHECK( c.red() == 0 );
        CHECK( c.green() == 0 );
        CHECK( c.blue() == 0 );
    }

    SECTION("hex colors")
    {
        mapnik::css_color_grammar<std::string::const_iterator> color_grammar;
        boost::spirit::qi::ascii::space_type space;

        {
            std::string s("#abcdef");
            mapnik::color c;
            CHECK( boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xff );
            CHECK( c.red() == 0xab );
            CHECK( c.green() == 0xcd );
            CHECK( c.blue() == 0xef );
        }

        {
            std::string s("#abcdef12");
            mapnik::color c;
            CHECK( boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0x12 );
            CHECK( c.red() == 0xab );
            CHECK( c.green() == 0xcd );
            CHECK( c.blue() == 0xef );
        }

        {
            std::string s("  #abcdef");
            mapnik::color c;
            CHECK( boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xff );
            CHECK( c.red() == 0xab );
            CHECK( c.green() == 0xcd );
            CHECK( c.blue() == 0xef );
        }

        {
            std::string s("   #abcdef12");
            mapnik::color c;
            CHECK( boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0x12 );
            CHECK( c.red() == 0xab );
            CHECK( c.green() == 0xcd );
            CHECK( c.blue() == 0xef );
        }

        {
            std::string s("# abcdef");
            CHECK( !boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space) );
        }

        {
            std::string s("# abcdef12");
            CHECK( !boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space) );
        }

        {
            std::string s("#ab cdef");
            CHECK( !boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space) );
        }

        {
            std::string s("#ab cdef12");
            CHECK( !boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space) );
        }

        // hex_color_small

        {
            std::string s("#abc");
            mapnik::color c;
            CHECK( boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xff );
            CHECK( c.red() == 0xaa );
            CHECK( c.green() == 0xbb );
            CHECK( c.blue() == 0xcc );
        }

        {
            std::string s("#abcd");
            mapnik::color c;
            CHECK( boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xdd );
            CHECK( c.red() == 0xaa );
            CHECK( c.green() == 0xbb );
            CHECK( c.blue() == 0xcc );
        }

        {
            std::string s("   #abc");
            mapnik::color c;
            CHECK( boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xff );
            CHECK( c.red() == 0xaa );
            CHECK( c.green() == 0xbb );
            CHECK( c.blue() == 0xcc );
        }

        {
            std::string s("   #abcd");
            mapnik::color c;
            CHECK( boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xdd );
            CHECK( c.red() == 0xaa );
            CHECK( c.green() == 0xbb );
            CHECK( c.blue() == 0xcc );
        }

        {
            std::string s("# abc");
            CHECK( !boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space) );
        }

        {
            std::string s("# abcd");
            CHECK( !boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space) );
        }

        {
            std::string s("#a bc");
            CHECK( !boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space) );
        }

        {
            std::string s("#a bcd");
            CHECK( !boost::spirit::qi::phrase_parse(s.cbegin(), s.cend(), color_grammar, space) );
        }
    }
    SECTION("operator<< / to_string()")
    {
        mapnik::color c("salmon");
        std::ostringstream ss;
        ss << c ;
        CHECK(ss.str() == "rgb(250,128,114)");
        c.set_alpha(127);
        ss.seekp(0);
        ss << c ;
        CHECK(ss.str() == "rgba(250,128,114,0.498)");
    }
    SECTION("premultiply/demultiply")
    {
        mapnik::color c("cornflowerblue");
        c.set_alpha(127);
        c.premultiply();
        CHECK(int(c.red()) == 50);
        CHECK(int(c.green()) == 74);
        CHECK(int(c.blue()) == 118);
        CHECK(int(c.alpha()) == 127);
        c.demultiply();
        CHECK(c == mapnik::color(100, 148, 236, 127));
    }
}
