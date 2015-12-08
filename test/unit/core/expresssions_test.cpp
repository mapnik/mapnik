#include "catch.hpp"

#include <mapnik/expression.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/unicode.hpp>
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

}
TEST_CASE("expressions")
{
    using properties_type = std::vector<std::pair<std::string, mapnik::value> > ;
    mapnik::transcoder tr("utf8");

    properties_type prop = {{ "foo"   , tr.transcode("bar") },
                            { "name"  , tr.transcode("Québec")},
                            { "double", mapnik::value_double(1.23456)},
                            { "int"   , mapnik::value_integer(123)},
                            { "bool"  , mapnik::value_bool(true)},
                            { "null"  , mapnik::value_null()}};

    auto feature = make_test_feature(1, "POINT(100 200)", prop);

    auto expr = mapnik::parse_expression("[foo]='bar'");
    REQUIRE(evaluate(*feature, *expr) == true);

    // primary expressions
    // null
    expr = mapnik::parse_expression("null");
    REQUIRE(mapnik::to_expression_string(*expr) == "null");
    // boolean
    expr = mapnik::parse_expression("true");
    REQUIRE(mapnik::to_expression_string(*expr) == "true");
    expr = mapnik::parse_expression("false");
    REQUIRE(mapnik::to_expression_string(*expr) == "false");
    // floating point
    expr = mapnik::parse_expression("3.14159");
    REQUIRE(mapnik::to_expression_string(*expr) == "3.14159");
    // integer
    expr = mapnik::parse_expression("123");
    REQUIRE(mapnik::to_expression_string(*expr) == "123");
    // unicode
    expr = mapnik::parse_expression("'single_quoted_string'");
    REQUIRE(mapnik::to_expression_string(*expr) == "'single_quoted_string'");
    expr = mapnik::parse_expression("\"double_quoted_string\"");
    REQUIRE(mapnik::to_expression_string(*expr) == "'double_quoted_string'");

    // floating point constants
    expr = mapnik::parse_expression("pi");
    REQUIRE(mapnik::to_expression_string(*expr) == "3.14159");
    expr = mapnik::parse_expression("deg_to_rad");
    REQUIRE(mapnik::to_expression_string(*expr) == "0.0174533");
    expr = mapnik::parse_expression("rad_to_deg");
    REQUIRE(mapnik::to_expression_string(*expr) == "57.2958");

    // unary functions
    // sin / cos
    expr = mapnik::parse_expression("sin(0.25 * pi)/cos(0.25 * pi)");
    double value = evaluate(*feature, *expr).to_double();
    REQUIRE(std::fabs(value - 1.0) < 1e-6);
    // tan
    auto expr2 = mapnik::parse_expression("tan(0.25 * pi)");
    double value2 = evaluate(*feature, *expr).to_double();
    REQUIRE(value == value2);
    // atan
    expr = mapnik::parse_expression("rad_to_deg * atan(1.0)");
    REQUIRE(std::fabs(evaluate(*feature, *expr).to_double() -  45.0) < 1e-6);
    // exp
    expr = mapnik::parse_expression("exp(0.0)");
    REQUIRE(evaluate(*feature, *expr).to_double() == 1.0);
    // abs
    expr = mapnik::parse_expression("abs(cos(-pi))");
    REQUIRE(evaluate(*feature, *expr).to_double() == 1.0);
    // length (string)
    expr = mapnik::parse_expression("length('1234567890')");
    REQUIRE(evaluate(*feature, *expr).to_int() == 10);

    // binary functions
    // min
    expr = mapnik::parse_expression("min(-0.01, 0.001)");
    REQUIRE(evaluate(*feature, *expr).to_double() == -0.01);
    // max
    expr = mapnik::parse_expression("max(0.01, -0.1)");
    REQUIRE(evaluate(*feature, *expr).to_double() == 0.01);
    // pow
    expr = mapnik::parse_expression("pow(2, 32)");
    REQUIRE(evaluate(*feature, *expr).to_double() == 4294967296.0);

    // geometry types
    expr = mapnik::parse_expression("[mapnik::geometry_type] = point");
    REQUIRE(evaluate(*feature, *expr) == true);
    expr = mapnik::parse_expression("[mapnik::geometry_type] <> linestring");
    REQUIRE(evaluate(*feature, *expr) == true);
    expr = mapnik::parse_expression("[mapnik::geometry_type] != polygon");
    REQUIRE(evaluate(*feature, *expr) == true);
    expr = mapnik::parse_expression("[mapnik::geometry_type] neq collection");
    REQUIRE(evaluate(*feature, *expr) == true);
    expr = mapnik::parse_expression("[mapnik::geometry_type] eq collection");
    REQUIRE(evaluate(*feature, *expr) == false);

    //unary expression
    expr = mapnik::parse_expression("-123.456");
    REQUIRE(evaluate(*feature, *expr).to_double() == -123.456);
    expr = mapnik::parse_expression("+123.456");
    REQUIRE(evaluate(*feature, *expr).to_double() == 123.456);

    // multiplicative/additive
    expr = mapnik::parse_expression("(2.0 * 2.0 + 3.0 * 3.0)/(2.0 * 2.0 - 3.0 * 3.0)");
    REQUIRE(evaluate(*feature, *expr).to_double() == -2.6);
    expr2 = mapnik::parse_expression("(2.0 * 2.0 + 3.0 * 3.0)/((2.0 - 3.0) * (2.0 + 3.0))");
    REQUIRE(evaluate(*feature, *expr).to_double() == evaluate(*feature, *expr2).to_double());

    // logical
    expr = mapnik::parse_expression("[int] = 123 and [double] = 1.23456 && [bool] = true and [null] = null  && [foo] = 'bar'");
    REQUIRE(evaluate(*feature, *expr) == true);

    // relational
    expr = mapnik::parse_expression("[int] > 100 and [int] gt 100.0 and [double] < 2 and [double] lt 2.0");
    REQUIRE(evaluate(*feature, *expr) == true);
    expr = mapnik::parse_expression("[int] >= 123 and [int] ge 123.0 and [double] <= 1.23456 and [double] le 1.23456");
    REQUIRE(evaluate(*feature, *expr) == true);

    // regex
    // replace
    expr = mapnik::parse_expression("[foo].replace('(\\B)|( )','$1 ')");
    REQUIRE(evaluate(*feature, *expr) == tr.transcode("b a r"));

    // match
    expr = mapnik::parse_expression("[name].match('Québec')");
    REQUIRE(evaluate(*feature, *expr) == true);
}
