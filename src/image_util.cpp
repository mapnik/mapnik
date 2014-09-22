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

#if defined(HAVE_PNG)
extern "C"
{
#include <png.h>
}
#endif

// mapnik
#if defined(HAVE_PNG)
#include <mapnik/png_io.hpp>
#endif

#if defined(HAVE_TIFF)
#include <mapnik/tiff_io.hpp>
#endif

#if defined(HAVE_JPEG)
#include <mapnik/jpeg_io.hpp>
#endif

#if defined(HAVE_WEBP)
#include <mapnik/webp_io.hpp>
#endif

#include <mapnik/image_util.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/memory.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/palette.hpp>
#include <mapnik/map.hpp>
#include <mapnik/util/conversions.hpp>

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

#if defined(HAVE_PNG)

void handle_png_options(std::string const& type,
                        png_options & opts)
{
    if (type == "png" || type == "png24" || type == "png32")
    {
        opts.paletted = false;
        return;
    }
    else if (type == "png8" || type == "png256")
    {
        opts.paletted = true;
        return;
    }
    boost::char_separator<char> sep(":");
    boost::tokenizer< boost::char_separator<char> > tokens(type, sep);
    bool set_colors = false;
    bool set_gamma = false;
    for (std::string const& t : tokens)
    {
        if (t == "png8" || t == "png256")
        {
            opts.paletted = true;
        }
        else if (t == "png" || t == "png24" || t == "png32")
        {
            opts.paletted = false;
        }
        else if (t == "m=o")
        {
            opts.use_hextree = false;
        }
        else if (t == "m=h")
        {
            opts.use_hextree = true;
        }
        else if (t == "e=miniz")
        {
            opts.use_miniz = true;
        }
        else if (boost::algorithm::starts_with(t, "c="))
        {
            set_colors = true;
            if (!mapnik::util::string2int(t.substr(2),opts.colors) || opts.colors < 1 || opts.colors > 256)
            {
                throw ImageWriterException("invalid color parameter: " + t.substr(2));
            }
        }
        else if (boost::algorithm::starts_with(t, "t="))
        {
            if (!mapnik::util::string2int(t.substr(2),opts.trans_mode) || opts.trans_mode < 0 || opts.trans_mode > 2)
            {
                throw ImageWriterException("invalid trans_mode parameter: " + t.substr(2));
            }
        }
        else if (boost::algorithm::starts_with(t, "g="))
        {
            set_gamma = true;
            if (!mapnik::util::string2double(t.substr(2),opts.gamma) || opts.gamma < 0)
            {
                throw ImageWriterException("invalid gamma parameter: " + t.substr(2));
            }
        }
        else if (boost::algorithm::starts_with(t, "z="))
        {
            /*
              #define Z_NO_COMPRESSION         0
              #define Z_BEST_SPEED             1
              #define Z_BEST_COMPRESSION       9
              #define Z_DEFAULT_COMPRESSION  (-1)
            */
            if (!mapnik::util::string2int(t.substr(2),opts.compression)
                || opts.compression < Z_DEFAULT_COMPRESSION
                || opts.compression > 10) // use 10 here rather than Z_BEST_COMPRESSION (9) to allow for MZ_UBER_COMPRESSION
            {
                throw ImageWriterException("invalid compression parameter: " + t.substr(2) + " (only -1 through 10 are valid)");
            }
        }
        else if (boost::algorithm::starts_with(t, "s="))
        {
            std::string s = t.substr(2);
            if (s == "default")
            {
                opts.strategy = Z_DEFAULT_STRATEGY;
            }
            else if (s == "filtered")
            {
                opts.strategy = Z_FILTERED;
            }
            else if (s == "huff")
            {
                opts.strategy = Z_HUFFMAN_ONLY;
            }
            else if (s == "rle")
            {
                opts.strategy = Z_RLE;
            }
            else if (s == "fixed")
            {
                opts.strategy = Z_FIXED;
            }
            else
            {
                throw ImageWriterException("invalid compression strategy parameter: " + s);
            }
        }
        else
        {
            throw ImageWriterException("unhandled png option: " + t);
        }
    }
    // validation
    if (!opts.paletted && set_colors)
    {
        throw ImageWriterException("invalid color parameter: unavailable for true color (non-paletted) images");
    }
    if (!opts.paletted && set_gamma)
    {
        throw ImageWriterException("invalid gamma parameter: unavailable for true color (non-paletted) images");
    }
    if ((opts.use_miniz == false) && opts.compression > Z_BEST_COMPRESSION)
    {
        throw ImageWriterException("invalid compression value: (only -1 through 9 are valid)");
    }
}
#endif

#if defined(HAVE_WEBP)
void handle_webp_options(std::string const& type,
                        WebPConfig & config,
                        bool & alpha)
{
    if (type == "webp")
    {
        return;
    }
    if (type.length() > 4){
        boost::char_separator<char> sep(":");
        boost::tokenizer< boost::char_separator<char> > tokens(type, sep);
        for (auto const& t : tokens)
        {
            if (t == "webp")
            {
                continue;
            }
            else if (boost::algorithm::starts_with(t, "quality="))
            {
                std::string val = t.substr(8);
                if (!val.empty())
                {
                    double quality = 90;
                    if (!mapnik::util::string2double(val,quality) || quality < 0.0 || quality > 100.0)
                    {
                        throw ImageWriterException("invalid webp quality: '" + val + "'");
                    }
                    config.quality = static_cast<float>(quality);
                }
            }
            else if (boost::algorithm::starts_with(t, "method="))
            {
                std::string val = t.substr(7);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.method) || config.method < 0 || config.method > 6)
                    {
                        throw ImageWriterException("invalid webp method: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "lossless="))
            {
                std::string val = t.substr(9);
                if (!val.empty())
                {
                    #if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1 // >= v0.1.99 / 0x0100
                    if (!mapnik::util::string2int(val,config.lossless) || config.lossless < 0 || config.lossless > 1)
                    {
                        throw ImageWriterException("invalid webp lossless: '" + val + "'");
                    }
                    #else
                        #ifdef _MSC_VER
                          #pragma NOTE(compiling against webp that does not support the lossless flag)
                        #else
                          #warning "compiling against webp that does not support the lossless flag"
                        #endif
                    throw ImageWriterException("your webp version does not support the lossless option");
                    #endif
                }
            }
            else if (boost::algorithm::starts_with(t, "image_hint="))
            {
                std::string val = t.substr(11);
                if (!val.empty())
                {
                    #if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1 // >= v0.1.99 / 0x0100
                    int image_hint = 0;
                    if (!mapnik::util::string2int(val,image_hint) || image_hint < 0 || image_hint > 3)
                    {
                        throw ImageWriterException("invalid webp image_hint: '" + val + "'");
                    }
                    config.image_hint = static_cast<WebPImageHint>(image_hint);
                    #else
                        #ifdef _MSC_VER
                          #pragma NOTE(compiling against webp that does not support the image_hint flag)
                        #else
                          #warning "compiling against webp that does not support the image_hint flag"
                        #endif
                    throw ImageWriterException("your webp version does not support the image_hint option");
                    #endif
                }
            }
            else if (boost::algorithm::starts_with(t, "alpha="))
            {
                std::string val = t.substr(6);
                if (!val.empty())
                {
                    if (!mapnik::util::string2bool(val,alpha))
                    {
                        throw ImageWriterException("invalid webp alpha: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "target_size="))
            {
                std::string val = t.substr(12);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.target_size))
                    {
                        throw ImageWriterException("invalid webp target_size: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "target_psnr="))
            {
                std::string val = t.substr(12);
                if (!val.empty())
                {
                    double psnr = 0;
                    if (!mapnik::util::string2double(val,psnr))
                    {
                        throw ImageWriterException("invalid webp target_psnr: '" + val + "'");
                    }
                    config.target_PSNR = psnr;
                }
            }
            else if (boost::algorithm::starts_with(t, "segments="))
            {
                std::string val = t.substr(9);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.segments))
                    {
                        throw ImageWriterException("invalid webp segments: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "sns_strength="))
            {
                std::string val = t.substr(13);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.sns_strength))
                    {
                        throw ImageWriterException("invalid webp sns_strength: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "filter_strength="))
            {
                std::string val = t.substr(16);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.filter_strength))
                    {
                        throw ImageWriterException("invalid webp filter_strength: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "filter_sharpness="))
            {
                std::string val = t.substr(17);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.filter_sharpness))
                    {
                        throw ImageWriterException("invalid webp filter_sharpness: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "filter_type="))
            {
                std::string val = t.substr(12);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.filter_type))
                    {
                        throw ImageWriterException("invalid webp filter_type: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "autofilter="))
            {
                std::string val = t.substr(11);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.autofilter))
                    {
                        throw ImageWriterException("invalid webp autofilter: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "alpha_compression="))
            {
                std::string val = t.substr(18);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.alpha_compression))
                    {
                        throw ImageWriterException("invalid webp alpha_compression: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "alpha_filtering="))
            {
                std::string val = t.substr(16);
                if (!val.empty())
                {
                    #if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1 // >= v0.1.99 / 0x0100
                    if (!mapnik::util::string2int(val,config.alpha_filtering))
                    {
                        throw ImageWriterException("invalid webp alpha_filtering: '" + val + "'");
                    }
                    #else
                        #ifdef _MSC_VER
                          #pragma NOTE(compiling against webp that does not support the alpha_filtering flag)
                        #else
                          #warning "compiling against webp that does not support the alpha_filtering flag"
                        #endif
                    throw ImageWriterException("your webp version does not support the alpha_filtering option");
                    #endif
                }
            }
            else if (boost::algorithm::starts_with(t, "alpha_quality="))
            {
                std::string val = t.substr(14);
                if (!val.empty())
                {
                    #if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1 // >= v0.1.99 / 0x0100
                    if (!mapnik::util::string2int(val,config.alpha_quality))
                    {
                        throw ImageWriterException("invalid webp alpha_quality: '" + val + "'");
                    }
                    #else
                        #ifdef _MSC_VER
                          #pragma NOTE(compiling against webp that does not support the alpha_quality flag)
                        #else
                          #warning "compiling against webp that does not support the alpha_quality flag"
                        #endif
                    throw ImageWriterException("your webp version does not support the alpha_quality option");
                    #endif
                }
            }
            else if (boost::algorithm::starts_with(t, "pass="))
            {
                std::string val = t.substr(5);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.pass))
                    {
                        throw ImageWriterException("invalid webp pass: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "preprocessing="))
            {
                std::string val = t.substr(14);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.preprocessing))
                    {
                        throw ImageWriterException("invalid webp preprocessing: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "partitions="))
            {
                std::string val = t.substr(11);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.partitions))
                    {
                        throw ImageWriterException("invalid webp partitions: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "partition_limit="))
            {
                std::string val = t.substr(16);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.partition_limit))
                    {
                        throw ImageWriterException("invalid webp partition_limit: '" + val + "'");
                    }
                }
            }
            else
            {
                throw ImageWriterException("unhandled webp option: " + t);
            }
        }
    }
}
#endif

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
#if defined(HAVE_PNG)
            if (palette.valid())
            {
                png_options opts;
                handle_png_options(t,opts);
                save_as_png8_pal(stream, image, palette, opts);
            }
            else
            {
                save_to_stream(image,stream,type);
            }
#else
            throw ImageWriterException("png output is not enabled in your build of Mapnik");
#endif
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
#if defined(HAVE_PNG)
            png_options opts;
            handle_png_options(t,opts);
            if (opts.paletted)
            {
                if (opts.use_hextree)
                {
                    save_as_png8_hex(stream, image, opts);
                }
                else
                {
                    save_as_png8_oct(stream, image, opts);
                }
            }
            else
            {
                save_as_png(stream, image, opts);
            }
#else
            throw ImageWriterException("png output is not enabled in your build of Mapnik");
#endif
        }
        else if (boost::algorithm::starts_with(t, "tif"))
        {
#if defined(HAVE_TIFF)
            save_as_tiff(stream, image);
#else
            throw ImageWriterException("tiff output is not enabled in your build of Mapnik");
#endif
        }
        else if (boost::algorithm::starts_with(t, "jpeg"))
        {
#if defined(HAVE_JPEG)
            int quality = 85;
            std::string val = t.substr(4);
            if (!val.empty())
            {
                if (!mapnik::util::string2int(val,quality) || quality < 0 || quality > 100)
                {
                    throw ImageWriterException("invalid jpeg quality: '" + val + "'");
                }
            }
            save_as_jpeg(stream, quality, image);
#else
            throw ImageWriterException("jpeg output is not enabled in your build of Mapnik");
#endif
        }
        else if (boost::algorithm::starts_with(t, "webp"))
        {
#if defined(HAVE_WEBP)
            WebPConfig config;
            // Default values set here will be lossless=0 and quality=75 (as least as of webp v0.3.1)
            if (!WebPConfigInit(&config))
            {
                throw std::runtime_error("version mismatch");
            }
            // see for more details: https://github.com/mapnik/mapnik/wiki/Image-IO#webp-output-options
            bool alpha = true;
            handle_webp_options(t,config,alpha);
            save_as_webp(stream,image,config,alpha);
#else
            throw ImageWriterException("webp output is not enabled in your build of Mapnik");
#endif
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

template void save_to_file<image_data_32>(image_data_32 const&,
                                          std::string const&,
                                          std::string const&);

template void save_to_file<image_data_32>(image_data_32 const&,
                                          std::string const&,
                                          std::string const&,
                                          rgba_palette const& palette);

template void save_to_file<image_data_32>(image_data_32 const&,
                                          std::string const&);

template void save_to_file<image_data_32>(image_data_32 const&,
                                          std::string const&,
                                          rgba_palette const& palette);

template std::string save_to_string<image_data_32>(image_data_32 const&,
                                                   std::string const&);

template std::string save_to_string<image_data_32>(image_data_32 const&,
                                                   std::string const&,
                                                   rgba_palette const& palette);

template void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                        std::string const&,
                                                        std::string const&);

template void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                        std::string const&,
                                                        std::string const&,
                                                        rgba_palette const& palette);

template void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                        std::string const&);

template void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                        std::string const&,
                                                        rgba_palette const& palette);

template std::string save_to_string<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                 std::string const&);

template std::string save_to_string<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                 std::string const&,
                                                                 rgba_palette const& palette);

void save_to_file(image_32 const& image,std::string const& file)
{
    save_to_file<image_data_32>(image.data(), file);
}

void save_to_file (image_32 const& image,
                   std::string const& file,
                   std::string const& type)
{
    save_to_file<image_data_32>(image.data(), file, type);
}

void save_to_file (image_32 const& image,
                   std::string const& file,
                   std::string const& type,
                   rgba_palette const& palette)
{
    save_to_file<image_data_32>(image.data(), file, type, palette);
}

std::string save_to_string(image_32 const& image,
                           std::string const& type)
{
    return save_to_string<image_data_32>(image.data(), type);
}

std::string save_to_string(image_32 const& image,
                           std::string const& type,
                           rgba_palette const& palette)
{
    return save_to_string<image_data_32>(image.data(), type, palette);
}

}
