#include <boost/version.hpp>
#include <boost/filesystem/convenience.hpp>
namespace fs = boost::filesystem;
using fs::path;
namespace sys = boost::system;

#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <mapnik/font_engine_freetype.hpp>

int main( int, char*[] )
{
    std::string fontdir("fonts/");

    BOOST_TEST( fs::exists( fontdir ) );
    BOOST_TEST( fs::is_directory( fontdir ) );

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

    if (!::boost::detail::test_errors()) {
        std::clog << "C++ fonts registration: \x1b[1;32m✓ \x1b[0m\n";
#if BOOST_VERSION >= 104600
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
#endif
    } else {
        return ::boost::report_errors();
    }
}
