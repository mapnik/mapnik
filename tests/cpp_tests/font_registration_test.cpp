#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/map.hpp>
#include <mapnik/load_map.hpp>
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
        mapnik::logger::severity_type original_severity = logger.get_severity();

        BOOST_TEST(set_working_dir(args));


        // grab references to global statics of registered/cached fonts
        auto const& global_mapping = mapnik::freetype_engine::get_mapping();
        auto const& global_cache = mapnik::freetype_engine::get_cache();

        // mapnik.Map object has parallel structure for localized fonts
        mapnik::Map m(1,1);
        auto const& local_mapping = m.get_font_file_mapping();
        auto const& local_cache = m.get_font_memory_cache();

        // should be empty to start
        BOOST_TEST( global_mapping.empty() );
        BOOST_TEST( global_cache.empty() );
        BOOST_TEST( local_mapping.empty() );
        BOOST_TEST( local_cache.empty() );

        std::string fontdir("fonts/");

        BOOST_TEST( mapnik::util::exists( fontdir ) );
        BOOST_TEST( mapnik::util::is_directory( fontdir ) );

        // test map cached fonts
        BOOST_TEST( m.register_fonts(fontdir , false ) );
        BOOST_TEST( m.get_font_memory_cache().size() == 0 );
        BOOST_TEST( m.get_font_file_mapping().size() == 1 );
        BOOST_TEST( m.load_fonts() );
        BOOST_TEST( m.get_font_memory_cache().size() == 1 );
        BOOST_TEST( m.register_fonts(fontdir , true ) );
        BOOST_TEST( m.get_font_file_mapping().size() == 22 );
        BOOST_TEST( m.load_fonts() );
        BOOST_TEST( m.get_font_memory_cache().size() == 22 );

        // copy discards memory cache but not file mapping
        mapnik::Map m2(m);
        BOOST_TEST( m2.get_font_memory_cache().size() == 0 );
        BOOST_TEST( m2.get_font_file_mapping().size() == 22 );
        BOOST_TEST( m2.load_fonts() );
        BOOST_TEST( m2.get_font_memory_cache().size() == 22 );

        // test font-directory from XML
        mapnik::Map m3(1,1);
        mapnik::load_map_string(m3,"<Map font-directory=\"fonts/\"></Map>");
        BOOST_TEST( m3.get_font_memory_cache().size() == 0 );
        BOOST_TEST( m3.load_fonts() );
        BOOST_TEST( m3.get_font_memory_cache().size() == 1 );

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
        // silence warnings here by altering the logging severity
        logger.set_severity(mapnik::logger::none);
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

        // now restore the original severity
        logger.set_severity(original_severity);

        // register unifont, since we know it sits in the root fonts/ dir
        BOOST_TEST( mapnik::freetype_engine::register_fonts(fontdir) );
        face_names = mapnik::freetype_engine::face_names();
        BOOST_TEST( face_names.size() > 0 );
        BOOST_TEST( face_names.size() == 1 );

        // re-register unifont, should not have any affect
        BOOST_TEST( mapnik::freetype_engine::register_fonts(fontdir, false) );
        face_names = mapnik::freetype_engine::face_names();
        BOOST_TEST( face_names.size() == 1 );

        // single dejavu font in separate location
        std::string dejavu_bold_oblique("tests/data/fonts/DejaVuSansMono-BoldOblique.ttf");
        BOOST_TEST( mapnik::freetype_engine::register_font(dejavu_bold_oblique) );
        face_names = mapnik::freetype_engine::face_names();
        BOOST_TEST( face_names.size() == 2 );

        // now, inspect font mapping and confirm the correct 'DejaVu Sans Mono Bold Oblique' is registered
        using font_file_mapping = std::map<std::string, std::pair<int,std::string> >;
        font_file_mapping const& name2file = mapnik::freetype_engine::get_mapping();
        bool found_dejavu = false;
        for (auto const& item : name2file)
        {
            if (item.first == "DejaVu Sans Mono Bold Oblique")
            {
                found_dejavu = true;
                BOOST_TEST( item.second.first == 0 );
                BOOST_TEST( item.second.second == dejavu_bold_oblique );
            }
        }
        BOOST_TEST( found_dejavu );

        // recurse to find all dejavu fonts
        BOOST_TEST( mapnik::freetype_engine::register_fonts(fontdir, true) );
        face_names = mapnik::freetype_engine::face_names();
        BOOST_TEST( face_names.size() == 22 );

        // we should have re-registered 'DejaVu Sans Mono Bold Oblique' again,
        // but now at a new path
        bool found_dejavu2 = false;
        for (auto const& item : name2file)
        {
            if (item.first == "DejaVu Sans Mono Bold Oblique")
            {
                found_dejavu2 = true;
                BOOST_TEST( item.second.first == 0 );
                // path should be different
                BOOST_TEST( item.second.second != dejavu_bold_oblique );
            }
        }
        BOOST_TEST( found_dejavu2 );

        // now that global registry is populated
        // now test that a map only loads new fonts
        mapnik::Map m4(1,1);
        BOOST_TEST( m4.register_fonts(fontdir , true ) );
        BOOST_TEST( m4.get_font_memory_cache().size() == 0 );
        BOOST_TEST( m4.get_font_file_mapping().size() == 22 );
        BOOST_TEST( !m4.load_fonts() );
        BOOST_TEST( m4.get_font_memory_cache().size() == 0 );
        BOOST_TEST( m4.register_fonts(dejavu_bold_oblique, false) );
        BOOST_TEST( m4.load_fonts() );
        BOOST_TEST( m4.get_font_memory_cache().size() == 1 );

        // check that we can correctly read a .ttc containing
        // multiple valid faces
        // https://github.com/mapnik/mapnik/issues/2274
        BOOST_TEST( mapnik::freetype_engine::register_font("tests/data/fonts/NotoSans-Regular.ttc") );
        face_names = mapnik::freetype_engine::face_names();
        BOOST_TEST( face_names.size() == 24 );

        // now blindly register as many system fonts as possible
        // the goal here to make sure we don't crash
        // linux
        mapnik::freetype_engine::register_fonts("/usr/share/fonts/", true);
        mapnik::freetype_engine::register_fonts("/usr/local/share/fonts/", true);
        // osx
        mapnik::freetype_engine::register_fonts("/Library/Fonts/", true);
        mapnik::freetype_engine::register_fonts("/System/Library/Fonts/", true);
        // windows
        mapnik::freetype_engine::register_fonts("C:\\Windows\\Fonts", true);
        face_names = mapnik::freetype_engine::face_names();
        BOOST_TEST( face_names.size() > 22 );
    }
    catch (std::exception const & ex)
    {
        std::clog << ex.what() << "\n";
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
