#include "catch.hpp"

#include <mapnik/safe_cast.hpp>
#include <mapnik/color.hpp>
#include <mapnik/css/css_color_grammar_x3.hpp>
#include <mapnik/css/css_color_grammar_x3_def.hpp>

#include <sstream>

TEST_CASE("CSS color")
{
    SECTION("conversions")
    {
        using namespace mapnik::css_color_grammar;
        CHECK(percent_converter::call(1.0) == 3);
        CHECK(percent_converter::call(60.0) == 153);
        CHECK(percent_converter::call(10) == 26);
        CHECK(percent_converter::call(35) == 89);
        CHECK(percent_converter::call(35.4999) == 91);
        // should not overflow on invalid input
        CHECK(percent_converter::call(100000.0) == 255);
        CHECK(percent_converter::call(-100000.0) == 0);
    }

    SECTION("CSS colors")
    {
        auto const& color_grammar = mapnik::css_color_grammar::css_color;
        boost::spirit::x3::ascii::space_type space;
        {
            // rgb
            std::string s("rgb(128,0,255)");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0xff);
            CHECK(c.red() == 0x80);
            CHECK(c.green() == 0x00);
            CHECK(c.blue() == 0xff);
        }
        {
            // rgb (percent)
            std::string s1("rgb(50%,0%,100%)");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s1.cbegin(), s1.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0xff);
            CHECK(c.red() == 0x80);
            CHECK(c.green() == 0x00);
            CHECK(c.blue() == 0xff);
            // rgb (fractional percent)
            std::string s2("rgb(50.5%,0.5%,99.5%)"); // #8101fe
            CHECK(boost::spirit::x3::phrase_parse(s2.cbegin(), s2.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0xff);
            CHECK(c.red() == 0x81);
            CHECK(c.green() == 0x01);
            CHECK(c.blue() == 0xfe);
        }
        {
            // rgba
            std::string s("rgba(128,0,255,0.5)");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0x80);
            CHECK(c.red() == 0x80);
            CHECK(c.green() == 0x00);
            CHECK(c.blue() == 0xff);
        }
        {
            // rgba (percent)
            std::string s1("rgba(50%,0%,100%,0.5)");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s1.cbegin(), s1.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0x80);
            CHECK(c.red() == 0x80);
            CHECK(c.green() == 0x00);
            CHECK(c.blue() == 0xff);
            // rgba (fractional percent)
            std::string s2("rgb(50.5%,0.5%,99.5%)"); // #8101fe80
            CHECK(boost::spirit::x3::phrase_parse(s2.cbegin(), s2.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0x80);
            CHECK(c.red() == 0x81);
            CHECK(c.green() == 0x01);
            CHECK(c.blue() == 0xfe);
        }
        {
            // named colours
            std::string s("darksalmon");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 255);
            CHECK(c.red() == 233);
            CHECK(c.green() == 150);
            CHECK(c.blue() == 122);
        }
        // hsl
        {
            std::string s("hsl(240,50%,50%)");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 255);
            CHECK(c.red() == 64);
            CHECK(c.green() == 64);
            CHECK(c.blue() == 191);
        }
        // hsl (fractional percent)
        {
            std::string s("hsl(240,50.5%,49.5%)"); // Color(R=62,G=62,B=190,A=255)
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 255);
            CHECK(c.red() == 62);
            CHECK(c.green() == 62);
            CHECK(c.blue() == 190);
        }
        // hsl (hue range 0..360)
        {
            std::string s1("hsl(0, 100%, 50%)");
            std::string s2("hsl(120, 100%, 50%)");
            std::string s3("hsl(240, 100%, 50%)");
            std::string s4("hsl(360, 100%, 50%)");
            std::string s5("hsl(480, 100%, 50%)"); // larger values are
            std::string s6("hsl(600, 100%, 50%)"); // normalised to fit into 0..360

            mapnik::color c1, c2, c3, c4, c5, c6;
            CHECK(boost::spirit::x3::phrase_parse(s1.cbegin(), s1.cend(), color_grammar, space, c1));
            CHECK(boost::spirit::x3::phrase_parse(s2.cbegin(), s2.cend(), color_grammar, space, c2));
            CHECK(boost::spirit::x3::phrase_parse(s3.cbegin(), s3.cend(), color_grammar, space, c3));
            CHECK(boost::spirit::x3::phrase_parse(s4.cbegin(), s4.cend(), color_grammar, space, c4));
            CHECK(boost::spirit::x3::phrase_parse(s5.cbegin(), s5.cend(), color_grammar, space, c5));
            CHECK(boost::spirit::x3::phrase_parse(s6.cbegin(), s6.cend(), color_grammar, space, c6));
            CHECK(c1 == mapnik::color("red"));
            CHECK(c2 == mapnik::color("lime"));
            CHECK(c3 == mapnik::color("blue"));
            CHECK(c1 == c4);
            CHECK(c2 == c5);
            CHECK(c3 == c6);
        }
        // hsla
        {
            std::string s("hsla(240,50%,50%,0.5)");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 128);
            CHECK(c.red() == 64);
            CHECK(c.green() == 64);
            CHECK(c.blue() == 191);
        }
        // hsla (fractional percent)
        {
            std::string s("hsla(240,50.5%,49.5%,0.5)"); // Color(R=62,G=62,B=190,A=128)
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 128);
            CHECK(c.red() == 62);
            CHECK(c.green() == 62);
            CHECK(c.blue() == 190);
        }
        // hsla (hue range 0..360)
        {
            std::string s1("hsla(0, 100%, 50%, 1)");
            std::string s2("hsla(120, 100%, 50%, 1)");
            std::string s3("hsla(240, 100%, 50%, 1)");
            std::string s4("hsla(360, 100%, 50%, 1)");
            std::string s5("hsla(480, 100%, 50%, 1)"); // larger values are
            std::string s6("hsla(600, 100%, 50%, 1)"); // normalised to fit into 0..360

            mapnik::color c1, c2, c3, c4, c5, c6;
            CHECK(boost::spirit::x3::phrase_parse(s1.cbegin(), s1.cend(), color_grammar, space, c1));
            CHECK(boost::spirit::x3::phrase_parse(s2.cbegin(), s2.cend(), color_grammar, space, c2));
            CHECK(boost::spirit::x3::phrase_parse(s3.cbegin(), s3.cend(), color_grammar, space, c3));
            CHECK(boost::spirit::x3::phrase_parse(s4.cbegin(), s4.cend(), color_grammar, space, c4));
            CHECK(boost::spirit::x3::phrase_parse(s5.cbegin(), s5.cend(), color_grammar, space, c5));
            CHECK(boost::spirit::x3::phrase_parse(s6.cbegin(), s6.cend(), color_grammar, space, c6));
            CHECK(c1 == mapnik::color("red"));
            CHECK(c2 == mapnik::color("lime"));
            CHECK(c3 == mapnik::color("blue"));
            CHECK(c1 == c4);
            CHECK(c2 == c5);
            CHECK(c3 == c6);
        }
        // hex colours
        {
            std::string s("#abcdef");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0xff);
            CHECK(c.red() == 0xab);
            CHECK(c.green() == 0xcd);
            CHECK(c.blue() == 0xef);
        }
        {
            std::string s("#abcdef12");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0x12);
            CHECK(c.red() == 0xab);
            CHECK(c.green() == 0xcd);
            CHECK(c.blue() == 0xef);
        }

        {
            std::string s("  #abcdef");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0xff);
            CHECK(c.red() == 0xab);
            CHECK(c.green() == 0xcd);
            CHECK(c.blue() == 0xef);
        }

        {
            std::string s("   #abcdef12");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0x12);
            CHECK(c.red() == 0xab);
            CHECK(c.green() == 0xcd);
            CHECK(c.blue() == 0xef);
        }

        {
            std::string s("# abcdef");
            mapnik::color c;
            CHECK(!boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
        }

        {
            std::string s("# abcdef12");
            mapnik::color c;
            CHECK(!boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
        }

        {
            std::string s("#ab cdef");
            mapnik::color c;
            CHECK(!boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
        }

        {
            std::string s("#ab cdef12");
            mapnik::color c;
            CHECK(!boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
        }

        // hex_color_small

        {
            std::string s("#abc");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0xff);
            CHECK(c.red() == 0xaa);
            CHECK(c.green() == 0xbb);
            CHECK(c.blue() == 0xcc);
        }

        {
            std::string s("#abcd");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0xdd);
            CHECK(c.red() == 0xaa);
            CHECK(c.green() == 0xbb);
            CHECK(c.blue() == 0xcc);
        }

        {
            std::string s("   #abc");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0xff);
            CHECK(c.red() == 0xaa);
            CHECK(c.green() == 0xbb);
            CHECK(c.blue() == 0xcc);
        }

        {
            std::string s("   #abcd");
            mapnik::color c;
            CHECK(boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
            CHECK(c.alpha() == 0xdd);
            CHECK(c.red() == 0xaa);
            CHECK(c.green() == 0xbb);
            CHECK(c.blue() == 0xcc);
        }

        {
            std::string s("# abc");
            mapnik::color c;
            CHECK(!boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
        }

        {
            std::string s("# abcd");
            mapnik::color c;
            CHECK(!boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
        }

        {
            std::string s("#a bc");
            mapnik::color c;
            CHECK(!boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
        }

        {
            std::string s("#a bcd");
            mapnik::color c;
            CHECK(!boost::spirit::x3::phrase_parse(s.cbegin(), s.cend(), color_grammar, space, c));
        }
    }

    SECTION("operator<< / to_string()")
    {
        mapnik::color c("salmon");
        std::ostringstream ss;
        ss << c;
        CHECK(ss.str() == "rgb(250,128,114)");
        c.set_alpha(127);
        ss.seekp(0);
        ss << c;
        CHECK(ss.str() == "rgba(250,128,114,0.498)");
    }
    SECTION("operator= operator==")
    {
        mapnik::color c1("cornflowerblue", true);
        mapnik::color c2 = c1; // make copy
        CHECK(c1.red() == c2.red());
        CHECK(c1.green() == c2.green());
        CHECK(c1.blue() == c2.blue());
        CHECK(c1.get_premultiplied() == c2.get_premultiplied());
        CHECK(c1 == c2);
        c1.demultiply();
        CHECK(c1 != c2);
        CHECK(c1.get_premultiplied() != c2.get_premultiplied());
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
