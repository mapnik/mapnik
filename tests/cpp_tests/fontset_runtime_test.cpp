#include <boost/detail/lightweight_test.hpp>
#include <iostream>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/datasource_cache.hpp>
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
#include <mapnik/value_types.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/text/formatting/text.hpp>
#include <vector>
#include <algorithm>
#include <mapnik/make_unique.hpp>

// icu - for memory cleanup (to make valgrind happy)
#include "unicode/uclean.h"

#include "utils.hpp"

int main(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i=1;i<argc;++i)
    {
        args.push_back(argv[i]);
    }
    bool quiet = std::find(args.begin(), args.end(), "-q")!=args.end();

    try {
        BOOST_TEST(set_working_dir(args));

        // create a renderable map with a fontset and a text symbolizer
        // and do not register any fonts, to ensure the error thrown is reasonable
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
        ctx->push("name");
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx,1));
        mapnik::transcoder tr("utf-8");
        mapnik::value_unicode_string ustr = tr.transcode("hello world!");
        feature->put("name",ustr);
        auto pt = std::make_unique<mapnik::geometry_type>(mapnik::geometry_type::types::Point);
        pt->move_to(128,128);
        feature->add_geometry(pt.release());

        mapnik::parameters params;
        params["type"]="memory";
        auto ds = std::make_shared<mapnik::memory_datasource>(params);
        ds->push(feature);
        mapnik::Map m(256,256);
        mapnik::font_set fontset("fontset");
        // NOTE: this is a valid font, but will fail because none are registered
        fontset.add_face_name("DejaVu Sans Book");
        m.insert_fontset("fontset", fontset);
        mapnik::layer lyr("layer");
        lyr.set_datasource(ds);
        lyr.add_style("style");
        m.add_layer(lyr);
        mapnik::feature_type_style the_style;
        mapnik::rule r;
        mapnik::text_symbolizer text_sym;
        mapnik::text_placements_ptr placement_finder = std::make_shared<mapnik::text_placements_dummy>();
        placement_finder->defaults.format_defaults.face_name = "DejaVu Sans Book";
        placement_finder->defaults.format_defaults.text_size = 10.0;
        placement_finder->defaults.format_defaults.fill = mapnik::color(0,0,0);
        placement_finder->defaults.format_defaults.fontset = fontset;
        placement_finder->defaults.set_format_tree(std::make_shared<mapnik::formatting::text_node>(mapnik::parse_expression("[name]")));
        mapnik::put<mapnik::text_placements_ptr>(text_sym, mapnik::keys::text_placements_, placement_finder);
        r.append(std::move(text_sym));
        the_style.add_rule(std::move(r));
        m.insert_style("style", std::move(the_style) );
        m.zoom_to_box(mapnik::box2d<double>(-256,-256,
                                            256,256));
        mapnik::image_32 buf(m.width(),m.height());
        mapnik::agg_renderer<mapnik::image_32> ren(m,buf);
        ren.apply();
    } catch (std::exception const& ex) {
        BOOST_TEST_EQ(std::string(ex.what()),std::string("Unable to find specified font face 'DejaVu Sans Book' in font set: 'fontset'"));
    }
    u_cleanup();
    if (!::boost::detail::test_errors()) {
        if (quiet) std::clog << "\x1b[1;32m.\x1b[0m";
        else std::clog << "C++ fontset runtime: \x1b[1;32m✓ \x1b[0m\n";
        ::boost::detail::report_errors_remind().called_report_errors_function = true;
    } else {
        return ::boost::report_errors();
    }
}
