#include "bench_framework.hpp"
#include <mapnik/map.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <stdexcept>
#include <mapnik/layer.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature_type_style.hpp>

#include <memory>

class test : public benchmark::test_case
{
public:
    test(mapnik::parameters const& params)
     : test_case(params) {}

    bool validate() const
    {
        return true;
    }
    bool operator()() const
    {
        mapnik::Map m(256,256,"epsg:3857");

        mapnik::parameters params;
        params["type"]="memory";
        auto ds = std::make_shared<mapnik::memory_datasource>(params);
        // add whitespace to trigger phony "reprojection"
        mapnik::layer lay("layer",m.srs() + " ");
        lay.set_datasource(ds);
        lay.add_style("style");
        m.add_layer(lay);
        // dummy style to ensure that layer is processed
        m.insert_style("style",mapnik::feature_type_style());
        // dummy bbox, but "valid" because minx and miny are less
        // with an invalid bbox then layer.visible() returns false
        // and the initial rendering setup is not run
        m.zoom_to_box(mapnik::box2d<double>(-1,-1,0,0));
        for (unsigned i=0;i<iterations_;++i)
        {
            mapnik::image_rgba8 im(256,256);
            mapnik::agg_renderer<mapnik::image_rgba8> ren(m,im);
            ren.apply();
        }
        return true;
    }
};

BENCHMARK(test,"rendering with reprojection")
