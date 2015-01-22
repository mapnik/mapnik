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
#include <mapnik/image.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/image_view_any.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/palette.hpp>
#include <mapnik/color.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/util/variant.hpp>

// agg
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"

// stl
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace mapnik
{

template <typename T>
MAPNIK_DECL std::string save_to_string(T const& image,
                           std::string const& type,
                           rgba_palette const& palette)
{
    std::ostringstream ss(std::ios::out|std::ios::binary);
    save_to_stream(image, ss, type, palette);
    return ss.str();
}

template <typename T>
MAPNIK_DECL std::string save_to_string(T const& image,
                           std::string const& type)
{
    std::ostringstream ss(std::ios::out|std::ios::binary);
    save_to_stream(image, ss, type);
    return ss.str();
}

template <typename T>
MAPNIK_DECL void save_to_file(T const& image,
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
MAPNIK_DECL void save_to_file(T const& image,
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
MAPNIK_DECL void save_to_stream(T const& image,
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

// This can be removed once image_any and image_view_any are the only 
// items using this template
template <>
MAPNIK_DECL void save_to_stream<image_rgba8>(image_rgba8 const& image,
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

// This can be removed once image_any and image_view_any are the only 
// items using this template
template <>
MAPNIK_DECL void save_to_stream<image_view_rgba8>(image_view_rgba8 const& image,
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
MAPNIK_DECL void save_to_stream(T const& image,
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

// This can be removed once image_any and image_view_any are the only 
// items using this template
template <>
MAPNIK_DECL void save_to_stream<image_rgba8>(image_rgba8 const& image,
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

// This can be removed once image_any and image_view_any are the only 
// items using this template
template <>
MAPNIK_DECL void save_to_stream<image_view_rgba8>(image_view_rgba8 const& image,
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
MAPNIK_DECL void save_to_file(T const& image, std::string const& filename)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        save_to_file<T>(image, filename, *type);
    }
    else throw ImageWriterException("Could not write file to " + filename );
}

template <typename T>
MAPNIK_DECL void save_to_file(T const& image, std::string const& filename, rgba_palette const& palette)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        save_to_file<T>(image, filename, *type, palette);
    }
    else throw ImageWriterException("Could not write file to " + filename );
}

// image_rgba8
template MAPNIK_DECL void save_to_file<image_rgba8>(image_rgba8 const&,
                                             std::string const&,
                                             std::string const&);

template MAPNIK_DECL void save_to_file<image_rgba8>(image_rgba8 const&,
                                             std::string const&,
                                             std::string const&,
                                             rgba_palette const& palette);

template MAPNIK_DECL void save_to_file<image_rgba8>(image_rgba8 const&,
                                             std::string const&);

template MAPNIK_DECL void save_to_file<image_rgba8>(image_rgba8 const&,
                                             std::string const&,
                                             rgba_palette const& palette);

template MAPNIK_DECL std::string save_to_string<image_rgba8>(image_rgba8 const&,
                                                      std::string const&);

template MAPNIK_DECL std::string save_to_string<image_rgba8>(image_rgba8 const&,
                                                      std::string const&,
                                                      rgba_palette const& palette);

// image_view_any
template MAPNIK_DECL void save_to_file<image_view_any> (image_view_any const&,
                                              std::string const&,
                                              std::string const&);

template MAPNIK_DECL void save_to_file<image_view_any> (image_view_any const&,
                                              std::string const&,
                                              std::string const&,
                                              rgba_palette const& palette);

template MAPNIK_DECL void save_to_file<image_view_any> (image_view_any const&,
                                              std::string const&);

template MAPNIK_DECL void save_to_file<image_view_any> (image_view_any const&,
                                              std::string const&,
                                              rgba_palette const& palette);

template MAPNIK_DECL std::string save_to_string<image_view_any> (image_view_any const&,
                                                       std::string const&);

template MAPNIK_DECL std::string save_to_string<image_view_any> (image_view_any const&,
                                                       std::string const&,
                                                       rgba_palette const& palette);

// image_any
template MAPNIK_DECL void save_to_file<image_any>(image_any const&,
                                           std::string const&,
                                           std::string const&);

template MAPNIK_DECL void save_to_file<image_any>(image_any const&,
                                           std::string const&,
                                           std::string const&,
                                           rgba_palette const& palette);

template MAPNIK_DECL void save_to_file<image_any>(image_any const&,
                                           std::string const&);

template MAPNIK_DECL void save_to_file<image_any>(image_any const&,
                                           std::string const&,
                                           rgba_palette const& palette);

template MAPNIK_DECL std::string save_to_string<image_any>(image_any const&,
                                                    std::string const&);

template MAPNIK_DECL std::string save_to_string<image_any>(image_any const&,
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

template bool is_solid_visitor::operator()<image_rgba8> (image_rgba8 const& data);
template bool is_solid_visitor::operator()<image_gray8> (image_gray8 const& data);
template bool is_solid_visitor::operator()<image_gray16> (image_gray16 const& data);
template bool is_solid_visitor::operator()<image_gray32f> (image_gray32f const& data);
template bool is_solid_visitor::operator()<image_view_rgba8> (image_view_rgba8 const& data);
template bool is_solid_visitor::operator()<image_view_gray8> (image_view_gray8 const& data);
template bool is_solid_visitor::operator()<image_view_gray16> (image_view_gray16 const& data);
template bool is_solid_visitor::operator()<image_view_gray32f> (image_view_gray32f const& data);

template<>
bool is_solid_visitor::operator()<image_null> (image_null const&)
{
    return true;
}

} // end detail ns

template <typename T>
MAPNIK_DECL bool is_solid(T const& image)
{
    return util::apply_visitor(detail::is_solid_visitor(), image);
}

template MAPNIK_DECL bool is_solid<image_any> (image_any const&);
template MAPNIK_DECL bool is_solid<image_view_any> (image_view_any const&);

// Temporary until image_rgba8 is removed from passing
template <>
MAPNIK_DECL bool is_solid<image_rgba8>(image_rgba8 const& image)
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
bool premultiply_visitor::operator()<image_rgba8> (image_rgba8 & data)
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
bool demultiply_visitor::operator()<image_rgba8> (image_rgba8 & data)
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

template MAPNIK_DECL bool premultiply_alpha<image_any> (image_any &);

// Temporary, can be removed once image_view_any and image_any are the only ones passed
template <>
MAPNIK_DECL bool premultiply_alpha<image_rgba8>(image_rgba8 & image)
{
    detail::premultiply_visitor visit;
    return visit(image);
}

template <typename T>
MAPNIK_DECL bool demultiply_alpha(T & image)
{
    return util::apply_visitor(detail::demultiply_visitor(), image);
}

template MAPNIK_DECL bool demultiply_alpha<image_any> (image_any &);

// Temporary, can be removed once image_view_any and image_any are the only ones passed
template <>
MAPNIK_DECL bool demultiply_alpha<image_rgba8>(image_rgba8 & image)
{
    detail::demultiply_visitor visit;
    return visit(image);
}

template <typename T>
MAPNIK_DECL void set_premultiplied_alpha(T & image, bool status)
{
    util::apply_visitor(detail::set_premultiplied_visitor(status), image);
}

template void set_premultiplied_alpha<image_any> (image_any &, bool);

// Temporary, can be removed once image_view_any and image_any are the only ones passed
template <>
MAPNIK_DECL void set_premultiplied_alpha<image_rgba8>(image_rgba8 & image, bool status)
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
void visitor_set_alpha::operator()<image_rgba8> (image_rgba8 & data)
{
    using pixel_type = typename image_rgba8::pixel_type;
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

} // end detail ns

template<>
MAPNIK_DECL void set_alpha<image_any> (image_any & data, float opacity)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    util::apply_visitor(detail::visitor_set_alpha(opacity), data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

// TEMPORARY can be removed once image_any is only way it is being passed.
template<>
MAPNIK_DECL void set_alpha<image_rgba8> (image_rgba8 & data, float opacity)
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

namespace detail {

struct visitor_set_grayscale_to_alpha
{
    template <typename T>
    void operator() (T & data)
    {
        throw std::runtime_error("Error: set_grayscale_to_alpha with " + std::string(typeid(data).name()) + " is not supported");
    }
};

template <>
void visitor_set_grayscale_to_alpha::operator()<image_rgba8> (image_rgba8 & data)
{
    using pixel_type = typename image_rgba8::pixel_type;
    for (unsigned int y = 0; y < data.height(); ++y)
    {
        pixel_type* row_from = data.getRow(y);
        for (unsigned int x = 0; x < data.width(); ++x)
        {
            pixel_type rgba = row_from[x];
            pixel_type r = rgba & 0xff;
            pixel_type g = (rgba >> 8 ) & 0xff;
            pixel_type b = (rgba >> 16) & 0xff;

            // magic numbers for grayscale
            pixel_type a = static_cast<pixel_type>(std::ceil((r * .3) + (g * .59) + (b * .11)));

            row_from[x] = (a << 24)| (255 << 16) |  (255 << 8) | (255) ;
        }
    }
}

} // end detail ns

template<>
MAPNIK_DECL void set_grayscale_to_alpha<image_any> (image_any & data)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    util::apply_visitor(detail::visitor_set_grayscale_to_alpha(), data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

// TEMPORARY can be removed once image_any is only way it is being passed.
template<>
MAPNIK_DECL void set_grayscale_to_alpha<image_rgba8> (image_rgba8 & data)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    detail::visitor_set_grayscale_to_alpha visit;
    visit(data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

namespace detail {

struct visitor_set_color_to_alpha
{
    visitor_set_color_to_alpha(color const& c)
        : c_(c) {}

    template <typename T>
    void operator() (T & data)
    {
        throw std::runtime_error("Error: set_color_to_alpha with " + std::string(typeid(data).name()) + " is not supported");
    }

  private:
    color const& c_;

};

template <>
void visitor_set_color_to_alpha::operator()<image_rgba8> (image_rgba8 & data)
{
    using pixel_type = typename image_rgba8::pixel_type;
    for (unsigned y = 0; y < data.height(); ++y)
    {
        pixel_type* row_from = data.getRow(y);
        for (unsigned x = 0; x < data.width(); ++x)
        {
            pixel_type rgba = row_from[x];
            pixel_type r = rgba & 0xff;
            pixel_type g = (rgba >> 8 ) & 0xff;
            pixel_type b = (rgba >> 16) & 0xff;
            if (r == c_.red() && g == c_.green() && b == c_.blue())
            {
                row_from[x] = 0;
            }
        }
    }
}

} // end detail ns

template<>
MAPNIK_DECL void set_color_to_alpha<image_any> (image_any & data, color const& c)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    util::apply_visitor(detail::visitor_set_color_to_alpha(c), data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

// TEMPORARY can be removed once image_any is only way it is being passed.
template<>
MAPNIK_DECL void set_color_to_alpha<image_rgba8> (image_rgba8 & data, color const& c)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    detail::visitor_set_color_to_alpha visit(c);
    visit(data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

namespace detail {

template <typename T1>
struct visitor_fill 
{
    visitor_fill(T1 const& val)
        : val_(val) {}

    template <typename T2>
    void operator() (T2 & data)
    {
        using pixel_type = typename T2::pixel_type;
        pixel_type val = static_cast<pixel_type>(val_);
        data.set(val);
    }

  private:
    T1 const& val_;
};

template<>
struct visitor_fill<color>
{
    visitor_fill(color const& val)
        : val_(val) {}

    template <typename T2>
    void operator() (T2 & data)
    {
        using pixel_type = typename T2::pixel_type;
        pixel_type val = static_cast<pixel_type>(val_.rgba());
        data.set(val);
    }

  private:
    color const& val_;
};

} // end detail ns

// For all the generic data types.
template <typename T1, typename T2>
MAPNIK_DECL void fill (T1 & data, T2 const& val)
{
    util::apply_visitor(detail::visitor_fill<T2>(val), data);
}

template MAPNIK_DECL void fill(image_any &, color const&);
template MAPNIK_DECL void fill(image_any &, uint32_t const&);
template MAPNIK_DECL void fill(image_any &, int32_t const&);
template MAPNIK_DECL void fill(image_any &, uint16_t const&);
template MAPNIK_DECL void fill(image_any &, int16_t const&);
template MAPNIK_DECL void fill(image_any &, uint8_t const&);
template MAPNIK_DECL void fill(image_any &, int8_t const&);
template MAPNIK_DECL void fill(image_any &, float const&);
template MAPNIK_DECL void fill(image_any &, double const&);


// Temporary remove these later!
template <>
MAPNIK_DECL void fill<image_rgba8, color> (image_rgba8 & data , color const& val)
{
    detail::visitor_fill<color> visitor(val);
    visitor(data);
}

// Temporary remove these later!
template <>
MAPNIK_DECL void fill<image_rgba8, uint32_t> (image_rgba8 & data , uint32_t const& val)
{
    detail::visitor_fill<uint32_t> visitor(val);
    visitor(data);
}

// Temporary remove these later!
template <>
MAPNIK_DECL void fill<image_rgba8, int32_t> (image_rgba8 & data , int32_t const& val)
{
    detail::visitor_fill<int32_t> visitor(val);
    visitor(data);
}

namespace detail {

struct visitor_set_rectangle
{
    visitor_set_rectangle(image_any const & src, int x0, int y0)
        : src_(src), x0_(x0), y0_(y0) {}

    template <typename T>
    void operator() (T & dst)
    {
        using pixel_type = typename T::pixel_type;
        T src = util::get<T>(src_);
        box2d<int> ext0(0,0,dst.width(),dst.height());
        box2d<int> ext1(x0_,y0_,x0_+src.width(),y0_+src.height());

        if (ext0.intersects(ext1))
        {
            box2d<int> box = ext0.intersect(ext1);
            for (std::size_t y = box.miny(); y < box.maxy(); ++y)
            {
                pixel_type* row_to =  dst.getRow(y);
                pixel_type const * row_from = src.getRow(y-y0_);

                for (std::size_t x = box.minx(); x < box.maxx(); ++x)
                {
                    row_to[x] = row_from[x-x0_];
                }
            }
        }
    }
  private:
    image_any const& src_;
    int x0_;
    int y0_;
};

template <>
void visitor_set_rectangle::operator()<image_rgba8> (image_rgba8 & dst)
{
    using pixel_type = typename image_rgba8::pixel_type;
    image_rgba8 src = util::get<image_rgba8>(src_);
    box2d<int> ext0(0,0,dst.width(),dst.height());
    box2d<int> ext1(x0_,y0_,x0_+src.width(),y0_+src.height());

    if (ext0.intersects(ext1))
    {
        box2d<int> box = ext0.intersect(ext1);
        for (std::size_t y = box.miny(); y < box.maxy(); ++y)
        {
            pixel_type* row_to =  dst.getRow(y);
            pixel_type const * row_from = src.getRow(y-y0_);

            for (std::size_t x = box.minx(); x < box.maxx(); ++x)
            {
                if (row_from[x-x0_] & 0xff000000) // Don't change if alpha == 0
                {
                    row_to[x] = row_from[x-x0_];
                }
            }
        }
    }
}

template<>
void visitor_set_rectangle::operator()<image_null> (image_null &)
{
    throw std::runtime_error("Set rectangle not support for null images");
}

} // end detail ns 

template <typename T>
MAPNIK_DECL void set_rectangle (T & dst, T const& src, int x, int y)
{
    detail::visitor_set_rectangle visit(src, x, y);
    visit(dst);
}

template <>
MAPNIK_DECL void set_rectangle<image_any> (image_any & dst, image_any const& src, int x, int y)
{
    util::apply_visitor(detail::visitor_set_rectangle(src, x, y), dst);
}

namespace detail
{

struct visitor_composite_pixel
{
    // Obviously c variable would only work for rgba8 currently, but didn't want to 
    // make this a template class until new rgba types exist. 
    visitor_composite_pixel(unsigned op, int x,int y, unsigned c, unsigned cover, double opacity)
        :   opacity_(opacity),
            op_(op),
            x_(x),
            y_(y),
            c_(c),
            cover_(cover) {}

    template <typename T>
    void operator() (T & data)
    {
        throw std::runtime_error("Composite pixel is not supported for this data type");
    }

  private:
    double opacity_;
    unsigned op_;
    int x_;
    int y_;
    int c_;
    unsigned cover_;

};

template<>
void visitor_composite_pixel::operator()<image_rgba8> (image_rgba8 & data)
{
    using color_type = agg::rgba8;
    using value_type = color_type::value_type;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba<color_type,order_type>;

    if (mapnik::check_bounds(data, x_, y_))
    {
        unsigned rgba = data(x_,y_);
        unsigned ca = (unsigned)(((c_ >> 24) & 0xff) * opacity_);
        unsigned cb = (c_ >> 16 ) & 0xff;
        unsigned cg = (c_ >> 8) & 0xff;
        unsigned cr = (c_ & 0xff);
        blender_type::blend_pix(op_, (value_type*)&rgba, cr, cg, cb, ca, cover_);
        data(x_,y_) = rgba;
    }
}

} // end detail ns

template <typename T>
MAPNIK_DECL void composite_pixel(T & data, unsigned op, int x, int y, unsigned c, unsigned cover, double opacity )
{
    util::apply_visitor(detail::visitor_composite_pixel(op, x, y, c, cover, opacity), data);
}

// Temporary delete later
template <>
MAPNIK_DECL void composite_pixel<image_rgba8>(image_rgba8 & data, unsigned op, int x, int y, unsigned c, unsigned cover, double opacity )
{
    detail::visitor_composite_pixel visitor(op, x, y, c, cover, opacity);
    visitor(data);
}

namespace detail {

template <typename T1>
struct visitor_set_pixel
{
    visitor_set_pixel(std::size_t x, std::size_t y, T1 const& val)
        : val_(val), x_(x), y_(y) {}

    template <typename T2>
    void operator() (T2 & data)
    {
        using pixel_type = typename T2::pixel_type;
        pixel_type val = static_cast<pixel_type>(val_);
        if (check_bounds(data, x_, y_))
        {
            data(x_, y_) = val;
        }
    }

  private:
    T1 const& val_;
    std::size_t x_;
    std::size_t y_;
};

template<>
struct visitor_set_pixel<color>
{
    visitor_set_pixel(std::size_t x, std::size_t y, color const& val)
        : val_(val), x_(x), y_(y) {}

    template <typename T2>
    void operator() (T2 & data)
    {
        using pixel_type = typename T2::pixel_type;
        pixel_type val = static_cast<pixel_type>(val_.rgba());
        if (check_bounds(data, x_, y_))
        {
            data(x_, y_) = val;
        }
    }

  private:
    color const& val_;
    std::size_t x_;
    std::size_t y_;
};

} // end detail ns

// For all the generic data types.
template <typename T1, typename T2>
MAPNIK_DECL void set_pixel (T1 & data, std::size_t x, std::size_t y, T2 const& val)
{
    util::apply_visitor(detail::visitor_set_pixel<T2>(x, y, val), data);
}

template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, double const&);


// Temporary remove these later!
template <>
MAPNIK_DECL void set_pixel<image_rgba8, color> (image_rgba8 & data, std::size_t x, std::size_t y, color const& val)
{
    detail::visitor_set_pixel<color> visitor(x, y, val);
    visitor(data);
}

// Temporary remove these later!
template <>
MAPNIK_DECL void set_pixel<image_rgba8, uint32_t> (image_rgba8 & data, std::size_t x, std::size_t y, uint32_t const& val)
{
    detail::visitor_set_pixel<uint32_t> visitor(x, y, val);
    visitor(data);
}

// Temporary remove these later!
template <>
MAPNIK_DECL void set_pixel<image_rgba8, int32_t> (image_rgba8 & data, std::size_t x, std::size_t y, int32_t const& val)
{
    detail::visitor_set_pixel<int32_t> visitor(x, y, val);
    visitor(data);
}

namespace detail {

template <typename T1>
struct visitor_get_pixel
{
    visitor_get_pixel(std::size_t x, std::size_t y)
        : x_(x), y_(y) {}

    template <typename T2>
    T1 operator() (T2 const& data)
    {
        if (check_bounds(data, x_, y_))
        {
            return static_cast<T1>(data(x_, y_));
        }
        else
        {
            throw std::runtime_error("Out of range for dataset with get pixel");
        }
    }

  private:
    std::size_t x_;
    std::size_t y_;
};

template<>
struct visitor_get_pixel<color>
{
    visitor_get_pixel(std::size_t x, std::size_t y)
        : x_(x), y_(y) {}

    template <typename T2>
    color operator() (T2 const& data)
    {
        if (check_bounds(data, x_, y_))
        {
            uint32_t val = static_cast<uint32_t>(data(x_, y_));
            return color(static_cast<uint8_t>(val),
                         static_cast<uint8_t>((val >>= 8)),
                         static_cast<uint8_t>((val >>= 8)),
                         static_cast<uint8_t>((val >>= 8)));
        }
        else
        {
            throw std::runtime_error("Out of range for dataset with get pixel");
        }
    }

  private:
    std::size_t x_;
    std::size_t y_;
};

} // end detail ns

// For all the generic data types.
template <typename T1, typename T2>
MAPNIK_DECL T2 get_pixel (T1 const& data, std::size_t x, std::size_t y)
{
    return util::apply_visitor(detail::visitor_get_pixel<T2>(x, y), data);
}

template MAPNIK_DECL color get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_any const&, std::size_t, std::size_t); 


// Temporary remove these later!
template <>
MAPNIK_DECL color get_pixel<image_rgba8, color> (image_rgba8 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<color> visitor(x, y);
    return visitor(data);
}

// Temporary remove these later!
template <>
MAPNIK_DECL uint32_t get_pixel<image_rgba8, uint32_t> (image_rgba8 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<uint32_t> visitor(x, y);
    return visitor(data);
}

// Temporary remove these later!
template <>
MAPNIK_DECL int32_t get_pixel<image_rgba8, int32_t> (image_rgba8 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<int32_t> visitor(x, y);
    return visitor(data);
}

namespace detail 
{

struct visitor_view_to_string
{
    visitor_view_to_string(std::ostringstream & ss)
        : ss_(ss) {}

    template <typename T>
    void operator() (T const& view)
    {
        for (std::size_t i=0;i<view.height();i++)
        {
            ss_.write(reinterpret_cast<const char*>(view.getRow(i)),
                     view.getRowSize());
        }
    }

  private:
    std::ostringstream & ss_;
};

} // end detail ns 


MAPNIK_DECL void view_to_string (image_view_any const& view, std::ostringstream & ss)
{
    util::apply_visitor(detail::visitor_view_to_string(ss), view);
}

namespace detail
{

struct visitor_create_view
{
    visitor_create_view(unsigned x,unsigned y, unsigned w, unsigned h)
        : x_(x), y_(y), w_(w), h_(h) {}

    template <typename T>
    image_view_any operator() (T const& data)
    {
        image_view<T> view(x_,y_,w_,h_,data);
        return image_view_any(view);
    }
  private:
    unsigned x_;
    unsigned y_;
    unsigned w_;
    unsigned h_;
};

template <>
image_view_any visitor_create_view::operator()<image_null> (image_null const&)
{
    throw std::runtime_error("Can not make a view from a null image");
}

} // end detail ns

MAPNIK_DECL image_view_any create_view(image_any const& data,unsigned x,unsigned y, unsigned w,unsigned h)
{
    return util::apply_visitor(detail::visitor_create_view(x,y,w,h), data);
}

} // end ns
