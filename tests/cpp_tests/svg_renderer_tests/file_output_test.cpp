#undef BOOST_SPIRIT_NO_PREDEFINED_TERMINALS
#define BOOST_TEST_MODULE file_output_test

/*
 * This test module contains test cases that
 * verify the correct generation of SVG output
 * using file streams as destinations.
 */

// boost.test
#include <boost/test/included/unit_test.hpp>

// boost.filesystem
#include <boost/filesystem.hpp>

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/svg/output/svg_renderer.hpp>
#include <mapnik/color_factory.hpp>

// stl
#include <fstream>
#include <iterator>

namespace fs = boost::filesystem;

/*
 * This test case tests the generation of an SVG document
 * using a file stream. It uses svg_renderer parameterized
 * with an std::ofstream as a target for output.
 *
 * It's important to notice that svg_renderer doesn't create
 * or close the file stream, but leaves that task to the client.
 *
 * The test fails if the file can't be created and succeeds
 * otherwise.
 *
 * Note: the file is created in the directory in which the
 * test is run.
 */
BOOST_AUTO_TEST_CASE(file_output_test_case)
{
    using namespace mapnik;
    typedef svg_renderer<std::ostream_iterator<char> > svg_ren;

    Map map(800, 600);
    map.set_background(parse_color("blue"));

    std::string output_filename = "file_output_test_case.svg";
    std::ofstream output_stream(output_filename.c_str());

    if(output_stream)
    {
        std::ostream_iterator<char> output_stream_iterator(output_stream);

        svg_ren renderer(map, output_stream_iterator);
        renderer.apply();

        output_stream.close();

        fs::path output_filename_path =
            fs::system_complete(fs::path(".")) / fs::path(output_filename);

        BOOST_CHECK_MESSAGE(fs::exists(output_filename_path), "File '"+output_filename_path.string()+"' was created.");
    }
    else
    {
        BOOST_FAIL("Could not create create/open file '"+output_filename+"'.");
    }
}
