#include "catch.hpp"

#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/map.hpp>
#include <mapnik/params.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_style_processor_impl.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/geometry/geometry_type.hpp>
#include <mapnik/well_known_srs.hpp>

struct rendering_result
{
    unsigned start_map_processing = 0;
    unsigned end_map_processing = 0;

    unsigned end_layer_processing = 0;

    unsigned start_style_processing = 0;
    unsigned end_style_processing = 0;
    std::size_t style_geometry_start = 0;
    std::vector<std::size_t> style_geometry_counts;

    std::vector<mapnik::box2d<double>> layer_query_extents;
    std::vector<mapnik::geometry::geometry<double>> geometries;
};

class test_renderer : public mapnik::feature_style_processor<test_renderer>
{
  public:
    using processor_impl_type = test_renderer;

    test_renderer(mapnik::Map const& map, rendering_result& result)
        : mapnik::feature_style_processor<test_renderer>(map),
          result_(result),
          painted_(false),
          vars_()
    {}

    void start_map_processing(mapnik::Map const& map) { result_.start_map_processing++; }

    void end_map_processing(mapnik::Map const& map) { result_.end_map_processing++; }

    void start_layer_processing(mapnik::layer const& lay, mapnik::box2d<double> const& query_extent)
    {
        result_.layer_query_extents.push_back(query_extent);
    }

    void end_layer_processing(mapnik::layer const& lay) { result_.end_layer_processing++; }

    void start_style_processing(mapnik::feature_type_style const& st)
    {
        result_.start_style_processing++;
        result_.style_geometry_start = result_.geometries.size();
    }

    void end_style_processing(mapnik::feature_type_style const& st)
    {
        result_.end_style_processing++;
        result_.style_geometry_counts.push_back(result_.geometries.size() - result_.style_geometry_start);
    }

    template<typename Symbolizer>
    void process(Symbolizer const& sym, mapnik::feature_impl& feature, mapnik::proj_transform const& prj_trans)
    {
        result_.geometries.push_back(feature.get_geometry());
    }

    bool process(mapnik::rule::symbolizers const&, mapnik::feature_impl&, mapnik::proj_transform const&)
    {
        return false;
    }

    double scale_factor() const { return 1; }

    mapnik::attributes const& variables() const { return vars_; }

    mapnik::eAttributeCollectionPolicy attribute_collection_policy() const
    {
        return mapnik::eAttributeCollectionPolicy::DEFAULT;
    }

    bool painted() const { return painted_; }

    void painted(bool painted) { painted_ = painted; }

  private:
    rendering_result& result_;
    bool painted_;
    mapnik::attributes vars_;
};

class unbuffered_bbox_datasource : public mapnik::memory_datasource
{
  public:
    unbuffered_bbox_datasource()
        : mapnik::memory_datasource(prepare_params()),
          query_count_(0)
    {}

    mapnik::featureset_ptr features(mapnik::query const& q) const override
    {
        last_bbox_ = q.get_bbox();
        last_unbuffered_bbox_ = q.get_unbuffered_bbox();
        ++query_count_;
        return mapnik::memory_datasource::features(q);
    }

    unsigned query_count() const { return query_count_; }
    mapnik::box2d<double> const& last_bbox() const { return last_bbox_; }
    mapnik::box2d<double> const& last_unbuffered_bbox() const { return last_unbuffered_bbox_; }

  private:
    static mapnik::parameters prepare_params()
    {
        mapnik::parameters params;
        params["type"] = "memory";
        return params;
    }

    mutable mapnik::box2d<double> last_bbox_;
    mutable mapnik::box2d<double> last_unbuffered_bbox_;
    mutable unsigned query_count_;
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

TEST_CASE("feature_style_processor")
{
    SECTION("test_renderer")
    {
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
        mapnik::box2d<double> const reference_query_extent(-10, 0, 15, 20);
        REQUIRE(result.layer_query_extents.front() == reference_query_extent);

        REQUIRE(result.geometries.size() == 2);
        REQUIRE(mapnik::geometry::geometry_type(result.geometries[0]) == mapnik::geometry::geometry_types::Point);
        REQUIRE(mapnik::geometry::geometry_type(result.geometries[1]) == mapnik::geometry::geometry_types::LineString);
    }

    SECTION("test_renderer - apply() with single layer")
    {
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
        mapnik::box2d<double> const reference_query_extent(-10, 0, 15, 20);
        REQUIRE(result.layer_query_extents.front() == reference_query_extent);

        REQUIRE(result.geometries.size() == 2);
        REQUIRE(mapnik::geometry::geometry_type(result.geometries[0]) == mapnik::geometry::geometry_types::Point);
        REQUIRE(mapnik::geometry::geometry_type(result.geometries[1]) == mapnik::geometry::geometry_types::LineString);
    }

    SECTION("test_renderer - apply_to_layer")
    {
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
        mapnik::box2d<double> const reference_query_extent(-10, 0, 15, 20);
        REQUIRE(result.layer_query_extents.front() == reference_query_extent);

        REQUIRE(result.geometries.size() == 2);
        REQUIRE(mapnik::geometry::geometry_type(result.geometries[0]) == mapnik::geometry::geometry_types::Point);
        REQUIRE(mapnik::geometry::geometry_type(result.geometries[1]) == mapnik::geometry::geometry_types::LineString);
    }

    SECTION("query unbuffered bbox equals the metatile before buffer padding when unclipped")
    {
        // Same SRS everywhere with no query clipping: !unbuffered_bbox! must
        // equal the render extent exactly and must not include buffer padding.
        auto datasource = std::make_shared<unbuffered_bbox_datasource>();
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
        {
            mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, 1));
            mapnik::geometry::line_string<double> path;
            path.emplace_back(-2000000, -2000000);
            path.emplace_back(2000000, 2000000);
            feature->set_geometry(std::move(path));
            datasource->push(feature);
        }

        mapnik::Map map(256, 256, mapnik::MAPNIK_WEBMERCATOR_PROJ);
        map.set_buffer_size(64);

        mapnik::feature_type_style lines_style;
        mapnik::rule rule;
        mapnik::line_symbolizer line_sym;
        rule.append(std::move(line_sym));
        lines_style.add_rule(std::move(rule));
        map.insert_style("lines", std::move(lines_style));

        mapnik::layer lyr("layer", mapnik::MAPNIK_WEBMERCATOR_PROJ);
        lyr.set_datasource(datasource);
        lyr.add_style("lines");
        map.add_layer(lyr);

        mapnik::box2d<double> const render_extent(-1000000, -1000000, 1000000, 1000000);
        map.zoom_to_box(render_extent);

        rendering_result result;
        test_renderer renderer(map, result);
        renderer.apply();

        REQUIRE(datasource->query_count() == 1);
        CHECK(datasource->last_bbox().contains(render_extent));
        CHECK(datasource->last_unbuffered_bbox() == render_extent);
    }

    SECTION("query unbuffered bbox is clipped to the regular query bbox")
    {
        // If maximum_extent clips the regular datasource query, the unbuffered
        // token must be clipped too so it can never be larger than !bbox!.
        auto datasource = std::make_shared<unbuffered_bbox_datasource>();
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
        {
            mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, 1));
            mapnik::geometry::line_string<double> path;
            path.emplace_back(-2000000, -2000000);
            path.emplace_back(2000000, 2000000);
            feature->set_geometry(std::move(path));
            datasource->push(feature);
        }

        mapnik::Map map(256, 256, mapnik::MAPNIK_WEBMERCATOR_PROJ);
        map.set_buffer_size(64);
        map.set_maximum_extent(mapnik::box2d<double>(-500000, -500000, 500000, 500000));

        mapnik::feature_type_style lines_style;
        mapnik::rule rule;
        mapnik::line_symbolizer line_sym;
        rule.append(std::move(line_sym));
        lines_style.add_rule(std::move(rule));
        map.insert_style("lines", std::move(lines_style));

        mapnik::layer lyr("layer", mapnik::MAPNIK_WEBMERCATOR_PROJ);
        lyr.set_datasource(datasource);
        lyr.add_style("lines");
        map.add_layer(lyr);

        mapnik::box2d<double> const render_extent(-1000000, -1000000, 1000000, 1000000);
        mapnik::box2d<double> const clipped_extent(-500000, -500000, 500000, 500000);
        map.zoom_to_box(render_extent);

        rendering_result result;
        test_renderer renderer(map, result);
        renderer.apply();

        REQUIRE(datasource->query_count() == 1);
        CHECK(datasource->last_bbox() == clipped_extent);
        CHECK(datasource->last_unbuffered_bbox() == clipped_extent);
    }

    SECTION("rule indexing preserves filter behavior")
    {
        mapnik::parameters params;
        params["type"] = "memory";
        auto datasource = std::make_shared<mapnik::memory_datasource>(params);
        auto context = std::make_shared<mapnik::context_type>();
        auto feature = mapnik::feature_factory::create(context, 1);
        feature->put_new("kind", mapnik::value_unicode_string("a"));
        feature->put_new("other", mapnik::value_unicode_string("b"));
        feature->set_geometry(mapnik::geometry::point<double>(1, 1));
        datasource->push(feature);

        auto make_rule = [](char const* filter, std::size_t symbol_count) {
            mapnik::rule result;
            if (filter)
            {
                result.set_filter(mapnik::parse_expression(filter));
            }
            for (std::size_t i = 0; i < symbol_count; ++i)
            {
                result.append(mapnik::line_symbolizer());
            }
            return result;
        };

        auto make_matching_style = [&](mapnik::filter_mode_e mode) {
            mapnik::feature_type_style style;
            style.set_filter_mode(mode);
            style.add_rule(make_rule("[kind] = 'a'", 1));
            style.add_rule(make_rule(nullptr, 2));
            style.add_rule(make_rule("[other] = 'b'", 3));
            return style;
        };

        mapnik::Map map(256, 256);
        map.insert_style("all", make_matching_style(mapnik::filter_mode_enum::FILTER_ALL));
        map.insert_style("first", make_matching_style(mapnik::filter_mode_enum::FILTER_FIRST));

        mapnik::feature_type_style else_style;
        else_style.add_rule(make_rule("[kind] = 'missing'", 1));
        mapnik::rule else_rule = make_rule(nullptr, 4);
        else_rule.set_else(true);
        else_style.add_rule(std::move(else_rule));
        map.insert_style("else", std::move(else_style));

        mapnik::feature_type_style also_style;
        also_style.add_rule(make_rule("[kind] = 'a'", 1));
        mapnik::rule also_rule = make_rule(nullptr, 5);
        also_rule.set_also(true);
        also_style.add_rule(std::move(also_rule));
        map.insert_style("also", std::move(also_style));

        mapnik::layer layer("layer");
        layer.set_datasource(datasource);
        layer.add_style("all");
        layer.add_style("first");
        layer.add_style("else");
        layer.add_style("also");
        map.add_layer(layer);
        map.zoom_to_box(mapnik::box2d<double>(0, 0, 2, 2));

        rendering_result result;
        test_renderer renderer(map, result);
        renderer.apply();

        REQUIRE(result.style_geometry_counts.size() == 4);
        CHECK(result.style_geometry_counts[0] == 6);
        CHECK(result.style_geometry_counts[1] == 1);
        CHECK(result.style_geometry_counts[2] == 4);
        CHECK(result.style_geometry_counts[3] == 6);
    }
}
