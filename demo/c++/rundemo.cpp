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

// define before any includes
#define BOOST_SPIRIT_THREADSAFE

#include <mapnik/map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/cairo_renderer.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/config_error.hpp>

#include <iostream>


int main ( int argc , char** argv)
{    
    if (argc != 2)
    {
        std::cout << "usage: ./rundemo <mapnik_install_dir>\n";
        return EXIT_SUCCESS;
    }
    
    using namespace mapnik;
    try {
        std::cout << " running demo ... \n";
        std::string mapnik_dir(argv[1]);
        datasource_cache::instance()->register_datasources(mapnik_dir + "/lib/mapnik/input/"); 
        freetype_engine::register_font(mapnik_dir + "/lib/mapnik/fonts/DejaVuSans.ttf");
        
        Map m(800,600);
        
        mapnik::load_map(m,"/Users/artem/projects/openstreetmap/mapnik/openstreetmap_kleptog.xml");
        
        m.zoomToBox(Envelope<double>(-762470.6985688356,4004842.088052442,-761928.349698612,4005328.834769584));
        
        //Image32 buf(m.getWidth(),m.getHeight());
        //agg_renderer<Image32> ren(m,buf);
        //ren.apply();
        
        //save_to_file<ImageData32>(buf.data(),"demo.jpg","jpeg");
        //save_to_file<ImageData32>(buf.data(),"demo.png","png");
        //save_to_file<ImageData32>(buf.data(),"demo256.png","png256");
        //std::cout << "Three maps have been rendered in the current directory:\n"
        //  "- demo.jpg\n"
        //  "- demo.png\n"
        //  "- demo256.png\n"
        //  "Have a look!\n";

        // Cairo renderer

        // Pdf
        Cairo::RefPtr<Cairo::PdfSurface> pdf = Cairo::PdfSurface::create("mapnik.pdf",m.getWidth(),m.getHeight());
        mapnik::cairo_renderer<Cairo::Surface> cairo1(m, pdf);
        cairo1.apply();
        // Svg
        //Cairo::RefPtr<Cairo::SvgSurface> svg = Cairo::SvgSurface::create("demo.svg",m.getWidth(),m.getHeight());
        //mapnik::cairo_renderer<Cairo::Surface> cairo2(m, svg);  
        //cairo2.apply();
        // Png
        //Cairo::RefPtr<Cairo::ImageSurface> image = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,m.getWidth(),m.getHeight());
        //mapnik::cairo_renderer<Cairo::Surface> cairo3(m, image);  
        //cairo3.apply();
        
        //std::string filename = "demo_cairo.png";
        //image->write_to_png(filename);
        
        // Ps
        //Cairo::RefPtr<Cairo::PsSurface> ps = Cairo::PsSurface::create("demo.ps",m.getWidth(),m.getHeight());
        //mapnik::cairo_renderer<Cairo::Surface> cairo4(m, ps);
        //cairo4.apply();
        
    }
    catch ( const mapnik::config_error & ex )
    {
        std::cerr << "### Configuration error: " << ex.what();
        return EXIT_FAILURE;
    }
    catch ( const std::exception & ex )
    {
        std::cerr << "### std::exception: " << ex.what();
        return EXIT_FAILURE;
    }
    catch ( ... )
    {
        std::cerr << "### Unknown exception." << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
