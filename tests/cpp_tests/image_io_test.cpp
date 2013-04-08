#include <boost/version.hpp>
#include <boost/filesystem/convenience.hpp>
namespace fs = boost::filesystem;
using fs::path;
namespace sys = boost::system;

#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <mapnik/image_reader.hpp>
#include <mapnik/image_util.hpp>


int main( int, char*[] )
{
    std::string should_throw;
    boost::optional<std::string> type;
    try
    {
        should_throw = "./tests/cpp_tests/data/blank.jpg";
        BOOST_TEST( fs::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        BOOST_TEST( type );
        try
        {
            std::auto_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type));
            BOOST_TEST( false );
        }
        catch (std::exception const&)
        {
            BOOST_TEST( true );
        }

        should_throw = "./tests/cpp_tests/data/blank.png";
        BOOST_TEST( fs::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        BOOST_TEST( type );
        try
        {
            std::auto_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type));
            BOOST_TEST( false );
        }
        catch (std::exception const&)
        {
            BOOST_TEST( true );
        }

        should_throw = "./tests/cpp_tests/data/blank.tiff";
        BOOST_TEST( fs::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        BOOST_TEST( type );
        try
        {
            std::auto_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type));
            BOOST_TEST( false );
        }
        catch (std::exception const&)
        {
            BOOST_TEST( true );
        }

        should_throw = "./tests/data/images/xcode-CgBI.png";
        BOOST_TEST( fs::exists( should_throw ) );
        type = mapnik::type_from_filename(should_throw);
        BOOST_TEST( type );
        try
        {
            std::auto_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(should_throw,*type));
            BOOST_TEST( false );
        }
        catch (std::exception const&)
        {
            BOOST_TEST( true );
        }

    }
    catch (std::exception const & ex)
    {
        std::clog << "C++ image i/o problem: " << ex.what() << "\n";
        BOOST_TEST(false);
    }

    if (!::boost::detail::test_errors()) {
        std::clog << "C++ image i/o: \x1b[1;32m✓ \x1b[0m\n";
#if BOOST_VERSION >= 104600
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
#endif
    } else {
        return ::boost::report_errors();
    }
}
