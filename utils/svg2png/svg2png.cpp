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
#include <string>

#include <mapnik/version.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop

#include "agg_rasterizer_scanline_aa.h"
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_renderer_base.h"
#include "agg_pixfmt_rgba.h"
#include "agg_scanline_u.h"

#include <libxml/parser.h> // for xmlInitParser(), xmlCleanupParser()


int main (int argc,char** argv)
{
    namespace po = boost::program_options;

    bool verbose = false;
    bool auto_open = false;
    int return_value = 0;
    std::vector<std::string> svg_files;
    mapnik::logger::instance().set_severity(mapnik::logger::error);

    try
    {
        po::options_description desc("svg2png utility");
        desc.add_options()
            ("help,h", "produce usage message")
            ("version,V","print version string")
            ("verbose,v","verbose output")
            ("open","automatically open the file after rendering (os x only)")
            ("svg",po::value<std::vector<std::string> >(),"svg file to read")
            ;

        po::positional_options_description p;
        p.add("svg",-1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("version"))
        {
            std::clog <<"version " << MAPNIK_VERSION_STRING << std::endl;
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

        if (vm.count("open"))
        {
            auto_open = true;
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

        xmlInitParser();

        while (itr != svg_files.end())
        {
            std::string svg_name (*itr++);
            if (verbose)
            {
                std::clog << "found: " << svg_name << "\n";
            }

            boost::optional<mapnik::marker_ptr> marker_ptr =
                mapnik::marker_cache::instance().find(svg_name, false);
            if (!marker_ptr)
            {
                std::clog << "svg2png error: could not open: '" << svg_name << "'\n";
                return_value = -1;
                continue;
            }
            mapnik::marker marker = **marker_ptr;
            if (!marker.is_vector())
            {
                std::clog << "svg2png error: '" << svg_name << "' is not a valid vector!\n";
                return_value = -1;
                continue;
            }

            using pixfmt = agg::pixfmt_rgba32_pre;
            using renderer_base = agg::renderer_base<pixfmt>;
            using renderer_solid = agg::renderer_scanline_aa_solid<renderer_base>;
            agg::rasterizer_scanline_aa<> ras_ptr;
            agg::scanline_u8 sl;

            double opacity = 1;
            int w = marker.width();
            int h = marker.height();
            if (verbose)
            {
                std::clog << "found width of '" << w << "' and height of '" << h << "'\n";
            }
            // 10 pixel buffer to avoid edge clipping of 100% svg's
            mapnik::image_32 im(w+0,h+0);
            agg::rendering_buffer buf(im.raw_data(), im.width(), im.height(), im.width() * 4);
            pixfmt pixf(buf);
            renderer_base renb(pixf);

            mapnik::box2d<double> const& bbox = (*marker.get_vector_data())->bounding_box();
            mapnik::coord<double,2> c = bbox.center();
            // center the svg marker on '0,0'
            agg::trans_affine mtx = agg::trans_affine_translation(-c.x,-c.y);
            // render the marker at the center of the marker box
            mtx.translate(0.5 * im.width(), 0.5 * im.height());

            mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage((*marker.get_vector_data())->source());
            mapnik::svg::svg_path_adapter svg_path(stl_storage);
            mapnik::svg::svg_renderer_agg<mapnik::svg::svg_path_adapter,
                agg::pod_bvector<mapnik::svg::path_attributes>,
                renderer_solid,
                agg::pixfmt_rgba32_pre > svg_renderer_this(svg_path,
                                                           (*marker.get_vector_data())->attributes());

            svg_renderer_this.render(ras_ptr, sl, renb, mtx, opacity, bbox);

            boost::algorithm::ireplace_last(svg_name,".svg",".png");
            im.demultiply();
            mapnik::save_to_file<mapnik::image_data_32>(im.data(),svg_name,"png");
            if (auto_open)
            {
                std::ostringstream s;
#ifdef DARWIN
                s << "open " << svg_name;
#else
                s << "xdg-open " << svg_name;
#endif
                int ret = system(s.str().c_str());
                if (ret != 0)
                    return_value = ret;
            }
            std::clog << "rendered to: " << svg_name << "\n";
        }
    }
    catch (...)
    {
        std::clog << "Exception of unknown type!" << std::endl;
        xmlCleanupParser();
        return -1;
    }

    // only call this once, on exit
    // to make sure valgrind output is clean
    // http://xmlsoft.org/xmlmem.html
    xmlCleanupParser();
    return return_value;
}
