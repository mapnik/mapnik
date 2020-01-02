#include "catch_ext.hpp"

#include <mapnik/expression.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/unicode.hpp>

#include <functional>
#include <vector>

namespace {

template <typename Properties>
mapnik::feature_ptr make_test_feature(mapnik::value_integer id, std::string const& wkt, Properties const& prop)
{
    auto ctx = std::make_shared<mapnik::context_type>();
    mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, id));
    mapnik::geometry::geometry<double> geom;
    if (mapnik::from_wkt(wkt, geom))
    {
        feature->set_geometry(std::move(geom));
    }

    for (auto const& kv : prop)
    {
        feature->put_new(kv.first, kv.second);
    }
    return feature;
}

template <typename Feature, typename Expression>
mapnik::value_type evaluate(Feature const& feature, Expression const& expr)
{
    auto value = mapnik::util::apply_visitor(
        mapnik::evaluate<Feature, mapnik::value_type, mapnik::attributes>(
            feature, mapnik::attributes()), expr);
    return value;
}

mapnik::value evaluate_string(mapnik::feature_ptr const& feature, std::string const& str)
{
    auto expr = mapnik::parse_expression(str);
    return evaluate(*feature, *expr);
}

std::string parse_and_dump(std::string const& str)
{
    auto expr = mapnik::parse_expression(str);
    return mapnik::to_expression_string(*expr);
}

} // namespace

TEST_CASE("expressions")
{
    using namespace std::placeholders;
    using properties_type = std::vector<std::pair<std::string, mapnik::value> > ;
    mapnik::transcoder tr("utf8");

    properties_type prop = {{ "foo"   , tr.transcode("bar") },
                            { "name"  , tr.transcode("Québec")},
                            { "grass" , tr.transcode("grow")},
                            { "wind"  , tr.transcode("blow")},
                            { "sky"   , tr.transcode("is blue")},
                            { "double", mapnik::value_double(1.23456)},
                            { "int"   , mapnik::value_integer(123)},
                            { "bool"  , mapnik::value_bool(true)},
                            { "null"  , mapnik::value_null()}};

    auto feature = make_test_feature(1, "POINT(100 200)", prop);
    auto eval = std::bind(evaluate_string, feature, _1);
    auto approx = Approx::custom().epsilon(1e-6);

    TRY_CHECK(eval(" [foo]='bar' ") == true);

    // primary expressions
    // null
    TRY_CHECK(parse_and_dump("null") == "null");
    // boolean
    TRY_CHECK(parse_and_dump("true") == "true");
    TRY_CHECK(parse_and_dump("false") == "false");
    // floating point
    TRY_CHECK(parse_and_dump("3.14159") == "3.14159");
    // integer
    TRY_CHECK(parse_and_dump("123") == "123");
    // unicode
    TRY_CHECK(parse_and_dump("''") == "''");
    TRY_CHECK(parse_and_dump("'single-quoted string'") == "'single-quoted string'");
    TRY_CHECK(parse_and_dump("\"double-quoted string\"") == "'double-quoted string'");
    TRY_CHECK(parse_and_dump("'escaped \\' apostrophe'") == "'escaped \\' apostrophe'");
    TRY_CHECK(parse_and_dump("'escaped \\\\ backslash'") == "'escaped \\\\ backslash'");

    // floating point constants
    TRY_CHECK(parse_and_dump("pi") == "3.14159");
    TRY_CHECK(parse_and_dump("deg_to_rad") == "0.0174533");
    TRY_CHECK(parse_and_dump("rad_to_deg") == "57.2958");

    // unary functions
    // sin / cos
    TRY_CHECK(eval(" sin(0.25 * pi) / cos(0.25 * pi) ").to_double() == approx(1.0));
    // tan
    TRY_CHECK(eval(" tan(0.25 * pi) ").to_double() == approx(1.0));
    // atan
    TRY_CHECK(eval(" rad_to_deg * atan(1.0) ").to_double() == approx(45.0));
    // exp
    TRY_CHECK(eval(" exp(0.0) ") == 1.0);
    // log
    TRY_CHECK(eval(" log(1.0) ") == 0.0);
    TRY_CHECK(eval(" log(exp(1.0)) ") == 1.0);
    // abs
    TRY_CHECK(eval(" abs(cos(-pi)) ") == 1.0);
    // length (string)
    TRY_CHECK(eval(" length('1234567890') ").to_int() == 10);

    // binary functions
    // min
    TRY_CHECK(eval(" min(-0.01, 0.001) ") == -0.01);
    // max
    TRY_CHECK(eval(" max(0.01, -0.1) ") == 0.01);
    // pow
    TRY_CHECK(eval(" pow(2, 32) ") == 4294967296.0);

    // geometry types
    TRY_CHECK(eval(" [mapnik::geometry_type] = point ") == true);
    TRY_CHECK(eval(" [mapnik::geometry_type] <> linestring ") == true);
    TRY_CHECK(eval(" [mapnik::geometry_type] != polygon ") == true);
    TRY_CHECK(eval(" [mapnik::geometry_type] neq collection ") == true);
    TRY_CHECK(eval(" [mapnik::geometry_type] eq collection ") == false);

    //unary expression
    TRY_CHECK(eval(" -123.456 ") == -123.456);
    TRY_CHECK(eval(" +123.456 ") == 123.456);

    // multiplicative/additive
    auto expr = mapnik::parse_expression("(2.0 * 2.0 + 3.0 * 3.0)/(2.0 * 2.0 - 3.0 * 3.0)");
    TRY_CHECK(evaluate(*feature, *expr) == -2.6);
    auto expr2 = mapnik::parse_expression("(2.0 * 2.0 + 3.0 * 3.0)/((2.0 - 3.0) * (2.0 + 3.0))");
    TRY_CHECK(evaluate(*feature, *expr) == evaluate(*feature, *expr2));

    // logical
    TRY_CHECK(eval(" [int] = 123 and [double] = 1.23456 && [bool] = true and [null] = null && [foo] = 'bar' ") == true);
    TRY_CHECK(eval(" [int] = 456 or [foo].match('foo') || length([foo]) = 3 ") == true);
    TRY_CHECK(eval(" not true  and not true  ") == false); // (not true) and (not true)
    TRY_CHECK(eval(" not false and not true  ") == false); // (not false) and (not true)
    TRY_CHECK(eval(" not true  or  not false ") == true); // (not true) or (not false)
    TRY_CHECK(eval(" not false or  not false ") == true); // (not false) or (not false)

    // test not/and/or precedence using combinations of "not EQ1 OP1 not EQ2 OP2 not EQ3"
    TRY_CHECK(eval(" not [grass] = 'grow' and not [wind] = 'blow' and not [sky] = 'is blue' ") == false);
    TRY_CHECK(eval(" not [grass] = 'grow' and not [wind] = 'blow' or  not [sky] = 'is blue' ") == false);
    TRY_CHECK(eval(" not [grass] = 'grow' or  not [wind] = 'blow' and not [sky] = 'is blue' ") == false);
    TRY_CHECK(eval(" not [grass] = 'grow' or  not [wind] = 'blow' or  not [sky] = 'is blue' ") == false);
    TRY_CHECK(eval(" not [grass] = 'grew' and not [wind] = 'blew' and not [sky] = 'was blue' ") == true);
    TRY_CHECK(eval(" not [grass] = 'grew' and not [wind] = 'blew' or  not [sky] = 'was blue' ") == true);
    TRY_CHECK(eval(" not [grass] = 'grew' or  not [wind] = 'blew' and not [sky] = 'was blue' ") == true);
    TRY_CHECK(eval(" not [grass] = 'grew' or  not [wind] = 'blew' or  not [sky] = 'was blue' ") == true);

    // relational
    TRY_CHECK(eval(" [int] > 100 and [int] gt 100.0 and [double] < 2 and [double] lt 2.0 ") == true);
    TRY_CHECK(eval(" [int] >= 123 and [int] ge 123.0 and [double] <= 1.23456 and [double] le 1.23456 ") == true);

    // empty string/null equality
    TRY_CHECK(eval("[null] = null") == true);
    TRY_CHECK(eval("[null] != null") == false);
    TRY_CHECK(eval("[null] = ''") == false);
    ///////////////////// ref: https://github.com/mapnik/mapnik/issues/1859
    TRY_CHECK(eval("[null] != ''") == false); // back compatible - will be changed in 3.1.x
    //////////////////////
    TRY_CHECK(eval("'' = [null]") == false);
    TRY_CHECK(eval("'' != [null]") == true);

    // regex
    // replace
    TRY_CHECK(eval(" [foo].replace('(\\B)|( )','$1 ') ") == tr.transcode("b a r"));

    // https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode
    //'\u265C\u265E\u265D\u265B\u265A\u265D\u265E\u265C' - black chess figures
    // replace black knights with white knights
    auto val0 = eval(u8"'\u265C\u265E\u265D\u265B\u265A\u265D\u265E\u265C'.replace('\u265E','\u2658')");
    auto val1 = eval("'♜♞♝♛♚♝♞♜'.replace('♞','♘')"); // ==> expected ♜♘♝♛♚♝♘♜
    TRY_CHECK(val0 == val1);
    TRY_CHECK(val0.to_string() == val1.to_string()); // UTF-8
    TRY_CHECK(val0.to_unicode() == val1.to_unicode()); // Unicode (UTF-16)

    // following test will fail if boost_regex is built without ICU support (unpaired surrogates in output)
    TRY_CHECK(eval("[name].replace('(\\B)|( )',' ') ") == tr.transcode("Q u é b e c"));
    TRY_CHECK(eval("'Москва'.replace('(?<!^)(\\B|b)(?!$)',' ')") == tr.transcode("М о с к в а"));
    // 'foo' =~ s:(\w)\1:$1x:r
    TRY_CHECK(eval(" 'foo'.replace('(\\w)\\1', '$1x') ") == tr.transcode("fox"));
    TRY_CHECK(parse_and_dump(" 'foo'.replace('(\\w)\\1', '$1x') ") == "'foo'.replace('(\\w)\\1','$1x')");

    // match
    TRY_CHECK(eval(" [name].match('Québec') ") == true);
    // 'Québec' =~ m:^Q\S*$:
    TRY_CHECK(eval(" [name].match('^Q\\S*$') ") == true);
    TRY_CHECK(parse_and_dump(" [name].match('^Q\\S*$') ") == "[name].match('^Q\\S*$')");

    // string & value concatenation
    // this should evaluate as two strings concatenating
    TRY_CHECK(eval("Hello + '!'") == eval("'Hello!'"));
    // this should evaulate as a combination of an int value and string
    TRY_CHECK(eval("[int]+m") == eval("'123m'"));
}
