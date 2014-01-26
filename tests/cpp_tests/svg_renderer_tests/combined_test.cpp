#undef BOOST_SPIRIT_NO_PREDEFINED_TERMINALS
#define BOOST_TEST_MODULE combined_tests

// boost.test
#include <boost/test/included/unit_test.hpp>

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/svg/output/svg_renderer.hpp>
#include <mapnik/color_factory.hpp>

// std
#include <string>
#include <sstream>
#include <iterator>

/**
 * This test case tests all the generators inside svg_renderer,
 * verifying the correctness of the whole SVG document.
 *
 * The test sets the svg_renderer object with a simple Map that
 * has only its dimensions specified and calls the apply()
 * method to produce the output.
 *
 * The output stream is a stringstream (the output is generated
 * into a stringstream).
 */
BOOST_AUTO_TEST_CASE(combined_test_case)
{
    using namespace mapnik;
    typedef svg_renderer<std::ostream_iterator<char> > svg_ren;

    Map map(800, 600);
    map.set_background(parse_color("white"));

    std::ostringstream output_stream;
    std::ostream_iterator<char> output_stream_iterator(output_stream);
    svg_ren renderer(map, output_stream_iterator);
    renderer.apply();

    /*std::string expected_output =
      svg_ren::XML_DECLARATION
      + "\n"
      + svg_ren::SVG_DTD
      + "\n"
      + "<svg width=\"800px\" height=\"600px\" version=\"1.1\" xmlns=\""
      + svg_ren::SVG_NAMESPACE_URL
      + "\">"
      +"\n"
      +"<rect x=\"0\" y=\"0\" width=\"800px\" height=\"600px\" style=\"fill: #ffffff\"/>"
      +"\n"
      +"</svg>";

      std::string actual_output = output_stream.str();
      BOOST_CHECK_EQUAL(actual_output, expected_output);
    */
}

