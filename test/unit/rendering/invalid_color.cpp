#include "catch.hpp"

#include <iostream>
#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/agg_renderer.hpp>

#include <memory>

using namespace mapnik;

TEST_CASE("invalid color should not stop rendering") {

    mapnik::Map m(256,256);
    // add layer to map with one point
    {
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
        ctx->push("name");
        mapnik::feature_ptr feature = std::make_shared<feature_impl>(ctx,1);
        mapnik::transcoder tr("utf-8");
        mapnik::value_unicode_string ustr = tr.transcode("hello world!");
        feature->put("name",ustr);
        mapnik::geometry::point<double> pt(128,128);
        feature->set_geometry(std::move(pt));
        mapnik::parameters params;
        params["type"]="memory";
        auto ds = std::make_shared<mapnik::memory_datasource>(params);
        ds->push(feature);
        mapnik::layer lyr("layer");
        lyr.set_datasource(ds);
        lyr.add_style("style");
        m.add_layer(lyr);
    }
    // add style to map
    {
        mapnik::feature_type_style the_style;
        mapnik::rule r;
        mapnik::dot_symbolizer sym;
        mapnik::put(sym, mapnik::keys::fill, parse_expression("2"));
        r.append(std::move(sym));
        the_style.add_rule(std::move(r));
        m.insert_style("style", std::move(the_style) );        
    }
    m.zoom_to_box(mapnik::box2d<double>(-256,-256,
                                        256,256));
    mapnik::image_rgba8 buf(m.width(),m.height());
    mapnik::agg_renderer<mapnik::image_rgba8> ren(m,buf);
    ren.apply();

}
