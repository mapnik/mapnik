#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <mapnik/image_reader.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/util/fs.hpp>
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

    std::string should_throw;
    boost::optional<std::string> type;
    try
    {
        BOOST_TEST(set_working_dir(args));

#if defined(HAVE_JPEG)
        should_throw = "./tests/cpp_tests/data/blank.jpg";
        BOOST_TEST( mapnik::util::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        BOOST_TEST( type );
        try
        {
            std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type));
            BOOST_TEST( false );
        }
        catch (std::exception const&)
        {
            BOOST_TEST( true );
        }
#endif

#if defined(HAVE_PNG)
        should_throw = "./tests/cpp_tests/data/blank.png";
        BOOST_TEST( mapnik::util::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        BOOST_TEST( type );
        try
        {
            std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type));
            BOOST_TEST( false );
        }
        catch (std::exception const&)
        {
            BOOST_TEST( true );
        }

        should_throw = "./tests/data/images/xcode-CgBI.png";
        BOOST_TEST( mapnik::util::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        BOOST_TEST( type );
        try
        {
            std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type));
            BOOST_TEST( false );
        }
        catch (std::exception const&)
        {
            BOOST_TEST( true );
        }
#endif

#if defined(HAVE_TIFF)
        should_throw = "./tests/cpp_tests/data/blank.tiff";
        BOOST_TEST( mapnik::util::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        BOOST_TEST( type );
        try
        {
            std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type));
            BOOST_TEST( false );
        }
        catch (std::exception const&)
        {
            BOOST_TEST( true );
        }
#endif

#if defined(HAVE_WEBP)
        should_throw = "./tests/cpp_tests/data/blank.webp";
        BOOST_TEST( mapnik::util::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        BOOST_TEST( type );
        try
        {
            std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type));
            BOOST_TEST( false );
        }
        catch (std::exception const&)
        {
            BOOST_TEST( true );
        }
#endif
    }
    catch (std::exception const & ex)
    {
        std::clog << "C++ image i/o problem: " << ex.what() << "\n";
        BOOST_TEST(false);
    }

    if (!::boost::detail::test_errors()) {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ image i/o: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    } else {
        return ::boost::report_errors();
    }
}
