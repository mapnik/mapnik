#include "catch.hpp"

#include <mapnik/safe_cast.hpp>
#include <mapnik/color.hpp>
#include <mapnik/css_color_grammar_x3.hpp>
#include <mapnik/css_color_grammar_x3_def.hpp>

#include <sstream>

TEST_CASE("CSS color") {

    SECTION("conversions")
    {
        using namespace mapnik::css_color_grammar;
        CHECK( percent_converter::call(1.0) == 3 );
        CHECK( percent_converter::call(60.0) == 153 );
        // should not overflow on invalid input
        CHECK( percent_converter::call(100000.0) == 255 );
        CHECK( percent_converter::call(-100000.0) == 0 );
    }

    SECTION("CSS colors")
    {
        auto const& color_grammar = mapnik::color_grammar();
        boost::spirit::x3::ascii::space_type space;
        {
            // rgb
            std::string s("rgb(128,0,255)");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xff );
            CHECK( c.red() == 0x80 );
            CHECK( c.green() == 0x00 );
            CHECK( c.blue() == 0xff );
        }
        {
            // rgb (percent)
            std::string s("rgb(50%,0%,100%)");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xff );
            CHECK( c.red() == 0x80 );
            CHECK( c.green() == 0x00 );
            CHECK( c.blue() == 0xff );
        }
        {
            // rgba
            std::string s("rgba(128,0,255,0.5)");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0x80 );
            CHECK( c.red() == 0x80 );
            CHECK( c.green() == 0x00 );
            CHECK( c.blue() == 0xff );
        }
        {
            // rgba (percent)
            std::string s("rgba(50%,0%,100%,0.5)");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0x80 );
            CHECK( c.red() == 0x80 );
            CHECK( c.green() == 0x00 );
            CHECK( c.blue() == 0xff );
        }
        {
            // named colours
            std::string s("darksalmon");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 255 );
            CHECK( c.red() == 233 );
            CHECK( c.green() == 150 );
            CHECK( c.blue() == 122 );
        }
        // hsl
        {
            std::string s("hsl(240,50%,50%)");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 255 );
            CHECK( c.red() == 64 );
            CHECK( c.green() == 64 );
            CHECK( c.blue() == 191 );
        }
        // hsla
        {
            std::string s("hsla(240,50%,50%,0.5)");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 128 );
            CHECK( c.red() == 64 );
            CHECK( c.green() == 64 );
            CHECK( c.blue() == 191 );
        }
        // hex colours
        {
            std::string s("#abcdef");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xff );
            CHECK( c.red() == 0xab );
            CHECK( c.green() == 0xcd );
            CHECK( c.blue() == 0xef );
        }
        {
            std::string s("#abcdef12");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0x12 );
            CHECK( c.red() == 0xab );
            CHECK( c.green() == 0xcd );
            CHECK( c.blue() == 0xef );
        }

        {
            std::string s("  #abcdef");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xff );
            CHECK( c.red() == 0xab );
            CHECK( c.green() == 0xcd );
            CHECK( c.blue() == 0xef );
        }

        {
            std::string s("   #abcdef12");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0x12 );
            CHECK( c.red() == 0xab );
            CHECK( c.green() == 0xcd );
            CHECK( c.blue() == 0xef );
        }

        {
            std::string s("# abcdef");
            mapnik::color c;
            CHECK( !boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
        }

        {
            std::string s("# abcdef12");
            mapnik::color c;
            CHECK( !boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
        }

        {
            std::string s("#ab cdef");
            mapnik::color c;
            CHECK( !boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
        }

        {
            std::string s("#ab cdef12");
            mapnik::color c;
            CHECK( !boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
        }

        // hex_color_small

        {
            std::string s("#abc");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xff );
            CHECK( c.red() == 0xaa );
            CHECK( c.green() == 0xbb );
            CHECK( c.blue() == 0xcc );
        }

        {
            std::string s("#abcd");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xdd );
            CHECK( c.red() == 0xaa );
            CHECK( c.green() == 0xbb );
            CHECK( c.blue() == 0xcc );
        }

        {
            std::string s("   #abc");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xff );
            CHECK( c.red() == 0xaa );
            CHECK( c.green() == 0xbb );
            CHECK( c.blue() == 0xcc );
        }

        {
            std::string s("   #abcd");
            mapnik::color c;
            CHECK( boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
            CHECK( c.alpha() == 0xdd );
            CHECK( c.red() == 0xaa );
            CHECK( c.green() == 0xbb );
            CHECK( c.blue() == 0xcc );
        }

        {
            std::string s("# abc");
            mapnik::color c;
            CHECK( !boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
        }

        {
            std::string s("# abcd");
            mapnik::color c;
            CHECK( !boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
        }

        {
            std::string s("#a bc");
            mapnik::color c;
            CHECK( !boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
        }

        {
            std::string s("#a bcd");
            mapnik::color c;
            CHECK( !boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c) );
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
