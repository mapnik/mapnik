#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/debug.hpp>

#include <boost/detail/lightweight_test.hpp>

#include <iostream>
#include <vector>
#include <algorithm>

#include "utils.hpp"

int main(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i=1;i<argc;++i)
    {
        args.push_back(argv[i]);
    }
    bool quiet = std::find(args.begin(), args.end(), "-q")!=args.end();

    try
    {
        mapnik::logger logger;
        logger.set_severity(mapnik::logger::none);

        BOOST_TEST(set_working_dir(args));

        std::string fontdir("fonts/");

        BOOST_TEST( mapnik::util::exists( fontdir ) );
        BOOST_TEST( mapnik::util::is_directory( fontdir ) );

        std::vector<std::string> face_names;

        std::string foo("foo");

        // fake directories
        BOOST_TEST( !mapnik::freetype_engine::register_fonts(foo , true ) );
        face_names = mapnik::freetype_engine::face_names();
        BOOST_TEST( face_names.size() == 0 );
        BOOST_TEST( !mapnik::freetype_engine::register_fonts(foo) );
        face_names = mapnik::freetype_engine::face_names();
        BOOST_TEST( face_names.size() == 0 );

        // directories without fonts
        std::string src("src");
        // an empty directory will not return true
        // we need to register at least one font and not fail on any
        // to return true
        BOOST_TEST( mapnik::freetype_engine::register_font(src) == false );
        BOOST_TEST( mapnik::freetype_engine::register_fonts(src, true) == false );
        BOOST_TEST( mapnik::freetype_engine::face_names().size() == 0 );

        // bogus, emtpy file that looks like font
        BOOST_TEST( mapnik::freetype_engine::register_font("tests/data/fonts/fake.ttf") == false );
        BOOST_TEST( mapnik::freetype_engine::register_fonts("tests/data/fonts/fake.ttf") == false );
        BOOST_TEST( mapnik::freetype_engine::face_names().size() == 0 );

        BOOST_TEST( mapnik::freetype_engine::register_font("tests/data/fonts/intentionally-broken.ttf") == false );
        BOOST_TEST( mapnik::freetype_engine::register_fonts("tests/data/fonts/intentionally-broken.ttf") == false );
        BOOST_TEST( mapnik::freetype_engine::face_names().size() == 0 );

        // register unifont, since we know it sits in the root fonts/ dir
        BOOST_TEST( mapnik::freetype_engine::register_fonts(fontdir) );
        face_names = mapnik::freetype_engine::face_names();
        //std::clog << "number of registered fonts: " << face_names.size() << std::endl;
        BOOST_TEST( face_names.size() > 0 );
        BOOST_TEST( face_names.size() == 1 );

        // re-register unifont, should not have any affect
        BOOST_TEST( mapnik::freetype_engine::register_fonts(fontdir, false) );
        face_names = mapnik::freetype_engine::face_names();
        //std::clog << "number of registered fonts: " << face_names.size() << std::endl;
        BOOST_TEST( face_names.size() == 1 );

        // register a single dejavu font
        std::string dejavu_bold_oblique("tests/data/fonts/DejaVuSansMono-BoldOblique.ttf");
        BOOST_TEST( mapnik::freetype_engine::register_font(dejavu_bold_oblique) );
        face_names = mapnik::freetype_engine::face_names();
        //std::clog << "number of registered fonts: " << face_names.size() << std::endl;
        BOOST_TEST( face_names.size() == 2 );

        // recurse to find all dejavu fonts
        BOOST_TEST( mapnik::freetype_engine::register_fonts(fontdir, true) );
        face_names = mapnik::freetype_engine::face_names();
        //std::clog << "number of registered fonts: " << face_names.size() << std::endl;
        BOOST_TEST( face_names.size() == 22 );
    }
    catch (std::exception const & ex)
    {
        std::clog << "C++ fonts registration problem: " << ex.what() << "\n";
        BOOST_TEST(false);
    }

    if (!::boost::detail::test_errors()) {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ fonts registration: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    } else {
        return ::boost::report_errors();
    }
}
