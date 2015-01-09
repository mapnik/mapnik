/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

// mapnik
#include <mapnik/image_util.hpp>
#include <mapnik/image_util_jpeg.hpp>
#include <mapnik/image_util_png.hpp>
#include <mapnik/image_util_tiff.hpp>
#include <mapnik/image_util_webp.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/memory.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/palette.hpp>
#include <mapnik/map.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/variant.hpp>

#ifdef HAVE_CAIRO
#include <mapnik/cairo/cairo_renderer.hpp>
#include <cairo.h>
#ifdef CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif
#ifdef CAIRO_HAS_PS_SURFACE
#include <cairo-ps.h>
#endif
#ifdef CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>
#endif
#endif

// boost
#include <boost/tokenizer.hpp>

// stl
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace mapnik
{


template <typename T>
std::string save_to_string(T const& image,
                           std::string const& type,
                           rgba_palette const& palette)
{
    std::ostringstream ss(std::ios::out|std::ios::binary);
    save_to_stream(image, ss, type, palette);
    return ss.str();
}

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
                  std::string const& type,
                  rgba_palette const& palette)
{
    std::ofstream file (filename.c_str(), std::ios::out| std::ios::trunc|std::ios::binary);
    if (file)
    {
        save_to_stream(image, file, type, palette);
    }
    else throw ImageWriterException("Could not write file to " + filename );
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
                    std::string const& type,
                    rgba_palette const& palette)
{
    if (stream && image.width() > 0 && image.height() > 0)
    {
        std::string t = type;
        std::transform(t.begin(), t.end(), t.begin(), ::tolower);
        if (t == "png" || boost::algorithm::starts_with(t, "png"))
        {
            png_saver_pal visitor(stream, t, palette);
            visitor(image);
            //mapnik::util::apply_visitor(visitor, image);
        }
        else if (boost::algorithm::starts_with(t, "tif"))
        {
            throw ImageWriterException("palettes are not currently supported when writing to tiff format (yet)");
        }
        else if (boost::algorithm::starts_with(t, "jpeg"))
        {
            throw ImageWriterException("palettes are not currently supported when writing to jpeg format");
        }
        else throw ImageWriterException("unknown file type: " + type);
    }
    else throw ImageWriterException("Could not write to empty stream" );
}

template <typename T>
void save_to_stream(T const& image,
                    std::ostream & stream,
                    std::string const& type)
{
    if (stream && image.width() > 0 && image.height() > 0)
    {
        std::string t = type;
        std::transform(t.begin(), t.end(), t.begin(), ::tolower);
        if (t == "png" || boost::algorithm::starts_with(t, "png"))
        {
            png_saver visitor(stream, t);
            visitor(image);
            //util::apply_visitor(visitor, image);
        }
        else if (boost::algorithm::starts_with(t, "tif"))
        {
            tiff_saver visitor(stream, t);  
            visitor(image);
            //util::apply_visitor(visitor, image);
        }
        else if (boost::algorithm::starts_with(t, "jpeg"))
        {
            jpeg_saver visitor(stream, t);
            visitor(image);
            //util::apply_visitor(visitor, image);
        }
        else if (boost::algorithm::starts_with(t, "webp"))
        {
            webp_saver visitor(stream, t);
            visitor(image);
            //util::apply_visitor(visitor, image);
        }
        else throw ImageWriterException("unknown file type: " + type);
    }
    else throw ImageWriterException("Could not write to empty stream" );
}

template <typename T>
void save_to_file(T const& image, std::string const& filename)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        save_to_file<T>(image, filename, *type);
    }
    else throw ImageWriterException("Could not write file to " + filename );
}

template <typename T>
void save_to_file(T const& image, std::string const& filename, rgba_palette const& palette)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        save_to_file<T>(image, filename, *type, palette);
    }
    else throw ImageWriterException("Could not write file to " + filename );
}

#if defined(HAVE_CAIRO)
// TODO - move to separate cairo_io.hpp
void save_to_cairo_file(mapnik::Map const& map, std::string const& filename, double scale_factor, double scale_denominator)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        save_to_cairo_file(map,filename,*type,scale_factor,scale_denominator);
    }
    else throw ImageWriterException("Could not write file to " + filename );
}

void save_to_cairo_file(mapnik::Map const& map,
                        std::string const& filename,
                        std::string const& type,
                        double scale_factor,
                        double scale_denominator)
{
    std::ofstream file (filename.c_str(), std::ios::out|std::ios::trunc|std::ios::binary);
    if (file)
    {
        cairo_surface_ptr surface;
        unsigned width = map.width();
        unsigned height = map.height();
        if (type == "pdf")
        {
#ifdef CAIRO_HAS_PDF_SURFACE
            surface = cairo_surface_ptr(cairo_pdf_surface_create(filename.c_str(),width,height),cairo_surface_closer());
#else
            throw ImageWriterException("PDFSurface not supported in the cairo backend");
#endif
        }
#ifdef CAIRO_HAS_SVG_SURFACE
        else if (type == "svg")
        {
            surface = cairo_surface_ptr(cairo_svg_surface_create(filename.c_str(),width,height),cairo_surface_closer());
        }
#endif
#ifdef CAIRO_HAS_PS_SURFACE
        else if (type == "ps")
        {
            surface = cairo_surface_ptr(cairo_ps_surface_create(filename.c_str(),width,height),cairo_surface_closer());
        }
#endif
#ifdef CAIRO_HAS_IMAGE_SURFACE
        else if (type == "ARGB32")
        {
            surface = cairo_surface_ptr(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height),cairo_surface_closer());
        }
        else if (type == "RGB24")
        {
            surface = cairo_surface_ptr(cairo_image_surface_create(CAIRO_FORMAT_RGB24,width,height),cairo_surface_closer());
        }
#endif
        else
        {
            throw ImageWriterException("unknown file type: " + type);
        }

        //cairo_t * ctx = cairo_create(surface);

        // TODO - expose as user option
        /*
          if (type == "ARGB32" || type == "RGB24")
          {
          context->set_antialias(Cairo::ANTIALIAS_NONE);
          }
        */

        mapnik::cairo_renderer<cairo_ptr> ren(map, create_context(surface), scale_factor);
        ren.apply(scale_denominator);

        if (type == "ARGB32" || type == "RGB24")
        {
            cairo_surface_write_to_png(&*surface, filename.c_str());
        }
        cairo_surface_finish(&*surface);
    }
}

#endif

template void save_to_file<image_data_rgba8>(image_data_rgba8 const&,
                                          std::string const&,
                                          std::string const&);

template void save_to_file<image_data_rgba8>(image_data_rgba8 const&,
                                          std::string const&,
                                          std::string const&,
                                          rgba_palette const& palette);

template void save_to_file<image_data_rgba8>(image_data_rgba8 const&,
                                          std::string const&);

template void save_to_file<image_data_rgba8>(image_data_rgba8 const&,
                                          std::string const&,
                                          rgba_palette const& palette);

template std::string save_to_string<image_data_rgba8>(image_data_rgba8 const&,
                                                   std::string const&);

template std::string save_to_string<image_data_rgba8>(image_data_rgba8 const&,
                                                   std::string const&,
                                                   rgba_palette const& palette);

template void save_to_file<image_view<image_data_rgba8> > (image_view<image_data_rgba8> const&,
                                                        std::string const&,
                                                        std::string const&);

template void save_to_file<image_view<image_data_rgba8> > (image_view<image_data_rgba8> const&,
                                                        std::string const&,
                                                        std::string const&,
                                                        rgba_palette const& palette);

template void save_to_file<image_view<image_data_rgba8> > (image_view<image_data_rgba8> const&,
                                                        std::string const&);

template void save_to_file<image_view<image_data_rgba8> > (image_view<image_data_rgba8> const&,
                                                        std::string const&,
                                                        rgba_palette const& palette);

template std::string save_to_string<image_view<image_data_rgba8> > (image_view<image_data_rgba8> const&,
                                                                 std::string const&);

template std::string save_to_string<image_view<image_data_rgba8> > (image_view<image_data_rgba8> const&,
                                                                 std::string const&,
                                                                 rgba_palette const& palette);

}
