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
#include <sstream>
#include <vector>
//#include <string>

#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>

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
    std::vector<std::string> svg_files;
    
    try
    {
        po::options_description desc("svg2png utility");
        desc.add_options()
            ("help,h", "produce usage message")
            ("version,V","print version string")
            ("verbose,v","verbose output")
            ("svg",po::value<std::vector<std::string> >(),"svg file to read")
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
            svg_files=vm["svg"].as< std::vector<std::string> >();
        }
        else
        {
            std::clog << "please provide an svg file!" << std::endl;
            return -1;
        }

        std::vector<std::string>::const_iterator itr = svg_files.begin();
        if (itr == svg_files.end())
        {
            std::clog << "no svg files to render" << std::endl;
            return 0;
        }
        while (itr != svg_files.end())
        {

            std::string svg_name (*itr++);

            boost::optional<mapnik::marker_ptr> marker_ptr = mapnik::marker_cache::instance()->find(svg_name, false);
            if (marker_ptr) {
            
                mapnik::marker marker  = **marker_ptr;
                if (marker.is_vector()) {
    
                    typedef agg::pixfmt_rgba32_plain pixfmt;
                    typedef agg::renderer_base<pixfmt> renderer_base;
                    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;
                    agg::rasterizer_scanline_aa<> ras_ptr;
                    agg::scanline_u8 sl;

                    double opacity = 1;
                    double scale_factor_ = .95;
                    int w = marker.width();
                    int h = marker.height();
                    mapnik::image_32 im(w,h);
                    agg::rendering_buffer buf(im.raw_data(), w, h, w * 4);
                    pixfmt pixf(buf);
                    renderer_base renb(pixf);

                    mapnik::box2d<double> const& bbox = (*marker.get_vector_data())->bounding_box();
                    mapnik::coord<double,2> c = bbox.center();
                    // center the svg marker on '0,0'
                    agg::trans_affine mtx = agg::trans_affine_translation(-c.x,-c.y);
                    // apply symbol transformation to get to map space
                    mtx *= agg::trans_affine_scaling(scale_factor_);
                    // render the marker at the center of the marker box
                    mtx.translate(0.5 * w, 0.5 * h);

                    mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage((*marker.get_vector_data())->source());
                    mapnik::svg::svg_path_adapter svg_path(stl_storage);
                    mapnik::svg::svg_renderer<mapnik::svg::svg_path_adapter,
                                 agg::pod_bvector<mapnik::svg::path_attributes>,
                                 renderer_solid,
                                 agg::pixfmt_rgba32_plain > svg_renderer_this(svg_path,
                                         (*marker.get_vector_data())->attributes());

                    svg_renderer_this.render(ras_ptr, sl, renb, mtx, opacity, bbox);

                    boost::algorithm::ireplace_last(svg_name,".svg",".png");
                    mapnik::save_to_file<mapnik::image_data_32>(im.data(),svg_name,"png");
                    std::ostringstream s;
                    s << "open " << svg_name;
                    return system(s.str().c_str());
                }
            }
        }
    }
    catch (...)
    {
        std::clog << "Exception of unknown type!" << std::endl;
        return -1;
    }

    return 0;
}

