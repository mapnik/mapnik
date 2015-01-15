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
#include <mapnik/image_data_any.hpp>
#include <mapnik/image_view_any.hpp>
#include <mapnik/memory.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/palette.hpp>
#include <mapnik/map.hpp>
#include <mapnik/color.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/variant.hpp>

// boost
#include <boost/tokenizer.hpp>

// agg
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"

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
        save_to_stream<T>(image, file, type, palette);
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
        save_to_stream<T>(image, file, type);
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

// This can be removed once image_data_any and image_view_any are the only 
// items using this template
template <>
void save_to_stream<image_data_rgba8>(image_data_rgba8 const& image,
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

// This can be removed once image_data_any and image_view_any are the only 
// items using this template
template <>
void save_to_stream<image_view_rgba8>(image_view_rgba8 const& image,
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
            util::apply_visitor(visitor, image);
        }
        else if (boost::algorithm::starts_with(t, "tif"))
        {
            tiff_saver visitor(stream, t);  
            util::apply_visitor(visitor, image);
        }
        else if (boost::algorithm::starts_with(t, "jpeg"))
        {
            jpeg_saver visitor(stream, t);
            util::apply_visitor(visitor, image);
        }
        else if (boost::algorithm::starts_with(t, "webp"))
        {
            webp_saver visitor(stream, t);
            util::apply_visitor(visitor, image);
        }
        else throw ImageWriterException("unknown file type: " + type);
    }
    else throw ImageWriterException("Could not write to empty stream" );
}

// This can be removed once image_data_any and image_view_any are the only 
// items using this template
template <>
void save_to_stream<image_data_rgba8>(image_data_rgba8 const& image,
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

// This can be removed once image_data_any and image_view_any are the only 
// items using this template
template <>
void save_to_stream<image_view_rgba8>(image_view_rgba8 const& image,
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

// image_data_rgba8
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

// image_view_rgba8
template void save_to_file<image_view_rgba8> (image_view_rgba8 const&,
                                              std::string const&,
                                              std::string const&);

template void save_to_file<image_view_rgba8> (image_view_rgba8 const&,
                                              std::string const&,
                                              std::string const&,
                                              rgba_palette const& palette);

template void save_to_file<image_view_rgba8> (image_view_rgba8 const&,
                                              std::string const&);

template void save_to_file<image_view_rgba8> (image_view_rgba8 const&,
                                              std::string const&,
                                              rgba_palette const& palette);

template std::string save_to_string<image_view_rgba8> (image_view_rgba8 const&,
                                                       std::string const&);

template std::string save_to_string<image_view_rgba8> (image_view_rgba8 const&,
                                                       std::string const&,
                                                       rgba_palette const& palette);

// image_data_any
template void save_to_file<image_data_any>(image_data_any const&,
                                           std::string const&,
                                           std::string const&);

template void save_to_file<image_data_any>(image_data_any const&,
                                           std::string const&,
                                           std::string const&,
                                           rgba_palette const& palette);

template void save_to_file<image_data_any>(image_data_any const&,
                                           std::string const&);

template void save_to_file<image_data_any>(image_data_any const&,
                                           std::string const&,
                                           rgba_palette const& palette);

template std::string save_to_string<image_data_any>(image_data_any const&,
                                                    std::string const&);

template std::string save_to_string<image_data_any>(image_data_any const&,
                                                    std::string const&,
                                                    rgba_palette const& palette);

namespace detail {

struct is_solid_visitor
{
    template <typename T>
    bool operator() (T const & data)
    {
        using pixel_type = typename T::pixel_type;
        if (data.width() > 0 && data.height() > 0)
        {
            pixel_type const* first_row = data.getRow(0);
            pixel_type const first_pixel = first_row[0];
            for (unsigned y = 0; y < data.height(); ++y)
            {
                pixel_type const * row = data.getRow(y);
                for (unsigned x = 0; x < data.width(); ++x)
                {
                    if (first_pixel != row[x])
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }
};

template bool is_solid_visitor::operator()<image_data_rgba8> (image_data_rgba8 const& data);
template bool is_solid_visitor::operator()<image_data_gray8> (image_data_gray8 const& data);
template bool is_solid_visitor::operator()<image_data_gray16> (image_data_gray16 const& data);
template bool is_solid_visitor::operator()<image_data_gray32f> (image_data_gray32f const& data);
template bool is_solid_visitor::operator()<image_view_rgba8> (image_view_rgba8 const& data);
template bool is_solid_visitor::operator()<image_view_gray8> (image_view_gray8 const& data);
template bool is_solid_visitor::operator()<image_view_gray16> (image_view_gray16 const& data);
template bool is_solid_visitor::operator()<image_view_gray32f> (image_view_gray32f const& data);

template<>
bool is_solid_visitor::operator()<image_data_null> (image_data_null const&)
{
    return true;
}

} // end detail ns

template <typename T>
MAPNIK_DECL bool is_solid(T const& image)
{
    return util::apply_visitor(detail::is_solid_visitor(), image);
}

template bool is_solid<image_data_any> (image_data_any const&);
template bool is_solid<image_view_any> (image_view_any const&);

// Temporary until image_data_rgba8 is removed from passing
template <>
MAPNIK_DECL bool is_solid<image_data_rgba8>(image_data_rgba8 const& image)
{
    detail::is_solid_visitor visitor;
    return visitor(image);
}

// Temporary until image_view_rgba8 is removed from passing
template <>
MAPNIK_DECL bool is_solid<image_view_rgba8>(image_view_rgba8 const& image)
{
    detail::is_solid_visitor visitor;
    return visitor(image);
}

namespace detail {

struct premultiply_visitor
{
    template <typename T>
    bool operator() (T & data) 
    {
        throw std::runtime_error("Error: Premultiply with " + std::string(typeid(data).name()) + " is not supported");
    }

};

template <>
bool premultiply_visitor::operator()<image_data_rgba8> (image_data_rgba8 & data)
{
    if (!data.get_premultiplied())
    {
        agg::rendering_buffer buffer(data.getBytes(),data.width(),data.height(),data.width() * 4);
        agg::pixfmt_rgba32 pixf(buffer);
        pixf.premultiply();
        data.set_premultiplied(true);
        return true;
    }
    return false;
}

struct demultiply_visitor
{
    template <typename T>
    bool operator() (T & data) 
    {
        throw std::runtime_error("Error: Premultiply with " + std::string(typeid(data).name()) + " is not supported");
    }

};

template <>
bool demultiply_visitor::operator()<image_data_rgba8> (image_data_rgba8 & data)
{
    if (data.get_premultiplied())
    {
        agg::rendering_buffer buffer(data.getBytes(),data.width(),data.height(),data.width() * 4);
        agg::pixfmt_rgba32_pre pixf(buffer);
        pixf.demultiply();
        data.set_premultiplied(false);
        return true;
    }
    return false;
}

struct set_premultiplied_visitor
{
    set_premultiplied_visitor(bool status)
        : status_(status) {}

    template <typename T>
    void operator() (T & data) 
    {
        data.set_premultiplied(status_);
    }
  private:
    bool status_;
};

} // end detail ns

template <typename T>
MAPNIK_DECL bool premultiply_alpha(T & image)
{
    return util::apply_visitor(detail::premultiply_visitor(), image);
}

template bool premultiply_alpha<image_data_any> (image_data_any &);

// Temporary, can be removed once image_view_any and image_data_any are the only ones passed
template <>
MAPNIK_DECL bool premultiply_alpha<image_data_rgba8>(image_data_rgba8 & image)
{
    detail::premultiply_visitor visit;
    return visit(image);
}

template <typename T>
MAPNIK_DECL bool demultiply_alpha(T & image)
{
    return util::apply_visitor(detail::demultiply_visitor(), image);
}

template bool demultiply_alpha<image_data_any> (image_data_any &);

// Temporary, can be removed once image_view_any and image_data_any are the only ones passed
template <>
MAPNIK_DECL bool demultiply_alpha<image_data_rgba8>(image_data_rgba8 & image)
{
    detail::demultiply_visitor visit;
    return visit(image);
}

template <typename T>
MAPNIK_DECL void set_premultiplied_alpha(T & image, bool status)
{
    util::apply_visitor(detail::set_premultiplied_visitor(status), image);
}

template void set_premultiplied_alpha<image_data_any> (image_data_any &, bool);

// Temporary, can be removed once image_view_any and image_data_any are the only ones passed
template <>
MAPNIK_DECL void set_premultiplied_alpha<image_data_rgba8>(image_data_rgba8 & image, bool status)
{
    detail::set_premultiplied_visitor visit(status);
    visit(image);
}

namespace detail {

struct visitor_set_alpha
{
    visitor_set_alpha(float opacity)
        : opacity_(opacity) {}

    template <typename T>
    void operator() (T & data)
    {
        throw std::runtime_error("Error: set_alpha with " + std::string(typeid(data).name()) + " is not supported");
    }

  private:
    float opacity_;

};

template <>
void visitor_set_alpha::operator()<image_data_rgba8> (image_data_rgba8 & data)
{
    using pixel_type = typename image_data_rgba8::pixel_type;
    for (unsigned int y = 0; y < data.height(); ++y)
    {
        pixel_type* row_to =  data.getRow(y);
        for (unsigned int x = 0; x < data.width(); ++x)
        {
            pixel_type rgba = row_to[x];
            pixel_type a0 = (rgba >> 24) & 0xff;
            pixel_type a1 = pixel_type( ((rgba >> 24) & 0xff) * opacity_ );
            //unsigned a1 = opacity;
            if (a0 == a1) continue;

            pixel_type r = rgba & 0xff;
            pixel_type g = (rgba >> 8 ) & 0xff;
            pixel_type b = (rgba >> 16) & 0xff;

            row_to[x] = (a1 << 24)| (b << 16) |  (g << 8) | (r) ;
        }
    }
}

template <>
void visitor_set_alpha::operator()<image_data_null> (image_data_null &)
{
    throw std::runtime_error("Can not set alpha for null image");
}

} // end detail ns

template<>
MAPNIK_DECL void set_alpha<image_data_any> (image_data_any & data, float opacity)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    util::apply_visitor(detail::visitor_set_alpha(opacity), data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

// TEMPORARY can be removed once image_data_any is only way it is being passed.
template<>
MAPNIK_DECL void set_alpha<image_data_rgba8> (image_data_rgba8 & data, float opacity)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    detail::visitor_set_alpha visit(opacity);
    visit(data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

} // end ns
