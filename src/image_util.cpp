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

//$Id: image_util.cpp 36 2005-04-05 14:32:18Z pavlenko $

extern "C"
{
#include <png.h>
}

// mapnik
#include <mapnik/image_util.hpp>
#include <mapnik/png_io.hpp>
#include <mapnik/jpeg_io.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/memory.hpp>
#include <mapnik/image_view.hpp>

#ifdef HAVE_CAIRO
#include <mapnik/cairo_renderer.hpp>
#endif

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

// stl
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace mapnik
{    
    template <typename T>
    std::string save_to_string(T const& image,
                               std::string const& type)
    {
        std::ostringstream ss(std::ios::out|std::ios::binary);
        save_to_stream(image, ss, type);
        return ss.str();
    }

    template <typename T>
    void save_to_file(T const& image,
                      std::string const& filename,
                      std::string const& type)
    {
        std::ofstream file (filename.c_str(), std::ios::out| std::ios::trunc|std::ios::binary);
        if (file)
        {
            save_to_stream(image, file, type);
        }
        else throw ImageWriterException("Could not write file to " + filename );
    }

    template <typename T>
    void save_to_stream(T const& image,
                      std::ostream & stream,
                      std::string const& type)
    {
        if (stream)
        {
            //all this should go into image_writer factory
            if (type == "png")  save_as_png(stream, image);
            else if (boost::algorithm::istarts_with(type, std::string("png256")) ||
                     boost::algorithm::istarts_with(type, std::string("png8"))
                     ) 
            {
                int colors  = 256;
                int trans_mode = -1;
                double gamma = -1;
                bool use_octree = true;
                if (type.length() > 6){
                    boost::char_separator<char> sep(":");
                    boost::tokenizer< boost::char_separator<char> > tokens(type, sep);
                    BOOST_FOREACH(string t, tokens)
                    {
                        if (t == "m=h")
                        {
                            use_octree = false;
                        }
                        if (t == "m=o")
                        {
                            use_octree = true;
                        }
                        if (boost::algorithm::istarts_with(t,std::string("c=")))
                        {
                            try 
                            {
                                colors = boost::lexical_cast<int>(t.substr(2));
                                if (colors < 0 || colors > 256)
                                    throw ImageWriterException("invalid color parameter: " + t.substr(2) + " out of bounds");
                            }
                            catch(boost::bad_lexical_cast &)
                            {
                                throw ImageWriterException("invalid color parameter: " + t.substr(2));
                            }
                        }
                        if (boost::algorithm::istarts_with(t, std::string("t=")))
                        {
                            try 
                            {
                                trans_mode= boost::lexical_cast<int>(t.substr(2));
                                if (trans_mode < 0 || trans_mode > 2)
                                    throw ImageWriterException("invalid trans_mode parameter: " + t.substr(2) + " out of bounds");
                            }
                            catch(boost::bad_lexical_cast &)
                            {
                                throw ImageWriterException("invalid trans_mode parameter: " + t.substr(2));
                            }
                        }
                        if (boost::algorithm::istarts_with(t, std::string("g=")))
                        {
                            try 
                            {
                                gamma= boost::lexical_cast<double>(t.substr(2));
                                if (gamma < 0)
                                    throw ImageWriterException("invalid gamma parameter: " + t.substr(2) + " out of bounds");
                            }
                            catch(boost::bad_lexical_cast &)
                            {
                                throw ImageWriterException("invalid gamma parameter: " + t.substr(2));
                            }
                        }
                    }

                }
                if (use_octree)
                    save_as_png256(stream, image, colors);
                else
                    save_as_png256_hex(stream, image, colors, trans_mode, gamma);
            }
            else if (boost::algorithm::istarts_with(type,std::string("jpeg")))
            {
                int quality = 85;
                try 
                {
                    if(type.substr(4).length() != 0)
                    {
                        quality = boost::lexical_cast<int>(type.substr(4));
                        if(quality<0 || quality>100)
                            throw ImageWriterException("invalid jpeg quality: " + type.substr(4) + " out of bounds");
                    }
                    save_as_jpeg(stream, quality, image); 
                } 
                catch(boost::bad_lexical_cast &)
                {
                    throw ImageWriterException("invalid jpeg quality: " + type.substr(4) + " not a number");
                }
            }
            else throw ImageWriterException("unknown file type: " + type);
        }
        else throw ImageWriterException("Could not write to empty stream" );
    }

	
    template <typename T>
    void save_to_file(T const& image,std::string const& filename)
    {
        std::string type = type_from_filename(filename);
        save_to_file<T>(image,filename,type);
    }


#if defined(HAVE_CAIRO)
    // TODO - move to separate cairo_io.hpp
    void save_to_cairo_file(mapnik::Map const& map, std::string const& filename)
    {
        std::string type = type_from_filename(filename);
        save_to_cairo_file(map,filename,type);
    }

    void save_to_cairo_file(mapnik::Map const& map,
                      std::string const& filename,
                      std::string const& type)
    {
        std::ofstream file (filename.c_str(), std::ios::out|std::ios::trunc|std::ios::binary);
        if (file)
        {
          Cairo::RefPtr<Cairo::Surface> surface;
          unsigned width = map.getWidth();
          unsigned height = map.getHeight();
          if (type == "pdf")
              surface = Cairo::PdfSurface::create(filename,width,height);
          else if (type == "svg")
              surface = Cairo::SvgSurface::create(filename,width,height);
          else if (type == "ps")
              surface = Cairo::PsSurface::create(filename,width,height);
          else if (type == "ARGB32")
              surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,width,height);
          else if (type == "RGB24")
              surface = Cairo::ImageSurface::create(Cairo::FORMAT_RGB24,width,height);
          else 
              throw ImageWriterException("unknown file type: " + type);    
          Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(surface);
    
          // TODO - expose as user option
          /*
          if (type == "ARGB32" || type == "RGB24") 
          { 
              context->set_antialias(Cairo::ANTIALIAS_NONE); 
          }
          */
          
    
          mapnik::cairo_renderer<Cairo::Context> ren(map, context);
          ren.apply();
    
          if (type == "ARGB32" || type == "RGB24") 
          { 
              surface->write_to_png(filename);
          }
          surface->finish();
        }
    }

#endif

    template void save_to_file<ImageData32>(ImageData32 const&,
                                            std::string const&,
                                            std::string const&);
                                            
    template void save_to_file<ImageData32>(ImageData32 const&,
                                            std::string const&);

    template std::string save_to_string<ImageData32>(ImageData32 const&,
                                                     std::string const&);

    template void save_to_file<image_view<ImageData32> > (image_view<ImageData32> const&,
                                                          std::string const&,
                                                          std::string const&);
   
    template void save_to_file<image_view<ImageData32> > (image_view<ImageData32> const&,
                                                          std::string const&);
   
    template std::string save_to_string<image_view<ImageData32> > (image_view<ImageData32> const&,
                                                                   std::string const&);

}
