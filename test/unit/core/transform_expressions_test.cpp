#include "catch.hpp"

#include <mapnik/transform/parse_transform.hpp>
#include <mapnik/transform/transform_expression.hpp>

namespace {

bool test_transform_expressions(std::string const& in, std::string const& out)
{
    auto tr_list = mapnik::parse_transform(in);
    return mapnik::to_expression_string(*tr_list) == out;
}

} // namespace

TEST_CASE("transform-expressions")
{
    // matrix
    CHECK(test_transform_expressions("matrix( 1,2,3,4,5, 6)", "matrix(1, 2, 3, 4, 5, 6)"));
    CHECK(test_transform_expressions("matrix(1 2 3 4 5  6)", "matrix(1, 2, 3, 4, 5, 6)"));
    CHECK(test_transform_expressions("matrix(1,2,3,4,5,4*2-1)", "matrix(1, 2, 3, 4, 5, (4*2-1))"));
    CHECK(test_transform_expressions("matrix(1,2,3,4,5,[value])", "matrix(1, 2, 3, 4, 5, [value])"));
    CHECK(test_transform_expressions("matrix(1,2,@value,4,5,6)", "matrix(1, 2, @value, 4, 5, 6)"));
    CHECK(test_transform_expressions("matrix(1,2,3,4,5,@value)", "matrix(1, 2, 3, 4, 5, @value)"));

    // translate
    CHECK(test_transform_expressions("translate(100)", "translate(100)"));
    CHECK(test_transform_expressions("translate([tx])", "translate([tx])"));
    CHECK(test_transform_expressions("translate(100 200)", "translate(100, 200)"));
    CHECK(test_transform_expressions("translate([tx],[ty])", "translate([tx], [ty])"));
    CHECK(test_transform_expressions("translate([tx],200)", "translate([tx], 200)"));
    CHECK(test_transform_expressions("translate(100,[ty])", "translate(100, [ty])"));

    // scale
    CHECK(test_transform_expressions("scale(1.5)", "scale(1.5)"));
    CHECK(test_transform_expressions("scale([sx])", "scale([sx])"));
    CHECK(test_transform_expressions("scale([sx],1.5)", "scale([sx], 1.5)"));
    CHECK(test_transform_expressions("scale(1.5,[sy])", "scale(1.5, [sy])"));
    CHECK(test_transform_expressions("scale([sx],[sy]/2)", "scale([sx], [sy]/2)"));

    // rotate
    CHECK(test_transform_expressions("rotate([a] -2)", "rotate(([a]-2))"));
    CHECK(test_transform_expressions("rotate([a] -2 -3)", "rotate((([a]-2)-3))"));
    CHECK(test_transform_expressions("rotate([a],-2,-3)", "rotate([a], -2, -3)"));
    CHECK(test_transform_expressions("rotate([a] -2 -3 -4)", "rotate(((([a]-2)-3)-4))"));
    CHECK(test_transform_expressions("rotate([a] -2, 3, 4)", "rotate(([a]-2), 3, 4)"));

    // skewX
    CHECK(test_transform_expressions("skewX(2)", "skewX(2)"));
    CHECK(test_transform_expressions("skewX(2*[x]+[y])", "skewX((2*[x]+[y]))"));

    // skewY
    CHECK(test_transform_expressions("skewY(2)", "skewY(2)"));
    CHECK(test_transform_expressions("skewY(2*[x]+[y])", "skewY((2*[x]+[y]))"));

    // compound
    CHECK(test_transform_expressions("translate([tx]) rotate([a])", "translate([tx]) rotate([a])"));
    CHECK(test_transform_expressions("rotate(30+@global_value) scale(2*[sx],[sy])",
                                     "rotate((30+@global_value)) scale(2*[sx], [sy])"));
}
