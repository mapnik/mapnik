#undef BOOST_SPIRIT_NO_PREDEFINED_TERMINALS

#define BOOST_TEST_MODULE background_color_test

/*
 * This test module contains several generators for
 * the implementation of a background color in SVG
 * using a rectangle.
 */

// boost.test
#include <boost/test/included/unit_test.hpp>

// boost.spirit
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/repository/include/karma_confix.hpp>

// boost
#include <boost/fusion/tuple.hpp>

// stl
#include <string>
#include <sstream>

namespace karma = boost::spirit::karma;
namespace repository = boost::spirit::repository;
namespace fusion = boost::fusion;
using namespace karma;

struct F
{
    F() :
        x(0),
        y(0),
        width(100),
        height(100),
        bgcolor("#ffffff"),
        expected_output("<rect x=\"0\" y=\"0\" width=\"100px\" height=\"100px\" style=\"fill: #ffffff\"/>") {}

    ~F() {}

    const int x;
    const int y;
    const int width;
    const int height;
    const std::string bgcolor;
    const std::string expected_output;
    std::ostringstream actual_output;
};

/*
 * This test case uses one compound generator to generate the SVG element.
 *
 * It makes only one call to the format function and uses an ostringstream
 * as its output destination.
 *
 * The attribute values are provided as arguments to the format function.
 * The disadvantage of this is that the number of arguments is limited
 * by the preprocessor constant SPIRIT_ARGUMENTS_LIMIT, whose default
 * value is 10. The advantage is that they can be of any type (as long
 * as they are compatible with the corresponding generator type).
 */
BOOST_FIXTURE_TEST_CASE(bgcolor_stream_test_case, F)
{
    actual_output
        << format(
            "<rect x=\"" << int_ << "\" "
            << "y=\"" << int_ << "\" "
            << "width=\"" << int_ << string << "\" "
            << "height=\"" << int_ << string << "\" "
            << "style=\"fill: " << string << "\""
            << "/>",
            x, y, width, "px", height, "px", bgcolor);

    BOOST_CHECK_EQUAL(actual_output.str(), expected_output);
}

/*
 * This test case uses confix to wrap each xml attribute's value
 * inside double quotes.
 *
 * Notice that the generators' attribute list contains also tuples.
 * Tuples are needed to specify each confix's attributes that contain
 * more than one generator, like confix()[int_ << string] (this
 * generator needs a tuple: tuple<int, string>).
 *
 * The difference between this generator and the one in test case
 * 'bgcolor_stream_test_case' is the use of less << operators and
 * a more clean handling of double quotes.
 */
BOOST_FIXTURE_TEST_CASE(bgcolor_stream_confix_test_case, F)
{
    using repository::confix;
    using fusion::tuple;

    actual_output
        << format(
            "<rect x=" << confix('"', '"')[int_]
            << " y=" << confix('"', '"')[int_]
            << " width=" << confix('"', '"')[int_ << string]
            << " height=" << confix('"', '"')[int_ << string]
            << " style=" << confix('"', '"')["fill: " << string]
            << "/>",
            x, y, tuple<int, std::string>(width, "px"), tuple<int, std::string>(height, "px"), bgcolor);

    BOOST_CHECK_EQUAL(actual_output.str(), expected_output);
}

/*
 * This test case generates the angle brackets of the svg/xml tag
 * using confix. notice that a confix generator can be part of another
 * confix generator's expression.
 *
 * Notice also that the attribute list is the same as in
 * 'bgcolor_stream_confix_test_case'. From this one can see that each
 * generator is meant to have a list of attributes if it has more than one.
 *
 * If the generator is nested inside another generator, the former's attribute
 * list will be another list (a tuple, for example) inside the latter's.
 */
BOOST_FIXTURE_TEST_CASE(bgcolor_stream_confix_complete_test_case, F)
{
    using repository::confix;
    using fusion::tuple;

    actual_output
        << format(
            confix('<', "/>")[
                "rect x=" << confix('"', '"')[int_]
                << " y=" << confix('"', '"')[int_]
                << " width=" << confix('"', '"')[int_ << string]
                << " height=" << confix('"', '"')[int_ << string]
                << " style=" << confix('"', '"')["fill: " << string]],
            x, y, tuple<int, std::string>(width, "px"), tuple<int, std::string>(height, "px"), bgcolor);

    BOOST_CHECK_EQUAL(actual_output.str(), expected_output);
}

/*
 * This test case uses an iterator to receive the generated
 * output. The iterator comes from the std::ostringstream that's
 * been used in the previous test cases, so the output is
 * actually generated into the stream.
 *
 * Using iterators instead of streams has the advantage that
 * more advanced concepts are implemented in karma for them,
 * like rules and grammars.
 */
BOOST_FIXTURE_TEST_CASE(bgcolor_stream_iterator_test_case, F)
{
    using repository::confix;
    using fusion::tuple;

    std::ostream_iterator<char> actual_output_iterator(actual_output);

    generate(
        actual_output_iterator,
        confix("<", "/>")[
            "rect x=" << confix('"', '"')[int_]
            << " y=" << confix('"', '"')[int_]
            << " width=" << confix('"', '"')[int_ << string]
            << " height=" << confix('"', '"')[int_ << string]
            << " style=" << confix('"', '"')["fill: " << string]],
        x, y, tuple<int, std::string>(width, "px"), tuple<int, std::string>(height, "px"), bgcolor);

    BOOST_CHECK_EQUAL(actual_output.str(), expected_output);
}
