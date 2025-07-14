#include "catch.hpp"

#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/map.hpp>
#include <mapnik/params.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/geometry/geometry_type.hpp>
#include <mapnik/agg_renderer.hpp>

class test_datasource : public mapnik::memory_datasource
{
  public:
    test_datasource(mapnik::box2d<double> const& expected_query_bbox)
        : mapnik::memory_datasource(prepare_params()),
          expected_query_bbox_(expected_query_bbox)
    {}

    virtual mapnik::featureset_ptr features(mapnik::query const& q) const
    {
        mapnik::box2d<double> const& actual_bbox = q.get_bbox();
        REQUIRE(actual_bbox.minx() == Approx(expected_query_bbox_.minx()));
        REQUIRE(actual_bbox.miny() == Approx(expected_query_bbox_.miny()));
        REQUIRE(actual_bbox.maxx() == Approx(expected_query_bbox_.maxx()));
        REQUIRE(actual_bbox.maxy() == Approx(expected_query_bbox_.maxy()));
        return mapnik::memory_datasource::features(q);
    }

  private:
    mapnik::parameters prepare_params() const
    {
        mapnik::parameters params;
        params["type"] = "memory";
        return params;
    }

    mapnik::box2d<double> expected_query_bbox_;
};

TEST_CASE("feature_style_processor: buffer-size with scale-factor")
{
    SECTION("query extent with buffer-size should not be affected by scale-factor")
    {
        mapnik::box2d<double> const expected_query_bbox(-0.5, -0.5, 1.5, 1.5);

        using datasource_ptr = std::shared_ptr<test_datasource>;
        datasource_ptr datasource = std::make_shared<test_datasource>(expected_query_bbox);
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
        {
            mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, 2));
            mapnik::geometry::line_string<double> path;
            path.emplace_back(-10, -10);
            path.emplace_back(10, 10);
            feature->set_geometry(std::move(path));
            datasource->push(feature);
        }

        mapnik::Map map(256, 256);
        map.set_buffer_size(128);

        mapnik::feature_type_style lines_style;
        mapnik::rule rule;
        mapnik::line_symbolizer line_sym;
        rule.append(std::move(line_sym));
        lines_style.add_rule(std::move(rule));
        map.insert_style("lines", std::move(lines_style));

        mapnik::layer lyr("layer");
        lyr.set_datasource(datasource);
        lyr.add_style("lines");
        map.add_layer(lyr);

        mapnik::box2d<double> const map_extent(0, 0, 1, 1);
        map.zoom_to_box(map_extent);

        {
            mapnik::image_rgba8 image(map.width(), map.height());
            mapnik::agg_renderer<mapnik::image_rgba8> ren(map, image);
            ren.apply();
        }

        {
            // Rendering with scale-factor 2.0 should query data
            // with the same extent as with scale-factor 1.0.
            map.resize(map.width() * 2, map.height() * 2);
            mapnik::image_rgba8 image(map.width(), map.height());
            mapnik::agg_renderer<mapnik::image_rgba8> ren(map, image, 2.0);
            ren.apply();
        }
    }
}
