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
#include <mapnik/image_util.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/agg_renderer.hpp>


// stl
#include <fstream>
#include <iterator>

namespace fs = boost::filesystem;
using namespace mapnik;

const std::string srs_lcc="+proj=lcc +ellps=GRS80 +lat_0=49 +lon_0=-95 +lat+1=49 +lat_2=77 \
                       +datum=NAD83 +units=m +no_defs";
const std::string srs_merc="+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 \
                       +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over";

void prepare_map(Map & m)
{
    // Provinces (polygon)
    feature_type_style provpoly_style;
    {
        rule r;
        r.set_filter(parse_expression("[NAME_EN] = 'Ontario'"));
        {
            polygon_symbolizer poly_sym;
            put(poly_sym, keys::fill, color(250, 190, 183));
            r.append(std::move(poly_sym));
        }
        provpoly_style.add_rule(r);
    }
    {
        rule r;
        r.set_filter(parse_expression("[NOM_FR] = 'Québec'"));
        {
            polygon_symbolizer poly_sym;
            put(poly_sym, keys::fill, color(217, 235, 203));
            r.append(std::move(poly_sym));
        }
        provpoly_style.add_rule(r);
    }
    m.insert_style("provinces",provpoly_style);

    // Provinces (polyline)
    feature_type_style provlines_style;
    {
        rule r;
        {
            line_symbolizer line_sym;
            put(line_sym,keys::stroke,color(0,0,0));
            put(line_sym,keys::stroke_width,1.0);
            dash_array dash;
            dash.emplace_back(8,4);
            dash.emplace_back(2,2);
            dash.emplace_back(2,2);
            put(line_sym,keys::stroke_dasharray,dash);
            r.append(std::move(line_sym));
        }
        provlines_style.add_rule(r);
    }
    m.insert_style("provlines",provlines_style);

    // Drainage
    feature_type_style qcdrain_style;
    {
        rule r;
        r.set_filter(parse_expression("[HYC] = 8"));
        {
            polygon_symbolizer poly_sym;
            put(poly_sym, keys::fill, color(153, 204, 255));
            r.append(std::move(poly_sym));
        }
        qcdrain_style.add_rule(r);
    }
    m.insert_style("drainage",qcdrain_style);

    // Roads 3 and 4 (The "grey" roads)
    feature_type_style roads34_style;
    {
        rule r;
        r.set_filter(parse_expression("[CLASS] = 3 or [CLASS] = 4"));
        {
            line_symbolizer line_sym;
            put(line_sym,keys::stroke,color(171,158,137));
            put(line_sym,keys::stroke_width,2.0);
            put(line_sym,keys::stroke_linecap,ROUND_CAP);
            put(line_sym,keys::stroke_linejoin,ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        roads34_style.add_rule(r);
    }
    m.insert_style("smallroads",roads34_style);

    // Roads 2 (The thin yellow ones)
    feature_type_style roads2_style_1;
    {
        rule r;
        r.set_filter(parse_expression("[CLASS] = 2"));
        {
            line_symbolizer line_sym;
            put(line_sym,keys::stroke,color(171,158,137));
            put(line_sym,keys::stroke_width,4.0);
            put(line_sym,keys::stroke_linecap,ROUND_CAP);
            put(line_sym,keys::stroke_linejoin,ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        roads2_style_1.add_rule(r);
    }
    m.insert_style("road-border", roads2_style_1);

    feature_type_style roads2_style_2;
    {
        rule r;
        r.set_filter(parse_expression("[CLASS] = 2"));
        {
            line_symbolizer line_sym;
            put(line_sym,keys::stroke,color(255,250,115));
            put(line_sym,keys::stroke_width,2.0);
            put(line_sym,keys::stroke_linecap,ROUND_CAP);
            put(line_sym,keys::stroke_linejoin,ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        roads2_style_2.add_rule(r);
    }
    m.insert_style("road-fill", roads2_style_2);

    // Roads 1 (The big orange ones, the highways)
    feature_type_style roads1_style_1;
    {
        rule r;
        r.set_filter(parse_expression("[CLASS] = 1"));
        {
            line_symbolizer line_sym;
            put(line_sym,keys::stroke,color(188,149,28));
            put(line_sym,keys::stroke_width,7.0);
            put(line_sym,keys::stroke_linecap,ROUND_CAP);
            put(line_sym,keys::stroke_linejoin,ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        roads1_style_1.add_rule(r);
    }
    m.insert_style("highway-border", roads1_style_1);

    feature_type_style roads1_style_2;
    {
        rule r;
        r.set_filter(parse_expression("[CLASS] = 1"));
        {
            line_symbolizer line_sym;
            put(line_sym,keys::stroke,color(242,191,36));
            put(line_sym,keys::stroke_width,5.0);
            put(line_sym,keys::stroke_linecap,ROUND_CAP);
            put(line_sym,keys::stroke_linejoin,ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        roads1_style_2.add_rule(r);
    }
    m.insert_style("highway-fill", roads1_style_2);

    // Populated Places
    feature_type_style popplaces_style;
    {
        rule r;
        {
            text_symbolizer text_sym;
            text_placements_ptr placement_finder = std::make_shared<text_placements_dummy>();
            placement_finder->defaults.format->face_name = "DejaVu Sans Book";
            placement_finder->defaults.format->text_size = 10;
            placement_finder->defaults.format->fill = color(0,0,0);
            placement_finder->defaults.format->halo_fill = color(255,255,200);
            placement_finder->defaults.format->halo_radius = 1;
            placement_finder->defaults.set_old_style_expression(parse_expression("[GEONAME]"));
            put<text_placements_ptr>(text_sym, keys::text_placements_, placement_finder);
            r.append(std::move(text_sym));
        }
        popplaces_style.add_rule(r);
    }

    m.insert_style("popplaces",popplaces_style );

    // layers
    // Provincial  polygons
    {
        parameters p;
        p["type"]="shape";
        p["file"]="demo/data/boundaries";
        p["encoding"]="latin1";

        layer lyr("Provinces");
        lyr.set_datasource(datasource_cache::instance().create(p));
        lyr.set_srs(srs_lcc);
        lyr.add_style("provinces");
        m.add_layer(lyr);
    }

    // Drainage
    {
        parameters p;
        p["type"]="shape";
        p["file"]="demo/data/qcdrainage";
        layer lyr("Quebec Hydrography");
        lyr.set_datasource(datasource_cache::instance().create(p));
        lyr.set_srs(srs_lcc);
        lyr.add_style("drainage");
        m.add_layer(lyr);
    }

    {
        parameters p;
        p["type"]="shape";
        p["file"]="demo/data/ontdrainage";
        layer lyr("Ontario Hydrography");
        lyr.set_datasource(datasource_cache::instance().create(p));
        lyr.set_srs(srs_lcc);
        lyr.add_style("drainage");
        m.add_layer(lyr);
    }

    // Provincial boundaries
    {
        parameters p;
        p["type"]="shape";
        p["file"]="demo/data/boundaries_l";
        layer lyr("Provincial borders");
        lyr.set_srs(srs_lcc);
        lyr.set_datasource(datasource_cache::instance().create(p));
        lyr.add_style("provlines");
        m.add_layer(lyr);
    }

    // Roads
    {
        parameters p;
        p["type"]="shape";
        p["file"]="demo/data/roads";
        layer lyr("Roads");
        lyr.set_srs(srs_lcc);
        lyr.set_datasource(datasource_cache::instance().create(p));
        lyr.add_style("smallroads");
        lyr.add_style("road-border");
        lyr.add_style("road-fill");
        lyr.add_style("highway-border");
        lyr.add_style("highway-fill");

        m.add_layer(lyr);
    }

    // popplaces
    {
        parameters p;
        p["type"]="shape";
        p["file"]="demo/data/popplaces";
        p["encoding"] = "latin1";
        layer lyr("Populated Places");
        lyr.set_srs(srs_lcc);
        lyr.set_datasource(datasource_cache::instance().create(p));
        lyr.add_style("popplaces");
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
    std::cout << " looking for 'shape.input' plugin in ./plugins/input/" << "\n";
    datasource_cache::instance().register_datasources("plugins/input/");
    Map m(800,600);
    m.set_background(parse_color("white"));
    m.set_srs(srs_merc);
    prepare_map(m);
    m.zoom_to_box(box2d<double>(-8024477.28459,5445190.38849,-7381388.20071,5662941.44855));
    render_to_file(m, "path_element_test_case_1.svg");
    mapnik::image_32 buf(m.width(),m.height());
    mapnik::agg_renderer<mapnik::image_32> ren(m,buf);
    ren.apply();
    mapnik::save_to_file(buf,"path_element_test_case_1.png");
}
