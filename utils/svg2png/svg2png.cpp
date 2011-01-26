/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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

#include <iostream>
#include <vector>
#include <string>

#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <boost/array.hpp>

#include "agg_rasterizer_scanline_aa.h"
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_renderer_base.h"
#include "agg_pixfmt_rgba.h"
#include "agg_scanline_u.h"


int main (int argc,char** argv) 
{
    namespace po = boost::program_options;
    
    bool verbose=false;
    std::string svg;
    
    try
    {
        po::options_description desc("svg2png utility");
        desc.add_options()
            ("help,h", "produce usage message")
            ("version,V","print version string")
            ("verbose,v","verbose output")
            ("svg",po::value<std::string>(),"svg file to read")
            ;
        
        po::positional_options_description p;
        p.add("svg",-1);
        po::variables_map vm;        
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);
        
        if (vm.count("version"))
        {
            std::clog<<"version 0.3.0" << std::endl;
            return 1;
        }

        if (vm.count("help")) 
        {
            std::clog << desc << std::endl;
            return 1;
        }
        if (vm.count("verbose")) 
        {
            verbose = true;
        }
        
        if (vm.count("svg"))
        {
            svg=vm["svg"].as<std::string>();
        }
        else
        {
            std::clog << "please provide an svg file!" << std::endl;
            return -1;
        }
        
        boost::optional<mapnik::marker_ptr> marker_ptr = mapnik::marker_cache::instance()->find(svg, false);
        if (marker_ptr) {
        
            mapnik::marker marker  = **marker_ptr;
            if (marker.is_vector()) {

                int width_ = 600;//marker.width() + 10;
                int height_ = 400;//marker.height() + 10;
                mapnik::image_32 pixmap_(width_,height_);
                agg::rasterizer_scanline_aa<> ras_ptr;
                
                typedef agg::pixfmt_rgba32_plain pixfmt;
                
                typedef agg::renderer_base<pixfmt> renderer_base;
        
        
                ras_ptr.reset();
                ras_ptr.gamma(agg::gamma_linear());
                agg::scanline_u8 sl;
                agg::rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_ * 4);
                pixfmt pixf(buf);
                renderer_base renb(pixf);
        
                mapnik::box2d<double> const& bbox = (*marker.get_vector_data())->bounding_box();
                double x1 = bbox.minx();
                double y1 = bbox.miny();
                double x2 = bbox.maxx();
                double y2 = bbox.maxy();
        
                
                agg::trans_affine recenter = agg::trans_affine_translation(0.5*(marker.width()-(x1+x2)),0.5*(marker.height()-(y1+y2)));
        
                mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage((*marker.get_vector_data())->source());
                
                mapnik::svg::svg_path_adapter svg_path(stl_storage);
                mapnik::svg::svg_renderer<mapnik::svg::svg_path_adapter,
                             agg::pod_bvector<mapnik::svg::path_attributes> > svg_renderer_this(svg_path,
                                     (*marker.get_vector_data())->attributes());
                agg::trans_affine tr;
                
                /*boost::array<double,6> matrix_;
                matrix_[0] = 1.0;
                matrix_[1] = 0.0;
                matrix_[2] = 0.0;
                matrix_[3] = 1.0;
                matrix_[4] = 0.0;
                matrix_[5] = 0.0;
                tr.load_from(&matrix_[0]);
                */

                agg::trans_affine mtx = recenter * tr;
                double scale_factor_ = 1;
                double opacity = 1;
                mtx *= agg::trans_affine_scaling(scale_factor_);
                //mtx *= agg::trans_affine_translation(width_/2, height_/2);
        
                svg_renderer_this.render(ras_ptr, sl, renb, mtx, opacity, bbox);
                
                mapnik::save_to_file<mapnik::image_data_32>(pixmap_.data(),"test.png","png");
                system("open test.png");
         
            
            }
        }        
        
    }
    catch (...)
    {
        std::clog << "Exception of unknown type!" << std::endl;
        return -1;
    }
        
    std::clog << "done!" << std::endl;
    return 0;
}

