#include "catch.hpp"

#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/map.hpp>
#include <mapnik/params.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/feature_style_processor_impl.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/geometry/geometry_type.hpp>

struct rendering_result
{
    unsigned start_map_processing = 0;
    unsigned end_map_processing = 0;

    unsigned end_layer_processing = 0;

    unsigned start_style_processing = 0;
    unsigned end_style_processing = 0;

    std::vector<mapnik::box2d<double>> layer_query_extents;
    std::vector<mapnik::geometry::geometry<double>> geometries;
};

class test_renderer : public mapnik::feature_style_processor<test_renderer>
{
public:
    using processor_impl_type = test_renderer;

    test_renderer(mapnik::Map const& map, rendering_result & result)
        : mapnik::feature_style_processor<test_renderer>(map),
          result_(result),
          painted_(false),
          vars_()
    {
    }

    void start_map_processing(mapnik::Map const& map)
    {
        result_.start_map_processing++;
    }

    void end_map_processing(mapnik::Map const& map)
    {
        result_.end_map_processing++;
    }

    void start_layer_processing(mapnik::layer const& lay, mapnik::box2d<double> const& query_extent)
    {
        result_.layer_query_extents.push_back(query_extent);
    }

    void end_layer_processing(mapnik::layer const& lay)
    {
        result_.end_layer_processing++;
    }

    void start_style_processing(mapnik::feature_type_style const& st)
    {
        result_.start_style_processing++;
    }

    void end_style_processing(mapnik::feature_type_style const& st)
    {
        result_.end_style_processing++;
    }

    template <typename Symbolizer>
    void process(Symbolizer const& sym, mapnik::feature_impl & feature, mapnik::proj_transform const& prj_trans)
    {
        result_.geometries.push_back(feature.get_geometry());
    }

    bool process(mapnik::rule::symbolizers const&, mapnik::feature_impl&, mapnik::proj_transform const& )
    {
        return false;
    }

    double scale_factor() const
    {
        return 1;
    }

    mapnik::attributes const& variables() const
    {
        return vars_;
    }

    mapnik::eAttributeCollectionPolicy attribute_collection_policy() const
    {
        return mapnik::eAttributeCollectionPolicy::DEFAULT;
    }

    bool painted() const
    {
        return painted_;
    }

    void painted(bool painted)
    {
        painted_ = painted;
    }

private:
    rendering_result & result_;
    bool painted_;
    mapnik::attributes vars_;
};

std::shared_ptr<mapnik::memory_datasource> prepare_datasource()
{
    mapnik::parameters params;
    params["type"] = "memory";

    using datasource_ptr = std::shared_ptr<mapnik::memory_datasource>;
    datasource_ptr datasource = std::make_shared<mapnik::memory_datasource>(params);
    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();

    {
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, 1));
        mapnik::geometry::point<double> point(1, 2);
        feature->set_geometry(std::move(point));
        datasource->push(feature);
    }
    {
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, 2));
        mapnik::geometry::line_string<double> path;
        path.emplace_back(-10, 0);
        path.emplace_back(0, 20);
        path.emplace_back(10, 0);
        path.emplace_back(15, 5);
        feature->set_geometry(std::move(path));
        datasource->push(feature);
    }

    return datasource;
}

mapnik::Map prepare_map()
{
    mapnik::Map map(256, 256);

    mapnik::feature_type_style lines_style;
    mapnik::rule rule;
    mapnik::line_symbolizer line_sym;
    rule.append(std::move(line_sym));
    lines_style.add_rule(std::move(rule));
    map.insert_style("lines", std::move(lines_style));

    mapnik::layer lyr("layer");
    lyr.set_datasource(prepare_datasource());
    lyr.add_style("lines");
    map.add_layer(lyr);

    map.zoom_all();

    return map;
}

TEST_CASE("feature_style_processor") {

SECTION("test_renderer") {

    mapnik::Map map(prepare_map());
    rendering_result result;
    test_renderer renderer(map, result);
    renderer.apply();

    REQUIRE(renderer.painted());

    REQUIRE(result.start_map_processing == 1);
    REQUIRE(result.end_map_processing == 1);
    REQUIRE(result.end_layer_processing == 1);
    REQUIRE(result.start_style_processing == 1);
    REQUIRE(result.end_style_processing == 1);

    REQUIRE(result.layer_query_extents.size() == 1);
    const mapnik::box2d<double> reference_query_extent(-10, 0, 15, 20);
    REQUIRE(result.layer_query_extents.front() == reference_query_extent);

    REQUIRE(result.geometries.size() == 2);
    REQUIRE(mapnik::geometry::geometry_type(result.geometries[0]) == mapnik::geometry::geometry_types::Point);
    REQUIRE(mapnik::geometry::geometry_type(result.geometries[1]) == mapnik::geometry::geometry_types::LineString);
}

SECTION("test_renderer - apply() with single layer") {

    mapnik::Map map(prepare_map());
    rendering_result result;
    test_renderer renderer(map, result);
    std::set<std::string> attributes;
    mapnik::layer const& layer = map.get_layer(0);
    renderer.apply(layer, attributes);

    REQUIRE(renderer.painted());

    REQUIRE(result.start_map_processing == 1);
    REQUIRE(result.end_map_processing == 1);
    REQUIRE(result.end_layer_processing == 1);
    REQUIRE(result.start_style_processing == 1);
    REQUIRE(result.end_style_processing == 1);

    REQUIRE(result.layer_query_extents.size() == 1);
    const mapnik::box2d<double> reference_query_extent(-10, 0, 15, 20);
    REQUIRE(result.layer_query_extents.front() == reference_query_extent);

    REQUIRE(result.geometries.size() == 2);
    REQUIRE(mapnik::geometry::geometry_type(result.geometries[0]) == mapnik::geometry::geometry_types::Point);
    REQUIRE(mapnik::geometry::geometry_type(result.geometries[1]) == mapnik::geometry::geometry_types::LineString);
}

SECTION("test_renderer - apply_to_layer") {

    mapnik::Map map(prepare_map());
    rendering_result result;
    test_renderer renderer(map, result);
    std::set<std::string> attributes;
    mapnik::projection map_proj(map.srs(), true);
    mapnik::layer const& layer = map.get_layer(0);
    renderer.apply_to_layer(layer,
                            renderer,
                            map_proj,
                            map.scale(),
                            map.scale_denominator(),
                            map.width(),
                            map.height(),
                            map.get_current_extent(),
                            0,
                            attributes);

    REQUIRE(renderer.painted());

    REQUIRE(result.start_map_processing == 0);
    REQUIRE(result.end_map_processing == 0);
    REQUIRE(result.end_layer_processing == 1);
    REQUIRE(result.start_style_processing == 1);
    REQUIRE(result.end_style_processing == 1);

    REQUIRE(result.layer_query_extents.size() == 1);
    const mapnik::box2d<double> reference_query_extent(-10, 0, 15, 20);
    REQUIRE(result.layer_query_extents.front() == reference_query_extent);

    REQUIRE(result.geometries.size() == 2);
    REQUIRE(mapnik::geometry::geometry_type(result.geometries[0]) == mapnik::geometry::geometry_types::Point);
    REQUIRE(mapnik::geometry::geometry_type(result.geometries[1]) == mapnik::geometry::geometry_types::LineString);
}

}
