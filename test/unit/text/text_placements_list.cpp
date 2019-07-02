#include "catch.hpp"
#include <mapnik/text/placements/list.hpp>
#include <mapnik/text/formatting/text.hpp>
#include <mapnik/feature.hpp>

TEST_CASE("text_placements_list")
{
    mapnik::text_placements_list list;

    list.defaults.format_defaults.text_size = 12.0;
    list.defaults.set_format_tree(std::make_shared<mapnik::formatting::text_node>("\"Default text\""));

    {
        mapnik::text_symbolizer_properties & properties = list.add();
        properties.format_defaults.text_size = 10.0;
        properties.set_format_tree(std::make_shared<mapnik::formatting::text_node>("\"Alternative text 1\""));
    }

    {
        mapnik::text_symbolizer_properties & properties = list.add();
        properties.format_defaults.text_size = 8.0;
        properties.set_format_tree(std::make_shared<mapnik::formatting::text_node>("\"Alternative text 2\""));
    }

    mapnik::context_ptr context(std::make_shared<mapnik::context_type>());
    mapnik::feature_impl feature(context, 1);
    mapnik::attributes vars;
    mapnik::text_placement_info_ptr info = list.get_placement_info(1.0, feature, vars);

    {
        REQUIRE(info->next());
        CHECK(info->properties.format_defaults.text_size.get<mapnik::value_double>() == Approx(12.0));
        REQUIRE(info->properties.format_tree());
        mapnik::expression_set expressions;
        info->properties.format_tree()->add_expressions(expressions);
        REQUIRE(!expressions.empty());
        mapnik::expr_node const& expression = **expressions.begin();
        CHECK(expression.get<mapnik::value_unicode_string>() == mapnik::value_unicode_string("Default text"));
    }

    {
        REQUIRE(info->next());
        CHECK(info->properties.format_defaults.text_size.get<mapnik::value_double>() == Approx(10.0));
        REQUIRE(info->properties.format_tree());
        mapnik::expression_set expressions;
        info->properties.format_tree()->add_expressions(expressions);
        REQUIRE(!expressions.empty());
        mapnik::expr_node const& expression = **expressions.begin();
        CHECK(expression.get<mapnik::value_unicode_string>() == mapnik::value_unicode_string("Alternative text 1"));
    }

    {
        REQUIRE(info->next());
        CHECK(info->properties.format_defaults.text_size.get<mapnik::value_double>() == Approx(8.0));
        REQUIRE(info->properties.format_tree());
        mapnik::expression_set expressions;
        info->properties.format_tree()->add_expressions(expressions);
        REQUIRE(!expressions.empty());
        mapnik::expr_node const& expression = **expressions.begin();
        CHECK(expression.get<mapnik::value_unicode_string>() == mapnik::value_unicode_string("Alternative text 2"));
    }

    CHECK(!info->next());
}
