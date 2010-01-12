/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
// $Id$

#include <mapnik/map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/config_error.hpp>

#if defined(HAVE_CAIRO)
// cairo
#include <mapnik/cairo_renderer.hpp>
#include <cairomm/surface.h>
#endif

#include <iostream>


int main ( int argc , char** argv)
{    
    if (argc != 2)
    {
        std::cout << "usage: ./rundemo <mapnik_install_dir>\nUsually /usr/local/lib/mapnik\n";
        std::cout << "Warning: ./rundemo looks for data in ../data/,\nTherefore must be run from within the demo/c++ folder.\n";
        return EXIT_SUCCESS;
    }
    
    using namespace mapnik;
    try {
        std::cout << " running demo ... \n";
        std::string mapnik_dir(argv[1]);
        std::cout << " looking for 'shape.input' plugin in... " << mapnik_dir << "/input/" << "\n";
        datasource_cache::instance()->register_datasources(mapnik_dir + "/input/"); 
        std::cout << " looking for DejaVuSans font in... " << mapnik_dir << "/fonts/DejaVuSans.ttf" << "\n";
        freetype_engine::register_font(mapnik_dir + "/fonts/DejaVuSans.ttf");
        
        Map m(800,600);
        m.set_background(color_factory::from_string("white"));
        
        // create styles

        // Provinces (polygon)
        feature_type_style provpoly_style;
       
        rule_type provpoly_rule_on;
        provpoly_rule_on.set_filter(create_filter("[NAME_EN] = 'Ontario'"));
        provpoly_rule_on.append(polygon_symbolizer(color(250, 190, 183)));
        provpoly_style.add_rule(provpoly_rule_on);
        
        rule_type provpoly_rule_qc;
        provpoly_rule_qc.set_filter(create_filter("[NOM_FR] = 'Qu\xe9""bec'"));
        provpoly_rule_qc.append(polygon_symbolizer(color(217, 235, 203)));
        provpoly_style.add_rule(provpoly_rule_qc);
        
        m.insert_style("provinces",provpoly_style);

        // Provinces (polyline)
        feature_type_style provlines_style;
        
        stroke provlines_stk (color(0,0,0),1.0);
        provlines_stk.add_dash(8, 4);
        provlines_stk.add_dash(2, 2);
        provlines_stk.add_dash(2, 2);
        
        rule_type provlines_rule;
        provlines_rule.append(line_symbolizer(provlines_stk));
        provlines_style.add_rule(provlines_rule);
        
        m.insert_style("provlines",provlines_style);
        
        // Drainage 
        feature_type_style qcdrain_style;
        
        rule_type qcdrain_rule;
        qcdrain_rule.set_filter(create_filter("[HYC] = 8"));
        qcdrain_rule.append(polygon_symbolizer(color(153, 204, 255)));
        qcdrain_style.add_rule(qcdrain_rule);
        
        m.insert_style("drainage",qcdrain_style);
        
        // Roads 3 and 4 (The "grey" roads)
        feature_type_style roads34_style;    
        rule_type roads34_rule;
        roads34_rule.set_filter(create_filter("[CLASS] = 3 or [CLASS] = 4"));
        stroke roads34_rule_stk(color(171,158,137),2.0);
        roads34_rule_stk.set_line_cap(ROUND_CAP);
        roads34_rule_stk.set_line_join(ROUND_JOIN);
        roads34_rule.append(line_symbolizer(roads34_rule_stk));
        roads34_style.add_rule(roads34_rule);
        
        m.insert_style("smallroads",roads34_style);
        

        // Roads 2 (The thin yellow ones)
        feature_type_style roads2_style_1;
        rule_type roads2_rule_1;
        roads2_rule_1.set_filter(create_filter("[CLASS] = 2"));
        stroke roads2_rule_stk_1(color(171,158,137),4.0);
        roads2_rule_stk_1.set_line_cap(ROUND_CAP);
        roads2_rule_stk_1.set_line_join(ROUND_JOIN);
        roads2_rule_1.append(line_symbolizer(roads2_rule_stk_1));
        roads2_style_1.add_rule(roads2_rule_1);
        
        m.insert_style("road-border", roads2_style_1);
        
        feature_type_style roads2_style_2;
        rule_type roads2_rule_2;
        roads2_rule_2.set_filter(create_filter("[CLASS] = 2"));
        stroke roads2_rule_stk_2(color(255,250,115),2.0);
        roads2_rule_stk_2.set_line_cap(ROUND_CAP);
        roads2_rule_stk_2.set_line_join(ROUND_JOIN);
        roads2_rule_2.append(line_symbolizer(roads2_rule_stk_2));
        roads2_style_2.add_rule(roads2_rule_2);
        
        m.insert_style("road-fill", roads2_style_2);
        
        // Roads 1 (The big orange ones, the highways)
        feature_type_style roads1_style_1;
        rule_type roads1_rule_1;
        roads1_rule_1.set_filter(create_filter("[CLASS] = 1"));
        stroke roads1_rule_stk_1(color(188,149,28),7.0);
        roads1_rule_stk_1.set_line_cap(ROUND_CAP);
        roads1_rule_stk_1.set_line_join(ROUND_JOIN);
        roads1_rule_1.append(line_symbolizer(roads1_rule_stk_1));
        roads1_style_1.add_rule(roads1_rule_1);
        m.insert_style("highway-border", roads1_style_1);
        
        feature_type_style roads1_style_2;
        rule_type roads1_rule_2;
        roads1_rule_2.set_filter(create_filter("[CLASS] = 1"));
        stroke roads1_rule_stk_2(color(242,191,36),5.0);
        roads1_rule_stk_2.set_line_cap(ROUND_CAP);
        roads1_rule_stk_2.set_line_join(ROUND_JOIN);
        roads1_rule_2.append(line_symbolizer(roads1_rule_stk_2));
        roads1_style_2.add_rule(roads1_rule_2);
        m.insert_style("highway-fill", roads1_style_2);
        
        // Populated Places
        
        feature_type_style popplaces_style;
        rule_type popplaces_rule;
        text_symbolizer popplaces_text_symbolizer("GEONAME","DejaVu Sans Book",10,color(0,0,0));
        popplaces_text_symbolizer.set_halo_fill(color(255,255,200));
        popplaces_text_symbolizer.set_halo_radius(1);
        popplaces_rule.append(popplaces_text_symbolizer);
        popplaces_style.add_rule(popplaces_rule);
        
        m.insert_style("popplaces",popplaces_style );
        
        // Layers
        // Provincial  polygons
        {
            parameters p;
            p["type"]="shape";
            p["file"]="../data/boundaries";
            
            Layer lyr("Provinces"); 
            lyr.set_datasource(datasource_cache::instance()->create(p));
            lyr.add_style("provinces");    
            m.addLayer(lyr);
        }
        
        // Drainage
        {
            parameters p;
            p["type"]="shape";
            p["file"]="../data/qcdrainage";
            Layer lyr("Quebec Hydrography");
            lyr.set_datasource(datasource_cache::instance()->create(p));
            lyr.add_style("drainage");    
            m.addLayer(lyr);
        }
        
        {
            parameters p;
            p["type"]="shape";
            p["file"]="../data/ontdrainage";
            
            Layer lyr("Ontario Hydrography"); 
            lyr.set_datasource(datasource_cache::instance()->create(p));
            lyr.add_style("drainage");    
            m.addLayer(lyr);
        }
        
        // Provincial boundaries
        {
            parameters p;
            p["type"]="shape";
            p["file"]="../data/boundaries_l";
            Layer lyr("Provincial borders"); 
            lyr.set_datasource(datasource_cache::instance()->create(p));
            lyr.add_style("provlines");    
            m.addLayer(lyr);
        }
        
        // Roads
        {
            parameters p;
            p["type"]="shape";
            p["file"]="../data/roads";        
            Layer lyr("Roads"); 
            lyr.set_datasource(datasource_cache::instance()->create(p));
            lyr.add_style("smallroads");
            lyr.add_style("road-border");
            lyr.add_style("road-fill");
            lyr.add_style("highway-border");
            lyr.add_style("highway-fill");

            m.addLayer(lyr);        
        }
        // popplaces
        {
            parameters p;
            p["type"]="shape";
            p["file"]="../data/popplaces";
            p["encoding"] = "latin1";
            Layer lyr("Populated Places");
            lyr.set_datasource(datasource_cache::instance()->create(p));
            lyr.add_style("popplaces");    
            m.addLayer(lyr);
        }
        
        m.zoomToBox(Envelope<double>(1405120.04127408,-247003.813399447,
                                     1706357.31328276,-25098.593149577));
        
        Image32 buf(m.getWidth(),m.getHeight());
        agg_renderer<Image32> ren(m,buf);
        ren.apply();
        
        save_to_file<ImageData32>(buf.data(),"demo.jpg","jpeg");
        save_to_file<ImageData32>(buf.data(),"demo.png","png");
        save_to_file<ImageData32>(buf.data(),"demo256.png","png256");
        std::cout << "Three maps have been rendered using AGG in the current directory:\n"
           "- demo.jpg\n"
           "- demo.png\n"
           "- demo256.png\n"
           "Have a look!\n";

        #if defined(HAVE_CAIRO)
        Cairo::RefPtr<Cairo::ImageSurface> image_surface;

        image_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, m.getWidth(),m.getHeight());
        cairo_renderer<Cairo::Surface> png_render(m, image_surface);
        png_render.apply();
        image_surface->write_to_png("cairo-demo.png");

        Image32 im(image_surface);
        save_to_file(im, "cairo-demo256.png","png256");

        Cairo::RefPtr<Cairo::Surface> surface;
        surface = Cairo::PdfSurface::create("cairo-demo.pdf", m.getWidth(),m.getHeight());
        cairo_renderer<Cairo::Surface> pdf_render(m, surface);
        pdf_render.apply();

        surface = Cairo::SvgSurface::create("cairo-demo.svg", m.getWidth(),m.getHeight());
        cairo_renderer<Cairo::Surface> svg_render(m, surface);
        svg_render.apply();

        std::cout << "Three maps have been rendered using Cairo in the current directory:\n"
           "- cairo-demo.png\n"
           "- cairo-demo256.png\n"
           "- cairo-demo.pdf\n"
           "- cairo-demo.svg\n"
           "Have a look!\n";
        #endif

    }
    catch ( const mapnik::config_error & ex )
    {
        std::cerr << "### Configuration error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
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
