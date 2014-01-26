#undef BOOST_SPIRIT_NO_PREDEFINED_TERMINALS
#define BOOST_TEST_MODULE path_element_tests

// boost.test
#include <boost/test/included/unit_test.hpp>

// boost.filesystem
#include <boost/filesystem.hpp>

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/svg/output/svg_renderer.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/color_factory.hpp>

// stl
#include <fstream>
#include <iterator>

namespace fs = boost::filesystem;
using namespace mapnik;

void prepare_map(Map& m)
{
    const std::string mapnik_dir("/usr/local/lib/mapnik/");
    std::cout << " looking for 'shape.input' plugin in... " << mapnik_dir << "input/" << "\n";
    datasource_cache::instance().register_datasources(mapnik_dir + "input/");

    // create styles

    // Provinces (polygon)
    feature_type_style provpoly_style;

    rule provpoly_rule_on;
    provpoly_rule_on.set_filter(parse_expression("[NAME_EN] = 'Ontario'"));
    provpoly_rule_on.append(polygon_symbolizer(color(250, 190, 183)));
    provpoly_style.add_rule(provpoly_rule_on);

    rule provpoly_rule_qc;
    provpoly_rule_qc.set_filter(parse_expression("[NOM_FR] = 'Québec'"));
    provpoly_rule_qc.append(polygon_symbolizer(color(217, 235, 203)));
    provpoly_style.add_rule(provpoly_rule_qc);

    m.insert_style("provinces",provpoly_style);

    // Provinces (polyline)
    feature_type_style provlines_style;

    stroke provlines_stk (color(0,0,0),1.0);
    provlines_stk.add_dash(8, 4);
    provlines_stk.add_dash(2, 2);
    provlines_stk.add_dash(2, 2);

    rule provlines_rule;
    provlines_rule.append(line_symbolizer(provlines_stk));
    provlines_style.add_rule(provlines_rule);

    m.insert_style("provlines",provlines_style);

    // Drainage
    feature_type_style qcdrain_style;

    rule qcdrain_rule;
    qcdrain_rule.set_filter(parse_expression("[HYC] = 8"));
    qcdrain_rule.append(polygon_symbolizer(color(153, 204, 255)));
    qcdrain_style.add_rule(qcdrain_rule);

    m.insert_style("drainage",qcdrain_style);

    // Roads 3 and 4 (The "grey" roads)
    feature_type_style roads34_style;
    rule roads34_rule;
    roads34_rule.set_filter(parse_expression("[CLASS] = 3 or [CLASS] = 4"));
    stroke roads34_rule_stk(color(171,158,137),2.0);
    roads34_rule_stk.set_line_cap(ROUND_CAP);
    roads34_rule_stk.set_line_join(ROUND_JOIN);
    roads34_rule.append(line_symbolizer(roads34_rule_stk));
    roads34_style.add_rule(roads34_rule);

    m.insert_style("smallroads",roads34_style);

    // Roads 2 (The thin yellow ones)
    feature_type_style roads2_style_1;
    rule roads2_rule_1;
    roads2_rule_1.set_filter(parse_expression("[CLASS] = 2"));
    stroke roads2_rule_stk_1(color(171,158,137),4.0);
    roads2_rule_stk_1.set_line_cap(ROUND_CAP);
    roads2_rule_stk_1.set_line_join(ROUND_JOIN);
    roads2_rule_1.append(line_symbolizer(roads2_rule_stk_1));
    roads2_style_1.add_rule(roads2_rule_1);

    m.insert_style("road-border", roads2_style_1);

    feature_type_style roads2_style_2;
    rule roads2_rule_2;
    roads2_rule_2.set_filter(parse_expression("[CLASS] = 2"));
    stroke roads2_rule_stk_2(color(255,250,115),2.0);
    roads2_rule_stk_2.set_line_cap(ROUND_CAP);
    roads2_rule_stk_2.set_line_join(ROUND_JOIN);
    roads2_rule_2.append(line_symbolizer(roads2_rule_stk_2));
    roads2_style_2.add_rule(roads2_rule_2);

    m.insert_style("road-fill", roads2_style_2);

    // Roads 1 (The big orange ones, the highways)
    feature_type_style roads1_style_1;
    rule roads1_rule_1;
    roads1_rule_1.set_filter(parse_expression("[CLASS] = 1"));
    stroke roads1_rule_stk_1(color(188,149,28),7.0);
    roads1_rule_stk_1.set_line_cap(ROUND_CAP);
    roads1_rule_stk_1.set_line_join(ROUND_JOIN);
    roads1_rule_1.append(line_symbolizer(roads1_rule_stk_1));
    roads1_style_1.add_rule(roads1_rule_1);
    m.insert_style("highway-border", roads1_style_1);

    feature_type_style roads1_style_2;
    rule roads1_rule_2;
    roads1_rule_2.set_filter(parse_expression("[CLASS] = 1"));
    stroke roads1_rule_stk_2(color(242,191,36),5.0);
    roads1_rule_stk_2.set_line_cap(ROUND_CAP);
    roads1_rule_stk_2.set_line_join(ROUND_JOIN);
    roads1_rule_2.append(line_symbolizer(roads1_rule_stk_2));
    roads1_style_2.add_rule(roads1_rule_2);
    m.insert_style("highway-fill", roads1_style_2);

    // layers
    // Provincial  polygons
    {
        parameters p;
        p["type"]="shape";
        p["file"]="../../../demo/data/boundaries";

        layer lyr("Provinces");
        lyr.set_datasource(datasource_cache::instance().create(p));
        lyr.add_style("provinces");
        m.add_layer(lyr);
    }

    // Drainage
    {
        parameters p;
        p["type"]="shape";
        p["file"]="../../../demo/data/qcdrainage";
        layer lyr("Quebec Hydrography");
        lyr.set_datasource(datasource_cache::instance().create(p));
        lyr.add_style("drainage");
        m.add_layer(lyr);
    }

    {
        parameters p;
        p["type"]="shape";
        p["file"]="../../../demo/data/ontdrainage";

        layer lyr("Ontario Hydrography");
        lyr.set_datasource(datasource_cache::instance().create(p));
        lyr.add_style("drainage");
        m.add_layer(lyr);
    }

    // Provincial boundaries
    {
        parameters p;
        p["type"]="shape";
        p["file"]="../../../demo/data/boundaries_l";
        layer lyr("Provincial borders");
        lyr.set_datasource(datasource_cache::instance().create(p));
        lyr.add_style("provlines");
        m.add_layer(lyr);
    }

    // Roads
    {
        parameters p;
        p["type"]="shape";
        p["file"]="../../../demo/data/roads";
        layer lyr("Roads");
        lyr.set_datasource(datasource_cache::instance().create(p));
        lyr.add_style("smallroads");
        lyr.add_style("road-border");
        lyr.add_style("road-fill");
        lyr.add_style("highway-border");
        lyr.add_style("highway-fill");

        m.add_layer(lyr);
    }
}

void render_to_file(Map const& m, const std::string output_filename)
{
    std::ofstream output_stream(output_filename.c_str());

    if(output_stream)
    {
        typedef svg_renderer<std::ostream_iterator<char> > svg_ren;

        std::ostream_iterator<char> output_stream_iterator(output_stream);

        svg_ren renderer(m, output_stream_iterator);
        renderer.apply();

        output_stream.close();

        fs::path output_filename_path =
            fs::system_complete(fs::path(".")) / fs::path(output_filename);

        BOOST_CHECK_MESSAGE(fs::exists(output_filename_path),
                            "File '"+output_filename_path.string()+"' was created.");
    }
    else
    {
        BOOST_FAIL("Could not create create/open file '"+output_filename+"'.");
    }
}

BOOST_AUTO_TEST_CASE(path_element_test_case_1)
{
    Map m(800,600);
    m.set_background(parse_color("steelblue"));

    prepare_map(m);

    //m.zoom_to_box(box2d<double>(1405120.04127408, -247003.813399447,
    //1706357.31328276, -25098.593149577));
    m.zoom_all();
    render_to_file(m, "path_element_test_case_1.svg");
}
