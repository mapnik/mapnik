/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/line_symbolizer.hpp>
#include <mapnik/polygon_symbolizer.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/image_util.hpp>

#if defined(HAVE_CAIRO)
#include <mapnik/cairo_renderer.hpp>
#include <mapnik/cairo_context.hpp>
#endif

#include <iostream>


int main ( int argc , char** argv)
{
    if (argc != 2)
    {
        std::cout << "usage: ./rundemo <mapnik_install_dir>\nUsually /usr/local\n";
        std::cout << "Warning: ./rundemo looks for data in ../data/,\nTherefore must be run from within the demo/c++ folder.\n";
        return EXIT_SUCCESS;
    }

    using namespace mapnik;
    const std::string srs_lcc="+proj=lcc +ellps=GRS80 +lat_0=49 +lon_0=-95 +lat+1=49 +lat_2=77 \
                           +datum=NAD83 +units=m +no_defs";
    const std::string srs_merc="+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 \
                           +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over";

    try {
        std::cout << " running demo ... \n";
        std::string mapnik_dir(argv[1]);
        std::cout << " looking for 'shape.input' plugin in... " << mapnik_dir << "/lib/mapnik/input/" << "\n";
        datasource_cache::instance().register_datasources(mapnik_dir + "/lib/mapnik/input/");
        std::cout << " looking for DejaVuSans font in... " << mapnik_dir << "/lib/mapnik/fonts/DejaVuSans.ttf" << "\n";
        freetype_engine::register_font(mapnik_dir + "/lib/mapnik/fonts/DejaVuSans.ttf");

        Map m(800,600);
        m.set_background(parse_color("white"));
        m.set_srs(srs_merc);
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

        // Populated Places

        feature_type_style popplaces_style;
        rule popplaces_rule;
        text_symbolizer popplaces_text_symbolizer(parse_expression("[GEONAME]"),"DejaVu Sans Book",10,color(0,0,0));
        popplaces_text_symbolizer.set_halo_fill(color(255,255,200));
        popplaces_text_symbolizer.set_halo_radius(1);
        popplaces_rule.append(popplaces_text_symbolizer);
        popplaces_style.add_rule(popplaces_rule);

        m.insert_style("popplaces",popplaces_style );

        // layers
        // Provincial  polygons
        {
            parameters p;
            p["type"]="shape";
            p["file"]="../data/boundaries";
            p["encoding"]="latin1";

            layer lyr("Provinces");
            lyr.set_datasource(datasource_cache::instance().create(p));
            lyr.add_style("provinces");
            lyr.set_srs(srs_lcc);
            m.add_layer(lyr);
        }

        // Drainage
        {
            parameters p;
            p["type"]="shape";
            p["file"]="../data/qcdrainage";
            layer lyr("Quebec Hydrography");
            lyr.set_datasource(datasource_cache::instance().create(p));
            lyr.set_srs(srs_lcc);
            lyr.add_style("drainage");
            m.add_layer(lyr);
        }

        {
            parameters p;
            p["type"]="shape";
            p["file"]="../data/ontdrainage";
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
            p["file"]="../data/boundaries_l";
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
            p["file"]="../data/roads";
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
            p["file"]="../data/popplaces";
            p["encoding"] = "latin1";
            layer lyr("Populated Places");
            lyr.set_srs(srs_lcc);
            lyr.set_datasource(datasource_cache::instance().create(p));
            lyr.add_style("popplaces");
            m.add_layer(lyr);
        }

        m.zoom_to_box(box2d<double>(-8024477.28459,5445190.38849,-7381388.20071,5662941.44855));

        image_32 buf(m.width(),m.height());
        agg_renderer<image_32> ren(m,buf);
        ren.apply();
        std::string msg("These maps have been rendered using AGG in the current directory:\n");
#ifdef HAVE_JPEG
        save_to_file(buf,"demo.jpg","jpeg");
        msg += "- demo.jpg\n";
#endif
#ifdef HAVE_PNG
        save_to_file(buf,"demo.png","png");
        save_to_file(buf,"demo256.png","png8");
        msg += "- demo.png\n";
        msg += "- demo256.png\n";
#endif
#ifdef HAVE_TIFF
        save_to_file(buf,"demo.tif","tiff");
        msg += "- demo.tif\n";
#endif
#ifdef HAVE_WEBP
        save_to_file(buf,"demo.webp","webp");
        msg += "- demo.webp\n";
#endif
        msg += "Have a look!\n";
        std::cout << msg;

#if defined(HAVE_CAIRO)
        // save to pdf/svg files
        save_to_cairo_file(m,"cairo-demo.pdf");
        save_to_cairo_file(m,"cairo-demo.svg");

        /* we could also do:

           save_to_cairo_file(m,"cairo-demo.png");

           but instead let's build up a surface for more flexibility
        */

        cairo_surface_ptr image_surface(
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32,m.width(),m.height()),
            cairo_surface_closer());
        double scale_factor = 1.0;
        cairo_ptr image_context(create_context(image_surface));
        mapnik::cairo_renderer<cairo_ptr> png_render(m,image_context,scale_factor);
        png_render.apply();
        // we can now write to png with cairo functionality
        cairo_surface_write_to_png(&*image_surface, "cairo-demo.png");
        // but we can also benefit from quantization by converting
        // to a mapnik image object and then saving that
        image_32 im(image_surface);
        save_to_file(im, "cairo-demo256.png","png8");
        cairo_surface_finish(&*image_surface);

        std::cout << "Three maps have been rendered using Cairo in the current directory:\n"
            "- cairo-demo.png\n"
            "- cairo-demo256.png\n"
            "- cairo-demo.pdf\n"
            "- cairo-demo.svg\n"
            "Have a look!\n";
#endif

    }
    catch ( const std::exception & ex )
    {
        std::cerr << "### std::exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch ( ... )
    {
        std::cerr << "### Unknown exception." << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
