#include "catch.hpp"
#include <mapnik/text/placements/simple.hpp>
#include <mapnik/feature.hpp>

TEST_CASE("text_placements_simple")
{
    mapnik::text_placements_simple simple("N, S, 8");

    simple.defaults.format_defaults.text_size = 12.0;

    mapnik::context_ptr context(std::make_shared<mapnik::context_type>());
    mapnik::feature_impl feature(context, 1);
    mapnik::attributes vars;
    mapnik::text_placement_info_ptr info = simple.get_placement_info(1.0, feature, vars);

    REQUIRE(info->next());
    CHECK(info->properties.layout_defaults.dir == mapnik::NORTH);
    CHECK(info->properties.format_defaults.text_size.get<mapnik::value_double>() == Approx(12.0));

    REQUIRE(info->next());
    CHECK(info->properties.layout_defaults.dir == mapnik::SOUTH);
    CHECK(info->properties.format_defaults.text_size.get<mapnik::value_double>() == Approx(12.0));

    REQUIRE(info->next());
    CHECK(info->properties.layout_defaults.dir == mapnik::NORTH);
    CHECK(info->properties.format_defaults.text_size.get<mapnik::value_double>() == Approx(8.0));

    REQUIRE(info->next());
    CHECK(info->properties.layout_defaults.dir == mapnik::SOUTH);
    CHECK(info->properties.format_defaults.text_size.get<mapnik::value_double>() == Approx(8.0));

    CHECK(!info->next());
}
