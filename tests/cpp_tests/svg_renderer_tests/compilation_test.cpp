#define BOOST_TEST_MODULE compile_test

// boost.test
#include <boost/test/included/unit_test.hpp>

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/svg_renderer.hpp>

// std
#include <sstream>

/**
 * This test is meant to see if the empty
 * implementation of SVG renderer compiles
 * and runs when using the inherited methods.
 */
BOOST_AUTO_TEST_CASE(compile_test_case)
{
    using namespace mapnik;

    Map map(800, 600);

    try
    {
	std::ostringstream output_stream;
	svg_renderer<std::ostringstream> renderer(map, output_stream);
	renderer.apply();
    }
    catch(...)
    {
	BOOST_FAIL("Empty implementation throws exception.");
    }
}
