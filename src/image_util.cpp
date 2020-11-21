/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/safe_cast.hpp>
#ifdef SSE_MATH
#include <mapnik/sse.hpp>
#endif

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#if __cpp_lib_execution >= 201603
#include <execution>
#endif

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
    else throw image_writer_exception("Could not write file to " + filename );
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
    else throw image_writer_exception("Could not write file to " + filename );
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
        if (boost::algorithm::starts_with(t, "png"))
        {
            png_saver_pal visitor(stream, t, palette);
            mapnik::util::apply_visitor(visitor, image);
        }
        else if (boost::algorithm::starts_with(t, "tif"))
        {
            throw image_writer_exception("palettes are not currently supported when writing to tiff format (yet)");
        }
        else if (boost::algorithm::starts_with(t, "jpeg"))
        {
            throw image_writer_exception("palettes are not currently supported when writing to jpeg format");
        }
        else throw image_writer_exception("unknown file type: " + type);
    }
    else throw image_writer_exception("Could not write to empty stream" );
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
        if (boost::algorithm::starts_with(t, "png"))
        {
            png_saver_pal visitor(stream, t, palette);
            visitor(image);
            //mapnik::util::apply_visitor(visitor, image);
        }
        else if (boost::algorithm::starts_with(t, "tif"))
        {
            throw image_writer_exception("palettes are not currently supported when writing to tiff format (yet)");
        }
        else if (boost::algorithm::starts_with(t, "jpeg"))
        {
            throw image_writer_exception("palettes are not currently supported when writing to jpeg format");
        }
        else throw image_writer_exception("unknown file type: " + type);
    }
    else throw image_writer_exception("Could not write to empty stream" );
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
        if (boost::algorithm::starts_with(t, "png"))
        {
            png_saver_pal visitor(stream, t, palette);
            visitor(image);
            //mapnik::util::apply_visitor(visitor, image);
        }
        else if (boost::algorithm::starts_with(t, "tif"))
        {
            throw image_writer_exception("palettes are not currently supported when writing to tiff format (yet)");
        }
        else if (boost::algorithm::starts_with(t, "jpeg"))
        {
            throw image_writer_exception("palettes are not currently supported when writing to jpeg format");
        }
        else throw image_writer_exception("unknown file type: " + type);
    }
    else throw image_writer_exception("Could not write to empty stream" );
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
        if (boost::algorithm::starts_with(t, "png"))
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
        else throw image_writer_exception("unknown file type: " + type);
    }
    else throw image_writer_exception("Could not write to empty stream" );
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
        if (boost::algorithm::starts_with(t, "png"))
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
        else throw image_writer_exception("unknown file type: " + type);
    }
    else throw image_writer_exception("Could not write to empty stream" );
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
        if (boost::algorithm::starts_with(t, "png"))
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
        else throw image_writer_exception("unknown file type: " + type);
    }
    else throw image_writer_exception("Could not write to empty stream" );
}

template <typename T>
MAPNIK_DECL void save_to_file(T const& image, std::string const& filename)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        save_to_file<T>(image, filename, *type);
    }
    else throw image_writer_exception("Could not write file to " + filename );
}

template <typename T>
MAPNIK_DECL void save_to_file(T const& image, std::string const& filename, rgba_palette const& palette)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        save_to_file<T>(image, filename, *type, palette);
    }
    else throw image_writer_exception("Could not write file to " + filename );
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

template MAPNIK_DECL std::string save_to_string<image_view_rgba8> (image_view_rgba8 const&,
                                                                 std::string const&);

template MAPNIK_DECL std::string save_to_string<image_view_rgba8> (image_view_rgba8 const&,
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
    bool operator() (image_view_null const&) const
    {
        return true;
    }

    bool operator() (image_null const&) const
    {
        return true;
    }
    template <typename T>
    bool operator() (image_view<T> const& view) const
    {
        using pixel_type = typename T::pixel_type;
        if (view.width() > 0 && view.height() > 0)
        {
            pixel_type const* first_row = view.get_row(0);
            pixel_type const first_pixel = first_row[0];
            for (std::size_t y = 0; y < view.height(); ++y)
            {
                pixel_type const * row = view.get_row(y);
                for (std::size_t x = 0; x < view.width(); ++x)
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

    template <typename T>
    bool operator() (T const& image) const
    {
        using pixel_type = typename T::pixel_type;
        if (image.size() > 0)
        {
            pixel_type const first_p = *image.begin();
            auto itr = std::find_if(/*std::execution::par_unseq,*/ // still missing on ubuntu with
                                                                   // clang++10/libc++ (!)
                                    image.begin(), image.end(),
                                    [first_p](pixel_type const p) {
                                        return first_p != p;
                                    });
            return (itr == image.end());
        }
        return true;
    }
};

} // end detail ns

MAPNIK_DECL bool is_solid(image_any const& image)
{
    return util::apply_visitor(detail::is_solid_visitor(), image);
}

MAPNIK_DECL bool is_solid(image_view_any const& image)
{
    return util::apply_visitor(detail::is_solid_visitor(), image);
}

template <typename T>
MAPNIK_DECL bool is_solid(T const& image)
{
    detail::is_solid_visitor visitor;
    return visitor(image);
}

template MAPNIK_DECL bool is_solid(image_null const&);
template MAPNIK_DECL bool is_solid(image_rgba8 const&);
template MAPNIK_DECL bool is_solid(image_gray8 const&);
template MAPNIK_DECL bool is_solid(image_gray8s const&);
template MAPNIK_DECL bool is_solid(image_gray16 const&);
template MAPNIK_DECL bool is_solid(image_gray16s const&);
template MAPNIK_DECL bool is_solid(image_gray32 const&);
template MAPNIK_DECL bool is_solid(image_gray32s const&);
template MAPNIK_DECL bool is_solid(image_gray32f const&);
template MAPNIK_DECL bool is_solid(image_gray64 const&);
template MAPNIK_DECL bool is_solid(image_gray64s const&);
template MAPNIK_DECL bool is_solid(image_gray64f const&);
template MAPNIK_DECL bool is_solid(image_view_null const&);
template MAPNIK_DECL bool is_solid(image_view_rgba8 const&);
template MAPNIK_DECL bool is_solid(image_view_gray8 const&);
template MAPNIK_DECL bool is_solid(image_view_gray8s const&);
template MAPNIK_DECL bool is_solid(image_view_gray16 const&);
template MAPNIK_DECL bool is_solid(image_view_gray16s const&);
template MAPNIK_DECL bool is_solid(image_view_gray32 const&);
template MAPNIK_DECL bool is_solid(image_view_gray32s const&);
template MAPNIK_DECL bool is_solid(image_view_gray32f const&);
template MAPNIK_DECL bool is_solid(image_view_gray64 const&);
template MAPNIK_DECL bool is_solid(image_view_gray64s const&);
template MAPNIK_DECL bool is_solid(image_view_gray64f const&);

namespace detail {

struct premultiply_visitor
{
    bool operator() (image_rgba8 & data) const
    {
        if (!data.get_premultiplied())
        {
            agg::rendering_buffer buffer(data.bytes(),safe_cast<unsigned>(data.width()),safe_cast<unsigned>(data.height()),safe_cast<int>(data.row_size()));
            agg::pixfmt_rgba32 pixf(buffer);
            pixf.premultiply();
            data.set_premultiplied(true);
            return true;
        }
        return false;
    }

    template <typename T>
    bool operator() (T &) const
    {
        return false;
    }
};

struct demultiply_visitor
{
    bool operator() (image_rgba8 & data) const
    {
        if (data.get_premultiplied())
        {
            agg::rendering_buffer buffer(data.bytes(),safe_cast<unsigned>(data.width()),safe_cast<unsigned>(data.height()),safe_cast<int>(data.row_size()));
            agg::pixfmt_rgba32_pre pixf(buffer);
            pixf.demultiply();
            data.set_premultiplied(false);
            return true;
        }
        return false;
    }

    template <typename T>
    bool operator() (T &) const
    {
        return false;
    }
};

struct set_premultiplied_visitor
{
    set_premultiplied_visitor(bool status)
        : status_(status) {}

    template <typename T>
    void operator() (T & data) const
    {
        data.set_premultiplied(status_);
    }
private:
    bool const status_;
};

} // end detail ns

MAPNIK_DECL bool premultiply_alpha(image_any & image)
{
    return util::apply_visitor(detail::premultiply_visitor(), image);
}

template <typename T>
MAPNIK_DECL bool premultiply_alpha(T & image)
{
    detail::premultiply_visitor visit;
    return visit(image);
}

template MAPNIK_DECL bool premultiply_alpha(image_rgba8 &);
template MAPNIK_DECL bool premultiply_alpha(image_gray8 &);
template MAPNIK_DECL bool premultiply_alpha(image_gray8s &);
template MAPNIK_DECL bool premultiply_alpha(image_gray16 &);
template MAPNIK_DECL bool premultiply_alpha(image_gray16s &);
template MAPNIK_DECL bool premultiply_alpha(image_gray32 &);
template MAPNIK_DECL bool premultiply_alpha(image_gray32s &);
template MAPNIK_DECL bool premultiply_alpha(image_gray32f &);
template MAPNIK_DECL bool premultiply_alpha(image_gray64 &);
template MAPNIK_DECL bool premultiply_alpha(image_gray64s &);
template MAPNIK_DECL bool premultiply_alpha(image_gray64f &);

MAPNIK_DECL bool demultiply_alpha(image_any & image)
{
    return util::apply_visitor(detail::demultiply_visitor(), image);
}

template <typename T>
MAPNIK_DECL bool demultiply_alpha(T & image)
{
    detail::demultiply_visitor visit;
    return visit(image);
}

template MAPNIK_DECL bool demultiply_alpha(image_rgba8 &);
template MAPNIK_DECL bool demultiply_alpha(image_gray8 &);
template MAPNIK_DECL bool demultiply_alpha(image_gray8s &);
template MAPNIK_DECL bool demultiply_alpha(image_gray16 &);
template MAPNIK_DECL bool demultiply_alpha(image_gray16s &);
template MAPNIK_DECL bool demultiply_alpha(image_gray32 &);
template MAPNIK_DECL bool demultiply_alpha(image_gray32s &);
template MAPNIK_DECL bool demultiply_alpha(image_gray32f &);
template MAPNIK_DECL bool demultiply_alpha(image_gray64 &);
template MAPNIK_DECL bool demultiply_alpha(image_gray64s &);
template MAPNIK_DECL bool demultiply_alpha(image_gray64f &);

MAPNIK_DECL void set_premultiplied_alpha(image_any & image, bool status)
{
    util::apply_visitor(detail::set_premultiplied_visitor(status), image);
}

template <typename T>
MAPNIK_DECL void set_premultiplied_alpha(T & image, bool status)
{
    detail::set_premultiplied_visitor visit(status);
    visit(image);
}

template MAPNIK_DECL void set_premultiplied_alpha(image_rgba8 &, bool);
template MAPNIK_DECL void set_premultiplied_alpha(image_gray8 &, bool);
template MAPNIK_DECL void set_premultiplied_alpha(image_gray8s &, bool);
template MAPNIK_DECL void set_premultiplied_alpha(image_gray16 &, bool);
template MAPNIK_DECL void set_premultiplied_alpha(image_gray16s &, bool);
template MAPNIK_DECL void set_premultiplied_alpha(image_gray32 &, bool);
template MAPNIK_DECL void set_premultiplied_alpha(image_gray32s &, bool);
template MAPNIK_DECL void set_premultiplied_alpha(image_gray32f &, bool);
template MAPNIK_DECL void set_premultiplied_alpha(image_gray64 &, bool);
template MAPNIK_DECL void set_premultiplied_alpha(image_gray64s &, bool);
template MAPNIK_DECL void set_premultiplied_alpha(image_gray64f &, bool);

namespace detail {

namespace  {

template <typename T>
inline T clamp(T d, T min, T max)
{
    T const t = d < min ? min : d;
    return t > max ? max : t;
}

}
struct visitor_apply_opacity
{
    visitor_apply_opacity(float opacity)
        : opacity_(clamp(opacity, 0.0f, 1.0f)) {}

    void operator() (image_rgba8 & data) const
    {
        using pixel_type = image_rgba8::pixel_type;
        for (std::size_t y = 0; y < data.height(); ++y)
        {
            pixel_type* row_to =  data.get_row(y);
            for (std::size_t x = 0; x < data.width(); ++x)
            {
                pixel_type rgba = row_to[x];
                pixel_type a = static_cast<pixel_type>(((rgba >> 24u) & 0xff) * opacity_);
                pixel_type r = rgba & 0xff;
                pixel_type g = (rgba >> 8u ) & 0xff;
                pixel_type b = (rgba >> 16u) & 0xff;

                row_to[x] = (a << 24u) | (b << 16u) |  (g << 8u) | (r) ;
            }
        }
    }

    template <typename T>
    void operator() (T & data) const
    {
        throw std::runtime_error("Error: apply_opacity with " + std::string(typeid(data).name()) + " is not supported");
    }

private:
    float const opacity_;
};

} // end detail ns

MAPNIK_DECL void apply_opacity(image_any & data, float opacity)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    util::apply_visitor(detail::visitor_apply_opacity(opacity), data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

template <typename T>
MAPNIK_DECL void apply_opacity(T & data, float opacity)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    detail::visitor_apply_opacity visit(opacity);
    visit(data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

template MAPNIK_DECL void apply_opacity(image_rgba8 &, float);
template MAPNIK_DECL void apply_opacity(image_gray8 &, float);
template MAPNIK_DECL void apply_opacity(image_gray8s &, float);
template MAPNIK_DECL void apply_opacity(image_gray16 &, float);
template MAPNIK_DECL void apply_opacity(image_gray16s &, float);
template MAPNIK_DECL void apply_opacity(image_gray32 &, float);
template MAPNIK_DECL void apply_opacity(image_gray32s &, float);
template MAPNIK_DECL void apply_opacity(image_gray32f &, float);
template MAPNIK_DECL void apply_opacity(image_gray64 &, float);
template MAPNIK_DECL void apply_opacity(image_gray64s &, float);
template MAPNIK_DECL void apply_opacity(image_gray64f &, float);

namespace detail {

struct visitor_set_grayscale_to_alpha
{
    void operator() (image_rgba8 & data) const
    {
        using pixel_type = image_rgba8::pixel_type;
        for (std::size_t y = 0; y < data.height(); ++y)
        {
            pixel_type* row_from = data.get_row(y);
            for (std::size_t x = 0; x < data.width(); ++x)
            {
                pixel_type rgba = row_from[x];
                pixel_type r = rgba & 0xff;
                pixel_type g = (rgba >> 8u) & 0xff;
                pixel_type b = (rgba >> 16u) & 0xff;

                // magic numbers for grayscale
                pixel_type a = static_cast<pixel_type>(std::ceil((r * .3) + (g * .59) + (b * .11)));

                row_from[x] = (a << 24u) | (255 << 16u) |  (255 << 8u) | (255u) ;
            }
        }
    }

    template <typename T>
    void operator() (T & data) const
    {
        MAPNIK_LOG_WARN(image_util) << "Warning: set_grayscale_to_alpha with " + std::string(typeid(data).name()) + " is not supported, image was not modified";
    }
};

struct visitor_set_grayscale_to_alpha_c
{
    visitor_set_grayscale_to_alpha_c(color const& c)
        : c_(c) {}

    void operator() (image_rgba8 & data) const
    {
        using pixel_type = image_rgba8::pixel_type;
        for (std::size_t y = 0; y < data.height(); ++y)
        {
            pixel_type* row_from = data.get_row(y);
            for (std::size_t x = 0; x < data.width(); ++x)
            {
                pixel_type rgba = row_from[x];
                pixel_type r = rgba & 0xff;
                pixel_type g = (rgba >> 8 ) & 0xff;
                pixel_type b = (rgba >> 16) & 0xff;

                // magic numbers for grayscale
                pixel_type a = static_cast<pixel_type>(std::ceil((r * .3) + (g * .59) + (b * .11)));

                row_from[x] = static_cast<unsigned>(a << 24u) |
                              static_cast<unsigned>(c_.blue() << 16u) |
                              static_cast<unsigned>(c_.green() << 8u) |
                              static_cast<unsigned>(c_.red() );
            }
        }
    }

    template <typename T>
    void operator() (T & data) const
    {
        MAPNIK_LOG_WARN(image_util) << "Warning: set_grayscale_to_alpha with " + std::string(typeid(data).name()) + " is not supported, image was not modified";
    }

private:
    color const& c_;
};

} // end detail ns

MAPNIK_DECL void set_grayscale_to_alpha(image_any & data)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    util::apply_visitor(detail::visitor_set_grayscale_to_alpha(), data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

template <typename T>
MAPNIK_DECL void set_grayscale_to_alpha(T & data)
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

template MAPNIK_DECL void set_grayscale_to_alpha(image_rgba8 &);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray8 &);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray8s &);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray16 &);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray16s &);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray32 &);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray32s &);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray32f &);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray64 &);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray64s &);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray64f &);

MAPNIK_DECL void set_grayscale_to_alpha(image_any & data, color const& c)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    util::apply_visitor(detail::visitor_set_grayscale_to_alpha_c(c), data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

template <typename T>
MAPNIK_DECL void set_grayscale_to_alpha(T & data, color const& c)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    detail::visitor_set_grayscale_to_alpha_c visit(c);
    visit(data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

template MAPNIK_DECL void set_grayscale_to_alpha(image_rgba8 &, color const&);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray8 &, color const&);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray8s &, color const&);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray16 &, color const&);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray16s &, color const&);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray32 &, color const&);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray32s &, color const&);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray32f &, color const&);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray64 &, color const&);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray64s &, color const&);
template MAPNIK_DECL void set_grayscale_to_alpha(image_gray64f &, color const&);

namespace detail {

struct visitor_set_color_to_alpha
{
    visitor_set_color_to_alpha(color const& c)
        : c_(c) {}

    void operator() (image_rgba8 & data) const
    {
        using pixel_type = image_rgba8::pixel_type;
        for (std::size_t y = 0; y < data.height(); ++y)
        {
            pixel_type* row_from = data.get_row(y);
            for (std::size_t x = 0; x < data.width(); ++x)
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

    template <typename T>
    void operator() (T & data) const
    {
        throw std::runtime_error("Error: set_color_to_alpha with " + std::string(typeid(data).name()) + " is not supported");
    }

private:
    color const& c_;

};


} // end detail ns

MAPNIK_DECL void set_color_to_alpha (image_any & data, color const& c)
{
    // Prior to calling the data must not be premultiplied
    bool remultiply = mapnik::demultiply_alpha(data);
    util::apply_visitor(detail::visitor_set_color_to_alpha(c), data);
    if (remultiply)
    {
        mapnik::premultiply_alpha(data);
    }
}

template <typename T>
MAPNIK_DECL void set_color_to_alpha(T & data, color const& c)
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

template MAPNIK_DECL void set_color_to_alpha(image_rgba8 &, color const&);
template MAPNIK_DECL void set_color_to_alpha(image_gray8 &, color const&);
template MAPNIK_DECL void set_color_to_alpha(image_gray8s &, color const&);
template MAPNIK_DECL void set_color_to_alpha(image_gray16 &, color const&);
template MAPNIK_DECL void set_color_to_alpha(image_gray16s &, color const&);
template MAPNIK_DECL void set_color_to_alpha(image_gray32 &, color const&);
template MAPNIK_DECL void set_color_to_alpha(image_gray32s &, color const&);
template MAPNIK_DECL void set_color_to_alpha(image_gray32f &, color const&);
template MAPNIK_DECL void set_color_to_alpha(image_gray64 &, color const&);
template MAPNIK_DECL void set_color_to_alpha(image_gray64s &, color const&);
template MAPNIK_DECL void set_color_to_alpha(image_gray64f &, color const&);

namespace detail {

template <typename T1>
struct visitor_fill
{
    visitor_fill(T1 const& val)
        : val_(val) {}

    template <typename T2>
    void operator() (T2 & data) const
    {
        using pixel_type = typename T2::pixel_type;
        data.set(safe_cast<pixel_type>(val_));
    }

private:
    T1 const& val_;
};

template<>
struct visitor_fill<color>
{
    visitor_fill(color const& val)
        : val_(val) {}

    void operator() (image_rgba8 & data) const
    {
        using pixel_type = image_rgba8::pixel_type;
        pixel_type val = static_cast<pixel_type>(val_.rgba());
        data.set(val);
        data.set_premultiplied(val_.get_premultiplied());
    }

    template <typename T2>
    void operator() (T2 & data) const
    {
        using pixel_type = typename T2::pixel_type;
        pixel_type val = static_cast<pixel_type>(val_.rgba());
        data.set(val);
    }

private:
    color const& val_;
};

} // end detail ns

template <typename T>
MAPNIK_DECL void fill (image_any & data, T const& val)
{
    util::apply_visitor(detail::visitor_fill<T>(val), data);
}

template MAPNIK_DECL void fill(image_any &, color const&);
template MAPNIK_DECL void fill(image_any &, uint64_t const&);
template MAPNIK_DECL void fill(image_any &, int64_t const&);
template MAPNIK_DECL void fill(image_any &, uint32_t const&);
template MAPNIK_DECL void fill(image_any &, int32_t const&);
template MAPNIK_DECL void fill(image_any &, uint16_t const&);
template MAPNIK_DECL void fill(image_any &, int16_t const&);
template MAPNIK_DECL void fill(image_any &, uint8_t const&);
template MAPNIK_DECL void fill(image_any &, int8_t const&);
template MAPNIK_DECL void fill(image_any &, float const&);
template MAPNIK_DECL void fill(image_any &, double const&);

template <typename T>
MAPNIK_DECL void fill (image_rgba8 & data, T const& val)
{
    detail::visitor_fill<T> visitor(val);
    return visitor(data);
}

template MAPNIK_DECL void fill(image_rgba8 &, color const&);
template MAPNIK_DECL void fill(image_rgba8 &, uint64_t const&);
template MAPNIK_DECL void fill(image_rgba8 &, int64_t const&);
template MAPNIK_DECL void fill(image_rgba8 &, uint32_t const&);
template MAPNIK_DECL void fill(image_rgba8 &, int32_t const&);
template MAPNIK_DECL void fill(image_rgba8 &, uint16_t const&);
template MAPNIK_DECL void fill(image_rgba8 &, int16_t const&);
template MAPNIK_DECL void fill(image_rgba8 &, uint8_t const&);
template MAPNIK_DECL void fill(image_rgba8 &, int8_t const&);
template MAPNIK_DECL void fill(image_rgba8 &, float const&);
template MAPNIK_DECL void fill(image_rgba8 &, double const&);

template <typename T>
MAPNIK_DECL void fill (image_gray8 & data, T const& val)
{
    detail::visitor_fill<T> visitor(val);
    return visitor(data);
}

template MAPNIK_DECL void fill(image_gray8 &, color const&);
template MAPNIK_DECL void fill(image_gray8 &, uint64_t const&);
template MAPNIK_DECL void fill(image_gray8 &, int64_t const&);
template MAPNIK_DECL void fill(image_gray8 &, uint32_t const&);
template MAPNIK_DECL void fill(image_gray8 &, int32_t const&);
template MAPNIK_DECL void fill(image_gray8 &, uint16_t const&);
template MAPNIK_DECL void fill(image_gray8 &, int16_t const&);
template MAPNIK_DECL void fill(image_gray8 &, uint8_t const&);
template MAPNIK_DECL void fill(image_gray8 &, int8_t const&);
template MAPNIK_DECL void fill(image_gray8 &, float const&);
template MAPNIK_DECL void fill(image_gray8 &, double const&);

template <typename T>
MAPNIK_DECL void fill (image_gray8s & data, T const& val)
{
    detail::visitor_fill<T> visitor(val);
    return visitor(data);
}

template MAPNIK_DECL void fill(image_gray8s &, color const&);
template MAPNIK_DECL void fill(image_gray8s &, uint64_t const&);
template MAPNIK_DECL void fill(image_gray8s &, int64_t const&);
template MAPNIK_DECL void fill(image_gray8s &, uint32_t const&);
template MAPNIK_DECL void fill(image_gray8s &, int32_t const&);
template MAPNIK_DECL void fill(image_gray8s &, uint16_t const&);
template MAPNIK_DECL void fill(image_gray8s &, int16_t const&);
template MAPNIK_DECL void fill(image_gray8s &, uint8_t const&);
template MAPNIK_DECL void fill(image_gray8s &, int8_t const&);
template MAPNIK_DECL void fill(image_gray8s &, float const&);
template MAPNIK_DECL void fill(image_gray8s &, double const&);

template <typename T>
MAPNIK_DECL void fill (image_gray16 & data, T const& val)
{
    detail::visitor_fill<T> visitor(val);
    return visitor(data);
}

template MAPNIK_DECL void fill(image_gray16 &, color const&);
template MAPNIK_DECL void fill(image_gray16 &, uint64_t const&);
template MAPNIK_DECL void fill(image_gray16 &, int64_t const&);
template MAPNIK_DECL void fill(image_gray16 &, uint32_t const&);
template MAPNIK_DECL void fill(image_gray16 &, int32_t const&);
template MAPNIK_DECL void fill(image_gray16 &, uint16_t const&);
template MAPNIK_DECL void fill(image_gray16 &, int16_t const&);
template MAPNIK_DECL void fill(image_gray16 &, uint8_t const&);
template MAPNIK_DECL void fill(image_gray16 &, int8_t const&);
template MAPNIK_DECL void fill(image_gray16 &, float const&);
template MAPNIK_DECL void fill(image_gray16 &, double const&);

template <typename T>
MAPNIK_DECL void fill (image_gray16s & data, T const& val)
{
    detail::visitor_fill<T> visitor(val);
    return visitor(data);
}

template MAPNIK_DECL void fill(image_gray16s &, color const&);
template MAPNIK_DECL void fill(image_gray16s &, uint64_t const&);
template MAPNIK_DECL void fill(image_gray16s &, int64_t const&);
template MAPNIK_DECL void fill(image_gray16s &, uint32_t const&);
template MAPNIK_DECL void fill(image_gray16s &, int32_t const&);
template MAPNIK_DECL void fill(image_gray16s &, uint16_t const&);
template MAPNIK_DECL void fill(image_gray16s &, int16_t const&);
template MAPNIK_DECL void fill(image_gray16s &, uint8_t const&);
template MAPNIK_DECL void fill(image_gray16s &, int8_t const&);
template MAPNIK_DECL void fill(image_gray16s &, float const&);
template MAPNIK_DECL void fill(image_gray16s &, double const&);

template <typename T>
MAPNIK_DECL void fill (image_gray32 & data, T const& val)
{
    detail::visitor_fill<T> visitor(val);
    return visitor(data);
}

template MAPNIK_DECL void fill(image_gray32 &, color const&);
template MAPNIK_DECL void fill(image_gray32 &, uint64_t const&);
template MAPNIK_DECL void fill(image_gray32 &, int64_t const&);
template MAPNIK_DECL void fill(image_gray32 &, uint32_t const&);
template MAPNIK_DECL void fill(image_gray32 &, int32_t const&);
template MAPNIK_DECL void fill(image_gray32 &, uint16_t const&);
template MAPNIK_DECL void fill(image_gray32 &, int16_t const&);
template MAPNIK_DECL void fill(image_gray32 &, uint8_t const&);
template MAPNIK_DECL void fill(image_gray32 &, int8_t const&);
template MAPNIK_DECL void fill(image_gray32 &, float const&);
template MAPNIK_DECL void fill(image_gray32 &, double const&);

template <typename T>
MAPNIK_DECL void fill (image_gray32s & data, T const& val)
{
    detail::visitor_fill<T> visitor(val);
    return visitor(data);
}

template MAPNIK_DECL void fill(image_gray32s &, color const&);
template MAPNIK_DECL void fill(image_gray32s &, uint64_t const&);
template MAPNIK_DECL void fill(image_gray32s &, int64_t const&);
template MAPNIK_DECL void fill(image_gray32s &, uint32_t const&);
template MAPNIK_DECL void fill(image_gray32s &, int32_t const&);
template MAPNIK_DECL void fill(image_gray32s &, uint16_t const&);
template MAPNIK_DECL void fill(image_gray32s &, int16_t const&);
template MAPNIK_DECL void fill(image_gray32s &, uint8_t const&);
template MAPNIK_DECL void fill(image_gray32s &, int8_t const&);
template MAPNIK_DECL void fill(image_gray32s &, float const&);
template MAPNIK_DECL void fill(image_gray32s &, double const&);

template <typename T>
MAPNIK_DECL void fill (image_gray32f & data, T const& val)
{
    detail::visitor_fill<T> visitor(val);
    return visitor(data);
}

template MAPNIK_DECL void fill(image_gray32f &, color const&);
template MAPNIK_DECL void fill(image_gray32f &, uint64_t const&);
template MAPNIK_DECL void fill(image_gray32f &, int64_t const&);
template MAPNIK_DECL void fill(image_gray32f &, uint32_t const&);
template MAPNIK_DECL void fill(image_gray32f &, int32_t const&);
template MAPNIK_DECL void fill(image_gray32f &, uint16_t const&);
template MAPNIK_DECL void fill(image_gray32f &, int16_t const&);
template MAPNIK_DECL void fill(image_gray32f &, uint8_t const&);
template MAPNIK_DECL void fill(image_gray32f &, int8_t const&);
template MAPNIK_DECL void fill(image_gray32f &, float const&);
template MAPNIK_DECL void fill(image_gray32f &, double const&);

template <typename T>
MAPNIK_DECL void fill (image_gray64 & data, T const& val)
{
    detail::visitor_fill<T> visitor(val);
    return visitor(data);
}

template MAPNIK_DECL void fill(image_gray64 &, color const&);
template MAPNIK_DECL void fill(image_gray64 &, uint64_t const&);
template MAPNIK_DECL void fill(image_gray64 &, int64_t const&);
template MAPNIK_DECL void fill(image_gray64 &, uint32_t const&);
template MAPNIK_DECL void fill(image_gray64 &, int32_t const&);
template MAPNIK_DECL void fill(image_gray64 &, uint16_t const&);
template MAPNIK_DECL void fill(image_gray64 &, int16_t const&);
template MAPNIK_DECL void fill(image_gray64 &, uint8_t const&);
template MAPNIK_DECL void fill(image_gray64 &, int8_t const&);
template MAPNIK_DECL void fill(image_gray64 &, float const&);
template MAPNIK_DECL void fill(image_gray64 &, double const&);

template <typename T>
MAPNIK_DECL void fill (image_gray64s & data, T const& val)
{
    detail::visitor_fill<T> visitor(val);
    return visitor(data);
}

template MAPNIK_DECL void fill(image_gray64s &, color const&);
template MAPNIK_DECL void fill(image_gray64s &, uint64_t const&);
template MAPNIK_DECL void fill(image_gray64s &, int64_t const&);
template MAPNIK_DECL void fill(image_gray64s &, uint32_t const&);
template MAPNIK_DECL void fill(image_gray64s &, int32_t const&);
template MAPNIK_DECL void fill(image_gray64s &, uint16_t const&);
template MAPNIK_DECL void fill(image_gray64s &, int16_t const&);
template MAPNIK_DECL void fill(image_gray64s &, uint8_t const&);
template MAPNIK_DECL void fill(image_gray64s &, int8_t const&);
template MAPNIK_DECL void fill(image_gray64s &, float const&);
template MAPNIK_DECL void fill(image_gray64s &, double const&);

template <typename T>
MAPNIK_DECL void fill (image_gray64f & data, T const& val)
{
    detail::visitor_fill<T> visitor(val);
    return visitor(data);
}

template MAPNIK_DECL void fill(image_gray64f &, color const&);
template MAPNIK_DECL void fill(image_gray64f &, uint64_t const&);
template MAPNIK_DECL void fill(image_gray64f &, int64_t const&);
template MAPNIK_DECL void fill(image_gray64f &, uint32_t const&);
template MAPNIK_DECL void fill(image_gray64f &, int32_t const&);
template MAPNIK_DECL void fill(image_gray64f &, uint16_t const&);
template MAPNIK_DECL void fill(image_gray64f &, int16_t const&);
template MAPNIK_DECL void fill(image_gray64f &, uint8_t const&);
template MAPNIK_DECL void fill(image_gray64f &, int8_t const&);
template MAPNIK_DECL void fill(image_gray64f &, float const&);
template MAPNIK_DECL void fill(image_gray64f &, double const&);

namespace detail
{

struct visitor_composite_pixel
{
    // Obviously c variable would only work for rgba8 currently, but didn't want to
    // make this a template class until new rgba types exist.
    visitor_composite_pixel(composite_mode_e comp_op, std::size_t x, std::size_t y, unsigned c, unsigned cover, double opacity)
        :   opacity_(clamp(opacity, 0.0, 1.0)),
            comp_op_(comp_op),
            x_(x),
            y_(y),
            c_(c),
            cover_(cover) {}

    void operator() (image_rgba8 & data) const
    {
        using color_type = agg::rgba8;
        using value_type = color_type::value_type;
        using order_type = agg::order_rgba;
        using blender_type = agg::comp_op_adaptor_rgba<color_type,order_type>;

        if (mapnik::check_bounds(data, x_, y_))
        {
            image_rgba8::pixel_type rgba = data(x_,y_);
            // TODO use std::round for consistent rounding
            value_type ca = safe_cast<value_type>(((c_ >> 24u) & 0xff) * opacity_);
            value_type cb = (c_ >> 16u) & 0xff;
            value_type cg = (c_ >> 8u) & 0xff;
            value_type cr = (c_ & 0xff);
            blender_type::blend_pix(comp_op_, reinterpret_cast<value_type*>(&rgba), cr, cg, cb, ca, cover_);
            data(x_,y_) = rgba;
        }
    }

    template <typename T>
    void operator() (T &) const
    {
        throw std::runtime_error("Composite pixel is not supported for this data type");
    }

private:
    double const opacity_;
    composite_mode_e comp_op_;
    std::size_t const x_;
    std::size_t const y_;
    unsigned const c_;
    unsigned const cover_;

};

} // end detail ns

MAPNIK_DECL void composite_pixel(image_any & data, composite_mode_e comp_op, std::size_t x, std::size_t y, unsigned c, unsigned cover, double opacity )
{
    util::apply_visitor(detail::visitor_composite_pixel(comp_op, x, y, c, cover, opacity), data);
}

template <typename T>
MAPNIK_DECL void composite_pixel(T & data, composite_mode_e comp_op, std::size_t x, std::size_t y, unsigned c, unsigned cover, double opacity )
{
    detail::visitor_composite_pixel visitor(comp_op, x, y, c, cover, opacity);
    visitor(data);
}

template MAPNIK_DECL void composite_pixel(image_rgba8 &, composite_mode_e, std::size_t, std::size_t, unsigned, unsigned, double);
template MAPNIK_DECL void composite_pixel(image_gray8 &, composite_mode_e, std::size_t, std::size_t, unsigned, unsigned, double);
template MAPNIK_DECL void composite_pixel(image_gray8s &, composite_mode_e, std::size_t, std::size_t, unsigned, unsigned, double);
template MAPNIK_DECL void composite_pixel(image_gray16 &, composite_mode_e, std::size_t, std::size_t, unsigned, unsigned, double);
template MAPNIK_DECL void composite_pixel(image_gray16s &, composite_mode_e, std::size_t, std::size_t, unsigned, unsigned, double);
template MAPNIK_DECL void composite_pixel(image_gray32 &, composite_mode_e, std::size_t, std::size_t, unsigned, unsigned, double);
template MAPNIK_DECL void composite_pixel(image_gray32s &, composite_mode_e, std::size_t, std::size_t, unsigned, unsigned, double);
template MAPNIK_DECL void composite_pixel(image_gray32f &, composite_mode_e, std::size_t, std::size_t, unsigned, unsigned, double);
template MAPNIK_DECL void composite_pixel(image_gray64 &, composite_mode_e, std::size_t, std::size_t, unsigned, unsigned, double);
template MAPNIK_DECL void composite_pixel(image_gray64s &, composite_mode_e, std::size_t, std::size_t, unsigned, unsigned, double);
template MAPNIK_DECL void composite_pixel(image_gray64f &, composite_mode_e, std::size_t, std::size_t, unsigned, unsigned, double);

namespace detail {

template <typename T1>
struct visitor_set_pixel
{
    visitor_set_pixel(std::size_t x, std::size_t y, T1 const& val)
        : val_(val), x_(x), y_(y) {}

    template <typename T2>
    void operator() (T2 & data) const
    {
        using pixel_type = typename T2::pixel_type;
        if (check_bounds(data, x_, y_))
        {
            data(x_, y_) = safe_cast<pixel_type>(val_);
        }
    }

private:
    T1 const& val_;
    std::size_t const x_;
    std::size_t const y_;
};

template<>
struct visitor_set_pixel<color>
{
    visitor_set_pixel(std::size_t x, std::size_t y, color const& val)
        : val_(val), x_(x), y_(y) {}

    template <typename T2>
    void operator() (T2 & data) const
    {
        using pixel_type = typename T2::pixel_type;
        pixel_type val;
        if (data.get_premultiplied() && !val_.get_premultiplied())
        {
            color tmp(val_);
            tmp.premultiply();
            val = static_cast<pixel_type>(tmp.rgba());
        }
        else if (!data.get_premultiplied() && val_.get_premultiplied())
        {
            color tmp(val_);
            tmp.demultiply();
            val = static_cast<pixel_type>(tmp.rgba());
        }
        else
        {
            val = static_cast<pixel_type>(val_.rgba());
        }
        if (check_bounds(data, x_, y_))
        {
            data(x_, y_) = val;
        }
    }

private:
    color const& val_;
    std::size_t const x_;
    std::size_t const y_;
};

} // end detail ns

template <typename T>
MAPNIK_DECL void set_pixel (image_any & data, std::size_t x, std::size_t y, T const& val)
{
    util::apply_visitor(detail::visitor_set_pixel<T>(x, y, val), data);
}

template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, uint64_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, int64_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_any &, std::size_t, std::size_t, double const&);

template <typename T>
MAPNIK_DECL void set_pixel (image_rgba8 & data, std::size_t x, std::size_t y, T const& val)
{
    detail::visitor_set_pixel<T> visitor(x, y, val);
    visitor(data);
}

template MAPNIK_DECL void set_pixel(image_rgba8 &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_rgba8 &, std::size_t, std::size_t, uint64_t const&);
template MAPNIK_DECL void set_pixel(image_rgba8 &, std::size_t, std::size_t, int64_t const&);
template MAPNIK_DECL void set_pixel(image_rgba8 &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_rgba8 &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_rgba8 &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_rgba8 &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_rgba8 &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_rgba8 &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_rgba8 &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_rgba8 &, std::size_t, std::size_t, double const&);

template <typename T>
MAPNIK_DECL void set_pixel (image_gray8 & data, std::size_t x, std::size_t y, T const& val)
{
    detail::visitor_set_pixel<T> visitor(x, y, val);
    visitor(data);
}

template MAPNIK_DECL void set_pixel(image_gray8 &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_gray8 &, std::size_t, std::size_t, uint64_t const&);
template MAPNIK_DECL void set_pixel(image_gray8 &, std::size_t, std::size_t, int64_t const&);
template MAPNIK_DECL void set_pixel(image_gray8 &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_gray8 &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_gray8 &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_gray8 &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_gray8 &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_gray8 &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_gray8 &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_gray8 &, std::size_t, std::size_t, double const&);

template <typename T>
MAPNIK_DECL void set_pixel (image_gray8s & data, std::size_t x, std::size_t y, T const& val)
{
    detail::visitor_set_pixel<T> visitor(x, y, val);
    visitor(data);
}

template MAPNIK_DECL void set_pixel(image_gray8s &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_gray8s &, std::size_t, std::size_t, uint64_t const&);
template MAPNIK_DECL void set_pixel(image_gray8s &, std::size_t, std::size_t, int64_t const&);
template MAPNIK_DECL void set_pixel(image_gray8s &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_gray8s &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_gray8s &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_gray8s &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_gray8s &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_gray8s &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_gray8s &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_gray8s &, std::size_t, std::size_t, double const&);

template <typename T>
MAPNIK_DECL void set_pixel (image_gray16 & data, std::size_t x, std::size_t y, T const& val)
{
    detail::visitor_set_pixel<T> visitor(x, y, val);
    visitor(data);
}

template MAPNIK_DECL void set_pixel(image_gray16 &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_gray16 &, std::size_t, std::size_t, uint64_t const&);
template MAPNIK_DECL void set_pixel(image_gray16 &, std::size_t, std::size_t, int64_t const&);
template MAPNIK_DECL void set_pixel(image_gray16 &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_gray16 &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_gray16 &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_gray16 &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_gray16 &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_gray16 &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_gray16 &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_gray16 &, std::size_t, std::size_t, double const&);

template <typename T>
MAPNIK_DECL void set_pixel (image_gray16s & data, std::size_t x, std::size_t y, T const& val)
{
    detail::visitor_set_pixel<T> visitor(x, y, val);
    visitor(data);
}

template MAPNIK_DECL void set_pixel(image_gray16s &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_gray16s &, std::size_t, std::size_t, uint64_t const&);
template MAPNIK_DECL void set_pixel(image_gray16s &, std::size_t, std::size_t, int64_t const&);
template MAPNIK_DECL void set_pixel(image_gray16s &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_gray16s &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_gray16s &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_gray16s &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_gray16s &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_gray16s &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_gray16s &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_gray16s &, std::size_t, std::size_t, double const&);

template <typename T>
MAPNIK_DECL void set_pixel (image_gray32 & data, std::size_t x, std::size_t y, T const& val)
{
    detail::visitor_set_pixel<T> visitor(x, y, val);
    visitor(data);
}

template MAPNIK_DECL void set_pixel(image_gray32 &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_gray32 &, std::size_t, std::size_t, uint64_t const&);
template MAPNIK_DECL void set_pixel(image_gray32 &, std::size_t, std::size_t, int64_t const&);
template MAPNIK_DECL void set_pixel(image_gray32 &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_gray32 &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_gray32 &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_gray32 &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_gray32 &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_gray32 &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_gray32 &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_gray32 &, std::size_t, std::size_t, double const&);

template <typename T>
MAPNIK_DECL void set_pixel (image_gray32s & data, std::size_t x, std::size_t y, T const& val)
{
    detail::visitor_set_pixel<T> visitor(x, y, val);
    visitor(data);
}

template MAPNIK_DECL void set_pixel(image_gray32s &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_gray32s &, std::size_t, std::size_t, uint64_t const&);
template MAPNIK_DECL void set_pixel(image_gray32s &, std::size_t, std::size_t, int64_t const&);
template MAPNIK_DECL void set_pixel(image_gray32s &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_gray32s &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_gray32s &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_gray32s &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_gray32s &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_gray32s &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_gray32s &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_gray32s &, std::size_t, std::size_t, double const&);

template <typename T>
MAPNIK_DECL void set_pixel (image_gray32f & data, std::size_t x, std::size_t y, T const& val)
{
    detail::visitor_set_pixel<T> visitor(x, y, val);
    visitor(data);
}

template MAPNIK_DECL void set_pixel(image_gray32f &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_gray32f &, std::size_t, std::size_t, uint64_t const&);
template MAPNIK_DECL void set_pixel(image_gray32f &, std::size_t, std::size_t, int64_t const&);
template MAPNIK_DECL void set_pixel(image_gray32f &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_gray32f &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_gray32f &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_gray32f &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_gray32f &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_gray32f &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_gray32f &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_gray32f &, std::size_t, std::size_t, double const&);

template <typename T>
MAPNIK_DECL void set_pixel (image_gray64 & data, std::size_t x, std::size_t y, T const& val)
{
    detail::visitor_set_pixel<T> visitor(x, y, val);
    visitor(data);
}

template MAPNIK_DECL void set_pixel(image_gray64 &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_gray64 &, std::size_t, std::size_t, uint64_t const&);
template MAPNIK_DECL void set_pixel(image_gray64 &, std::size_t, std::size_t, int64_t const&);
template MAPNIK_DECL void set_pixel(image_gray64 &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_gray64 &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_gray64 &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_gray64 &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_gray64 &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_gray64 &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_gray64 &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_gray64 &, std::size_t, std::size_t, double const&);

template <typename T>
MAPNIK_DECL void set_pixel (image_gray64s & data, std::size_t x, std::size_t y, T const& val)
{
    detail::visitor_set_pixel<T> visitor(x, y, val);
    visitor(data);
}

template MAPNIK_DECL void set_pixel(image_gray64s &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_gray64s &, std::size_t, std::size_t, uint64_t const&);
template MAPNIK_DECL void set_pixel(image_gray64s &, std::size_t, std::size_t, int64_t const&);
template MAPNIK_DECL void set_pixel(image_gray64s &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_gray64s &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_gray64s &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_gray64s &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_gray64s &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_gray64s &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_gray64s &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_gray64s &, std::size_t, std::size_t, double const&);

template <typename T>
MAPNIK_DECL void set_pixel (image_gray64f & data, std::size_t x, std::size_t y, T const& val)
{
    detail::visitor_set_pixel<T> visitor(x, y, val);
    visitor(data);
}

template MAPNIK_DECL void set_pixel(image_gray64f &, std::size_t, std::size_t, color const&);
template MAPNIK_DECL void set_pixel(image_gray64f &, std::size_t, std::size_t, uint64_t const&);
template MAPNIK_DECL void set_pixel(image_gray64f &, std::size_t, std::size_t, int64_t const&);
template MAPNIK_DECL void set_pixel(image_gray64f &, std::size_t, std::size_t, uint32_t const&);
template MAPNIK_DECL void set_pixel(image_gray64f &, std::size_t, std::size_t, int32_t const&);
template MAPNIK_DECL void set_pixel(image_gray64f &, std::size_t, std::size_t, uint16_t const&);
template MAPNIK_DECL void set_pixel(image_gray64f &, std::size_t, std::size_t, int16_t const&);
template MAPNIK_DECL void set_pixel(image_gray64f &, std::size_t, std::size_t, uint8_t const&);
template MAPNIK_DECL void set_pixel(image_gray64f &, std::size_t, std::size_t, int8_t const&);
template MAPNIK_DECL void set_pixel(image_gray64f &, std::size_t, std::size_t, float const&);
template MAPNIK_DECL void set_pixel(image_gray64f &, std::size_t, std::size_t, double const&);

namespace detail {

template <typename T1>
struct visitor_get_pixel
{
    visitor_get_pixel(std::size_t x, std::size_t y)
        : x_(x), y_(y) {}

    template <typename T2>
    T1 operator() (T2 const& data) const
    {
        if (check_bounds(data, x_, y_))
        {
            return safe_cast<T1>(data(x_, y_));
        }
        else
        {
            throw std::runtime_error("Out of range for dataset with get pixel");
        }
    }

private:
    std::size_t const x_;
    std::size_t const y_;
};

template<>
struct visitor_get_pixel<color>
{
    visitor_get_pixel(std::size_t x, std::size_t y)
        : x_(x), y_(y) {}

    template <typename T2>
    color operator() (T2 const& data) const
    {
        if (check_bounds(data, x_, y_))
        {
            return color(static_cast<uint32_t>(data(x_, y_)), data.get_premultiplied());
        }
        else
        {
            throw std::runtime_error("Out of range for dataset with get pixel");
        }
    }

private:
    std::size_t const x_;
    std::size_t const y_;
};

} // end detail ns

template <typename T>
MAPNIK_DECL T get_pixel (image_any const& data, std::size_t x, std::size_t y)
{
    return util::apply_visitor(detail::visitor_get_pixel<T>(x, y), data);
}

template MAPNIK_DECL color get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_any const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_any const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_view_any const& data, std::size_t x, std::size_t y)
{
    return util::apply_visitor(detail::visitor_get_pixel<T>(x, y), data);
}

template MAPNIK_DECL color get_pixel(image_view_any const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_view_any const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_view_any const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_view_any const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_view_any const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_view_any const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_view_any const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_view_any const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_view_any const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_view_any const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_view_any const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_rgba8 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_rgba8 const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_gray8 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_gray8 const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_gray8s const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_gray8s const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_gray16 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_gray16 const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_gray16s const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_gray16s const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_gray32 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_gray32 const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_gray32s const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_gray32s const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_gray32f const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_gray32f const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_gray64 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_gray64 const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_gray64s const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_gray64s const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_gray64f const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_gray64f const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_view_rgba8 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_view_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_view_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_view_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_view_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_view_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_view_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_view_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_view_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_view_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_view_rgba8 const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_view_rgba8 const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_view_gray8 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_view_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_view_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_view_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_view_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_view_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_view_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_view_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_view_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_view_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_view_gray8 const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_view_gray8 const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_view_gray8s const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_view_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_view_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_view_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_view_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_view_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_view_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_view_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_view_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_view_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_view_gray8s const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_view_gray8s const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_view_gray16 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_view_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_view_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_view_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_view_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_view_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_view_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_view_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_view_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_view_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_view_gray16 const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_view_gray16 const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_view_gray16s const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_view_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_view_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_view_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_view_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_view_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_view_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_view_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_view_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_view_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_view_gray16s const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_view_gray16s const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_view_gray32 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_view_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_view_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_view_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_view_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_view_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_view_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_view_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_view_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_view_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_view_gray32 const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_view_gray32 const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_view_gray32s const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_view_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_view_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_view_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_view_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_view_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_view_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_view_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_view_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_view_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_view_gray32s const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_view_gray32s const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_view_gray32f const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_view_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_view_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_view_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_view_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_view_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_view_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_view_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_view_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_view_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_view_gray32f const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_view_gray32f const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_view_gray64 const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_view_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_view_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_view_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_view_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_view_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_view_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_view_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_view_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_view_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_view_gray64 const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_view_gray64 const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_view_gray64s const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_view_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_view_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_view_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_view_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_view_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_view_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_view_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_view_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_view_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_view_gray64s const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_view_gray64s const&, std::size_t, std::size_t);

template <typename T>
MAPNIK_DECL T get_pixel (image_view_gray64f const& data, std::size_t x, std::size_t y)
{
    detail::visitor_get_pixel<T> visitor(x, y);
    return visitor(data);
}

template MAPNIK_DECL color get_pixel(image_view_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint64_t get_pixel(image_view_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL int64_t get_pixel(image_view_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint32_t get_pixel(image_view_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL int32_t get_pixel(image_view_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint16_t get_pixel(image_view_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL int16_t get_pixel(image_view_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL uint8_t get_pixel(image_view_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL int8_t get_pixel(image_view_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL float get_pixel(image_view_gray64f const&, std::size_t, std::size_t);
template MAPNIK_DECL double get_pixel(image_view_gray64f const&, std::size_t, std::size_t);

namespace detail
{

template <typename Out>
struct visitor_view_to_stream
{
    visitor_view_to_stream(Out & os)
        : os_(os) {}

    template <typename T>
    void operator() (T const& view) const
    {
        for (std::size_t i=0;i<view.height();i++)
        {
            os_.write(reinterpret_cast<const char*>(view.get_row(i)),
                      safe_cast<std::streamsize>(view.row_size()));
        }
    }

private:
    Out & os_;
};

} // end detail ns

template <typename Out>
void view_to_stream (image_view_any const& view, Out & os)
{
    util::apply_visitor(detail::visitor_view_to_stream<Out>(os), view);
}

template MAPNIK_DECL void view_to_stream(image_view_any const& view, std::ostringstream & os);

namespace detail
{

struct visitor_create_view
{
    visitor_create_view(std::size_t x, std::size_t y, std::size_t w, std::size_t h)
        : x_(x), y_(y), w_(w), h_(h) {}

    image_view_any operator() (image_null const&) const
    {
        throw std::runtime_error("Can not make a view from a null image");
    }

    template <typename T>
    image_view_any operator() (T const& data) const
    {
        image_view<T> view(x_,y_,w_,h_,data);
        return image_view_any(view);
    }
private:
    std::size_t const x_;
    std::size_t const y_;
    std::size_t const w_;
    std::size_t const h_;
};

} // end detail ns

MAPNIK_DECL image_view_any create_view(image_any const& data, std::size_t x, std::size_t y, std::size_t w, std::size_t h)
{
    return util::apply_visitor(detail::visitor_create_view(x, y, w, h), data);
}

template <typename T>
MAPNIK_DECL std::size_t compare(T const& im1, T const& im2, double threshold, bool)
{
    using pixel_type = typename T::pixel_type;
    if (im1.width() != im2.width() || im1.height() != im2.height())
    {
        return im1.width() * im1.height();
    }
    std::size_t difference = 0;
    for (std::size_t y = 0; y < im1.height(); ++y)
    {
        const pixel_type * row_from = im1.get_row(y);
        const pixel_type * row_from2 = im2.get_row(y);
        for (std::size_t x = 0; x < im1.width(); ++x)
        {
            double d = std::abs(static_cast<double>(row_from[x]) - static_cast<double>(row_from2[x]));
            if (d > threshold)
            {
                ++difference;
            }
        }
    }
    return difference;
}

template MAPNIK_DECL std::size_t compare(image_gray8 const&, image_gray8 const&, double, bool);
template MAPNIK_DECL std::size_t compare(image_gray8s const&, image_gray8s const&, double, bool);
template MAPNIK_DECL std::size_t compare(image_gray16 const&, image_gray16 const&, double, bool);
template MAPNIK_DECL std::size_t compare(image_gray16s const&, image_gray16s const&, double, bool);
template MAPNIK_DECL std::size_t compare(image_gray32 const&, image_gray32 const&, double, bool);
template MAPNIK_DECL std::size_t compare(image_gray32s const&, image_gray32s const&, double, bool);
template MAPNIK_DECL std::size_t compare(image_gray32f const&, image_gray32f const&, double, bool);
template MAPNIK_DECL std::size_t compare(image_gray64 const&, image_gray64 const&, double, bool);
template MAPNIK_DECL std::size_t compare(image_gray64s const&, image_gray64s const&, double, bool);
template MAPNIK_DECL std::size_t compare(image_gray64f const&, image_gray64f const&, double, bool);

template <>
MAPNIK_DECL std::size_t compare<image_null>(image_null const&, image_null const&, double, bool)
{
    return 0;
}

template <>
MAPNIK_DECL std::size_t compare<image_rgba8>(image_rgba8 const& im1, image_rgba8 const& im2, double threshold, bool alpha)
{
    using pixel_type = image_rgba8::pixel_type;
    if (im1.width() != im2.width() || im1.height() != im2.height())
    {
        return im1.width() * im1.height();
    }
    unsigned difference = 0;
#ifdef SSE_MATH
    __m128i true_set = _mm_set1_epi8(0xff);
    __m128i one = _mm_set1_epi32(1);
    __m128i sum = _mm_setzero_si128();
    uint32_t alphamask = 0xffffffff;
    if (!alpha)
    {
        alphamask = 0xffffff;
    }
    __m128i mask = _mm_set1_epi32(alphamask);
    if (threshold == 0.0)
    {
        for (unsigned int y = 0; y < im1.height(); ++y)
        {
            const pixel_type * row_from = im1.get_row(y);
            const pixel_type * row_from2 = im2.get_row(y);
            int x = 0;
            for (; x < ROUND_DOWN(im1.width(),4); x +=4 )
            {
                __m128i rgba = _mm_loadu_si128((__m128i*)(row_from + x));
                __m128i rgba2 = _mm_loadu_si128((__m128i*)(row_from2 + x));
                rgba = _mm_and_si128(rgba, mask);
                rgba2 = _mm_and_si128(rgba2, mask);
                __m128i eq = _mm_cmpeq_epi8(rgba,rgba2);
                __m128i comp2 = _mm_cmpeq_epi32(eq, true_set);
                sum = _mm_add_epi32(sum, _mm_andnot_si128(comp2, one));
            }
            for (; x < im1.width(); ++x)
            {
                if ((row_from[x] & alphamask) != (row_from2[x] & alphamask))
                {
                    ++difference;
                }
            }
        }
        m128_int diff_sum;
        diff_sum.v = sum;
        difference += diff_sum.u32[0];
        difference += diff_sum.u32[1];
        difference += diff_sum.u32[2];
        difference += diff_sum.u32[3];
    }
    else
    {
        uint8_t thres = static_cast<uint8_t>(threshold);
        __m128i m_thres = _mm_set1_epi8(thres);
        for (unsigned int y = 0; y < im1.height(); ++y)
        {
            const pixel_type * row_from = im1.get_row(y);
            const pixel_type * row_from2 = im2.get_row(y);
            int x = 0;
            for (; x < ROUND_DOWN(im1.width(),4); x +=4 )
            {
                __m128i rgba = _mm_loadu_si128((__m128i*)(row_from + x));
                __m128i rgba2 = _mm_loadu_si128((__m128i*)(row_from2 + x));
                rgba = _mm_and_si128(rgba, mask);
                rgba2 = _mm_and_si128(rgba2, mask);
                __m128i abs = _mm_absdiff_epu8(rgba, rgba2);
                __m128i compare = _mm_cmpgt_epu8(abs, m_thres);
                __m128i comp2 = _mm_cmpeq_epi32(compare, _mm_setzero_si128());
                sum = _mm_add_epi32(sum, _mm_andnot_si128(comp2, one));
            }
            for (; x < im1.width(); ++x)
            {
                unsigned rgba = row_from[x];
                unsigned rgba2 = row_from2[x];
                unsigned r = rgba & 0xff;
                unsigned g = (rgba >> 8u) & 0xff;
                unsigned b = (rgba >> 16u) & 0xff;
                unsigned r2 = rgba2 & 0xff;
                unsigned g2 = (rgba2 >> 8u) & 0xff;
                unsigned b2 = (rgba2 >> 16u) & 0xff;
                if (std::abs(static_cast<int>(r - r2)) > static_cast<int>(threshold) ||
                    std::abs(static_cast<int>(g - g2)) > static_cast<int>(threshold) ||
                    std::abs(static_cast<int>(b - b2)) > static_cast<int>(threshold)) {
                    ++difference;
                    continue;
                }
                if (alpha) {
                    unsigned a = (rgba >> 24u) & 0xff;
                    unsigned a2 = (rgba2 >> 24u) & 0xff;
                    if (std::abs(static_cast<int>(a - a2)) > static_cast<int>(threshold)) {
                        ++difference;
                        continue;
                    }
                }
            }
        }
        m128_int diff_sum;
        diff_sum.v = sum;
        difference += diff_sum.u32[0];
        difference += diff_sum.u32[1];
        difference += diff_sum.u32[2];
        difference += diff_sum.u32[3];
    }
#else
    for (unsigned int y = 0; y < im1.height(); ++y)
    {
        const pixel_type * row_from = im1.get_row(y);
        const pixel_type * row_from2 = im2.get_row(y);
        for (unsigned int x = 0; x < im1.width(); ++x)
        {
            unsigned rgba = row_from[x];
            unsigned rgba2 = row_from2[x];
            unsigned r = rgba & 0xff;
            unsigned g = (rgba >> 8u) & 0xff;
            unsigned b = (rgba >> 16u) & 0xff;
            unsigned r2 = rgba2 & 0xff;
            unsigned g2 = (rgba2 >> 8u) & 0xff;
            unsigned b2 = (rgba2 >> 16u) & 0xff;
            if (std::abs(static_cast<int>(r - r2)) > static_cast<int>(threshold) ||
                std::abs(static_cast<int>(g - g2)) > static_cast<int>(threshold) ||
                std::abs(static_cast<int>(b - b2)) > static_cast<int>(threshold)) {
                ++difference;
                continue;
            }
            if (alpha) {
                unsigned a = (rgba >> 24u) & 0xff;
                unsigned a2 = (rgba2 >> 24u) & 0xff;
                if (std::abs(static_cast<int>(a - a2)) > static_cast<int>(threshold)) {
                    ++difference;
                    continue;
                }
            }
        }
    }
#endif
    return difference;
}

namespace detail
{

struct visitor_compare
{
    visitor_compare(image_any const& im2, double threshold, bool alpha)
        : im2_(im2),
          threshold_(threshold),
          alpha_(alpha) {}

    template <typename T>
    std::size_t operator() (T const & im1) const
    {
        if (!im2_.is<T>())
        {
            return im1.width() * im1.height();
        }
        return mapnik::compare<T>(im1, util::get<T>(im2_), threshold_, alpha_);
    }

private:
    image_any const& im2_;
    double const threshold_;
    bool const alpha_;
};

} // end detail ns

template <>
MAPNIK_DECL std::size_t compare<image_any>(image_any const& im1, image_any const& im2, double threshold, bool alpha)
{
    return util::apply_visitor(detail::visitor_compare(im2, threshold, alpha), im1);
}

} // end ns
