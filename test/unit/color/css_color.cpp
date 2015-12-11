#include "catch.hpp"

#include <mapnik/safe_cast.hpp>
#include <mapnik/color.hpp>
#include <mapnik/svg2_color_grammar_def.hpp>


TEST_CASE("SVG2 color") {

    SECTION("conversions")
    {
        using namespace  mapnik::svg2_color_grammar;
        CHECK( percent_converter::call(1.0) == 3 );
        CHECK( percent_converter::call(60.0) == 153 );
        // should not overflow on invalid input
        CHECK( mapnik::svg2_color_grammar::percent_converter::call(100000.0) == 255 );
        CHECK( mapnik::svg2_color_grammar::percent_converter::call(-100000.0) == 0 );

#if 0
        CHECK( opacity(0.5) == 128 );
        CHECK( opacity(1.0) == 255 );
        // should not overflow on invalid input
        CHECK( opacity(60.0) == 255 );
        CHECK( opacity(100000.0) == 255 );
        CHECK( opacity(-100000.0) == 0 );
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
#endif
    }

    SECTION("SVG2 colors")
    {
        auto const& color_grammar = mapnik::svg2_color_grammar::expression;
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
}
