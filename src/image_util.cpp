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
#include <mapnik/image_util_png.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/graphics.hpp>
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
                           rgba_palette_ptr const& palette)
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
                  rgba_palette_ptr const& palette)
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

#if defined(HAVE_TIFF)
void handle_tiff_options(std::string const& type,
                        tiff_config & config)
{
    if (type == "tiff")
    {
        return;
    }
    if (type.length() > 4)
    {
        boost::char_separator<char> sep(":");
        boost::tokenizer< boost::char_separator<char> > tokens(type, sep);
        for (auto const& t : tokens)
        {
            if (t == "tiff")
            {
                continue;
            }
            else if (boost::algorithm::starts_with(t, "compression="))
            {
                std::string val = t.substr(12);
                if (!val.empty())
                {
                    if (val == "deflate")
                    {
                        config.compression = COMPRESSION_DEFLATE;
                    }
                    else if (val == "adobedeflate")
                    {
                        config.compression = COMPRESSION_ADOBE_DEFLATE;
                    }
                    else if (val == "lzw")
                    {
                        config.compression = COMPRESSION_LZW;
                    }
                    else if (val == "none")
                    {   
                        config.compression = COMPRESSION_NONE;
                    }
                    else
                    {
                        throw ImageWriterException("invalid tiff compression: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "method="))
            {
                std::string val = t.substr(7);
                if (!val.empty())
                {
                    if (val == "scanline")
                    {
                        config.method = TIFF_WRITE_SCANLINE;
                    }
                    else if (val == "strip" || val == "stripped")
                    {
                        config.method = TIFF_WRITE_STRIPPED;
                    }
                    else if (val == "tiled")
                    {
                        config.method = TIFF_WRITE_TILED;
                    }
                    else
                    {
                        throw ImageWriterException("invalid tiff method: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "zlevel="))
            {
                std::string val = t.substr(7);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.zlevel) || config.zlevel < 0 || config.zlevel > 9)
                    {
                        throw ImageWriterException("invalid tiff zlevel: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "tile_height="))
            {
                std::string val = t.substr(12);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.tile_height) || config.tile_height < 0 )
                    {
                        throw ImageWriterException("invalid tiff tile_height: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "tile_width="))
            {
                std::string val = t.substr(11);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.tile_width) || config.tile_width < 0 )
                    {
                        throw ImageWriterException("invalid tiff tile_width: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "rows_per_strip="))
            {
                std::string val = t.substr(15);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.rows_per_strip) || config.rows_per_strip < 0 )
                    {
                        throw ImageWriterException("invalid tiff rows_per_strip: '" + val + "'");
                    }
                }
            }
            else
            {
                throw ImageWriterException("unhandled tiff option: " + t);
            }
        }
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

void save_to_stream(image_data_any const& image,
                    std::ostream & stream,
                    std::string const& type,
                    rgba_palette_ptr const& palette)
{
    if (stream && image.width() > 0 && image.height() > 0)
    {
        std::string t = type;
        std::transform(t.begin(), t.end(), t.begin(), ::tolower);
        if (t == "png" || boost::algorithm::starts_with(t, "png"))
        {   
            png_saver visitor(stream, t, palette);
            mapnik::util::apply_visitor(visitor, image);
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

void save_to_stream(image_data_any const& image,
                    std::ostream & stream,
                    std::string const& type)
{
    if (stream && image.width() > 0 && image.height() > 0)
    {
        std::string t = type;
        std::transform(t.begin(), t.end(), t.begin(), ::tolower);
        if (t == "png" || boost::algorithm::starts_with(t, "png"))
        {
            mapnik::util::apply_visitor(png_saver(stream, t), image);
        }
        else if (boost::algorithm::starts_with(t, "tif"))
        {
#if defined(HAVE_TIFF)
            tiff_config config;
            handle_tiff_options(t, config);
            save_as_tiff(stream, image, config);
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
void save_to_file(T const& image, std::string const& filename, rgba_palette_ptr const& palette)
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
                                          rgba_palette_ptr const& palette);

template void save_to_file<image_data_rgba8>(image_data_rgba8 const&,
                                          std::string const&);

template void save_to_file<image_data_rgba8>(image_data_rgba8 const&,
                                          std::string const&,
                                          rgba_palette_ptr const& palette);

template std::string save_to_string<image_data_rgba8>(image_data_rgba8 const&,
                                                   std::string const&);

template std::string save_to_string<image_data_rgba8>(image_data_rgba8 const&,
                                                   std::string const&,
                                                   rgba_palette_ptr const& palette);

template void save_to_file<image_view<image_data_rgba8> > (image_view<image_data_rgba8> const&,
                                                        std::string const&,
                                                        std::string const&);

template void save_to_file<image_view<image_data_rgba8> > (image_view<image_data_rgba8> const&,
                                                        std::string const&,
                                                        std::string const&,
                                                        rgba_palette_ptr const& palette);

template void save_to_file<image_view<image_data_rgba8> > (image_view<image_data_rgba8> const&,
                                                        std::string const&);

template void save_to_file<image_view<image_data_rgba8> > (image_view<image_data_rgba8> const&,
                                                        std::string const&,
                                                        rgba_palette_ptr const& palette);

template std::string save_to_string<image_view<image_data_rgba8> > (image_view<image_data_rgba8> const&,
                                                                 std::string const&);

template std::string save_to_string<image_view<image_data_rgba8> > (image_view<image_data_rgba8> const&,
                                                                 std::string const&,
                                                                 rgba_palette_ptr const& palette);

void save_to_file(image_32 const& image,std::string const& file)
{
    save_to_file<image_data_rgba8>(image.data(), file);
}

void save_to_file (image_32 const& image,
                   std::string const& file,
                   std::string const& type)
{
    save_to_file<image_data_rgba8>(image.data(), file, type);
}

void save_to_file (image_32 const& image,
                   std::string const& file,
                   std::string const& type,
                   rgba_palette_ptr const& palette)
{
    save_to_file<image_data_rgba8>(image.data(), file, type, palette);
}

std::string save_to_string(image_32 const& image,
                           std::string const& type)
{
    return save_to_string<image_data_rgba8>(image.data(), type);
}

std::string save_to_string(image_32 const& image,
                           std::string const& type,
                           rgba_palette_ptr const& palette)
{
    return save_to_string<image_data_rgba8>(image.data(), type, palette);
}

}
