#include <boost/version.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/map.hpp>
#include <mapnik/params.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/save_map.hpp>


int main( int, char*[] )
{
    // create a renderable map with a fontset and a text symbolizer
    // and do not register any fonts, to ensure the error thrown is reasonable
    mapnik::context_ptr ctx = boost::make_shared<mapnik::context_type>();
    ctx->push("name");
    mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx,1));
    mapnik::transcoder tr("utf-8");
    UnicodeString ustr = tr.transcode("hello world!");
    feature->put("name",ustr);
    mapnik::geometry_type * pt = new mapnik::geometry_type(mapnik::Point);
    pt->move_to(128,128);
    feature->add_geometry(pt);
    mapnik::datasource_ptr memory_ds = boost::make_shared<mapnik::memory_datasource>();
    mapnik::memory_datasource *cache = dynamic_cast<mapnik::memory_datasource *>(memory_ds.get());
    cache->push(feature);
    mapnik::Map m(256,256);
    mapnik::font_set fontset("fontset");
    // NOTE: this is a valid font, but will fail because none are registered
    fontset.add_face_name("DejaVu Sans Book");
    m.insert_fontset("fontset", fontset);
    mapnik::layer lyr("layer");
    lyr.set_datasource(memory_ds);
    lyr.add_style("style");
    m.addLayer(lyr);
    mapnik::feature_type_style the_style;
    mapnik::rule the_rule;
    mapnik::text_symbolizer text_sym(mapnik::parse_expression("[name]"),10,mapnik::color(0,0,0));
    text_sym.set_fontset(fontset);
    the_rule.append(text_sym);
    the_style.add_rule(the_rule);
    m.insert_style("style",the_style );
    m.zoom_to_box(mapnik::box2d<double>(-256,-256,
                                256,256));
    mapnik::image_32 buf(m.width(),m.height());
    mapnik::agg_renderer<mapnik::image_32> ren(m,buf);
    try {
        ren.apply();
    } catch (std::exception const& ex) {
        BOOST_TEST_EQ(std::string(ex.what()),std::string("No valid font face could be loaded for font set: 'fontset'"));
    }
    if (!::boost::detail::test_errors()) {
        std::clog << "C++ fontset runtime: \x1b[1;32m✓ \x1b[0m\n";
#if BOOST_VERSION >= 104600
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
#endif
    } else {
        return ::boost::report_errors();
    }
}
