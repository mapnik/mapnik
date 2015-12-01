/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop

#include "agg_rasterizer_scanline_aa.h"
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_renderer_base.h"
#include "agg_pixfmt_rgba.h"
#include "agg_scanline_u.h"

struct main_marker_visitor
{
    main_marker_visitor(std::string const& svg_name,
                        bool verbose,
                        bool auto_open)
        : svg_name_(svg_name),
          verbose_(verbose),
          auto_open_(auto_open) {}

    int operator() (mapnik::marker_svg const& marker)
    {
        using pixfmt = agg::pixfmt_rgba32_pre;
        using renderer_base = agg::renderer_base<pixfmt>;
        using renderer_solid = agg::renderer_scanline_aa_solid<renderer_base>;
        agg::rasterizer_scanline_aa<> ras_ptr;
        agg::scanline_u8 sl;

        double opacity = 1;
        int w = marker.width();
        int h = marker.height();
        if (verbose_)
        {
            std::clog << "found width of '" << w << "' and height of '" << h << "'\n";
        }
        // 10 pixel buffer to avoid edge clipping of 100% svg's
        mapnik::image_rgba8 im(w+0,h+0);
        agg::rendering_buffer buf(im.bytes(), im.width(), im.height(), im.row_size());
        pixfmt pixf(buf);
        renderer_base renb(pixf);

        mapnik::box2d<double> const& bbox = marker.get_data()->bounding_box();
        mapnik::coord<double,2> c = bbox.center();
        // center the svg marker on '0,0'
        agg::trans_affine mtx = agg::trans_affine_translation(-c.x,-c.y);
        // render the marker at the center of the marker box
        mtx.translate(0.5 * im.width(), 0.5 * im.height());

        mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(marker.get_data()->source());
        mapnik::svg::svg_path_adapter svg_path(stl_storage);
        mapnik::svg::svg_renderer_agg<mapnik::svg::svg_path_adapter,
            agg::pod_bvector<mapnik::svg::path_attributes>,
            renderer_solid,
            agg::pixfmt_rgba32_pre > svg_renderer_this(svg_path,
                                                       marker.get_data()->attributes());

        svg_renderer_this.render(ras_ptr, sl, renb, mtx, opacity, bbox);

        std::string png_name(svg_name_);
        boost::algorithm::ireplace_last(png_name,".svg",".png");
        demultiply_alpha(im);
        mapnik::save_to_file<mapnik::image_rgba8>(im,png_name,"png");
        int status = 0;
        if (auto_open_)
        {
            std::ostringstream s;
#ifdef DARWIN
            s << "open " << png_name;
#else
            s << "xdg-open " << png_name;
#endif
            int ret = std::system(s.str().c_str());
            if (ret != 0)
                status = ret;
        }
        std::clog << "rendered to: " << png_name << "\n";
        return status;
    }

    // default
    template <typename T>
    int operator() (T const&)
    {
        std::clog << "svg2png error: '" << svg_name_ << "' is not a valid vector!\n";
        return -1;
    }

  private:
    std::string const& svg_name_;
    bool verbose_;
    bool auto_open_;
};

int main (int argc,char** argv)
{
    namespace po = boost::program_options;

    bool verbose = false;
    bool auto_open = false;
    int status = 0;
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

        while (itr != svg_files.end())
        {
            std::string svg_name (*itr++);
            if (verbose)
            {
                std::clog << "found: " << svg_name << "\n";
            }

            std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(svg_name, false);
            main_marker_visitor visitor(svg_name, verbose, auto_open);
            status = mapnik::util::apply_visitor(visitor, *marker);
        }
    }
    catch (std::exception const& ex)
    {
        std::clog << "Exception caught:" << ex.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::clog << "Exception of unknown type!" << std::endl;
        return -1;
    }
    return status;
}
