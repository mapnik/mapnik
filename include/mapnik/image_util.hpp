/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_IMAGE_UTIL_HPP
#define MAPNIK_IMAGE_UTIL_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/pixel_types.hpp>
#include <mapnik/image_compositing.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/optional.hpp>
#pragma GCC diagnostic pop

// stl
#include <string>
#include <exception>

namespace mapnik {

// fwd declares
class rgba_palette;
struct image_any;
template <typename T> class image;
struct image_view_any;
template <typename T> class image_view;
class color;

class image_writer_exception : public std::exception
{
private:
    std::string message_;
public:
    image_writer_exception(std::string const& message)
        : message_(message) {}

    ~image_writer_exception(){}

    virtual const char* what() const noexcept
    {
        return message_.c_str();
    }
};

template <typename T>
MAPNIK_DECL void save_to_file(T const& image,
                              std::string const& filename,
                              std::string const& type);

template <typename T>
MAPNIK_DECL void save_to_file(T const& image,
                              std::string const& filename,
                              std::string const& type,
                              rgba_palette const& palette);

// guess type from file extension
template <typename T>
MAPNIK_DECL void save_to_file(T const& image,
                              std::string const& filename);

template <typename T>
MAPNIK_DECL void save_to_file(T const& image,
                              std::string const& filename,
                              rgba_palette const& palette);

template <typename T>
MAPNIK_DECL std::string save_to_string(T const& image,
                                       std::string const& type);

template <typename T>
MAPNIK_DECL std::string save_to_string(T const& image,
                                       std::string const& type,
                                       rgba_palette const& palette);

template <typename T>
MAPNIK_DECL void save_to_stream
(
    T const& image,
    std::ostream & stream,
    std::string const& type,
    rgba_palette const& palette
);

template <typename T>
MAPNIK_DECL void save_to_stream
(
    T const& image,
    std::ostream & stream,
    std::string const& type
);

// PREMULTIPLY ALPHA
MAPNIK_DECL bool premultiply_alpha(image_any & image);

template <typename T>
MAPNIK_DECL bool premultiply_alpha(T & image);

// DEMULTIPLY ALPHA
MAPNIK_DECL bool demultiply_alpha(image_any & image);

template <typename T>
MAPNIK_DECL bool demultiply_alpha(T & image);

// SET PREMULTIPLIED ALPHA
MAPNIK_DECL void set_premultiplied_alpha(image_any & image, bool status);

template <typename T>
MAPNIK_DECL void set_premultiplied_alpha(T & image, bool status);

// IS SOLID
MAPNIK_DECL bool is_solid (image_any const& image);
MAPNIK_DECL bool is_solid (image_view_any const& image);

template <typename T>
MAPNIK_DECL bool is_solid (T const& image);

// APPLY OPACITY
MAPNIK_DECL void apply_opacity (image_any & image, float opacity);

template <typename T>
MAPNIK_DECL void apply_opacity (T & image, float opacity);

// SET GRAYSCALE TO ALPHA
MAPNIK_DECL void set_grayscale_to_alpha (image_any & image);
MAPNIK_DECL void set_grayscale_to_alpha (image_any & image, color const& c);

template <typename T>
MAPNIK_DECL void set_grayscale_to_alpha (T & image);

template <typename T>
MAPNIK_DECL void set_grayscale_to_alpha (T & image, color const& c);

// SET COLOR TO ALPHA
MAPNIK_DECL void set_color_to_alpha (image_any & image, color const& c);

template <typename T>
MAPNIK_DECL void set_color_to_alpha (T & image, color const& c);

// FILL
template <typename T>
MAPNIK_DECL void fill (image_any & data, T const&);

template <typename T>
MAPNIK_DECL void fill (image<rgba8_t> & data, T const&);

template <typename T>
MAPNIK_DECL void fill (image<gray8_t>  & data, T const&);

template <typename T>
MAPNIK_DECL void fill (image<gray8s_t>  & data, T const&);

template <typename T>
MAPNIK_DECL void fill (image<gray16_t>  & data, T const&);

template <typename T>
MAPNIK_DECL void fill (image<gray16s_t>  & data, T const&);

template <typename T>
MAPNIK_DECL void fill (image<gray32_t>  & data, T const&);

template <typename T>
MAPNIK_DECL void fill (image<gray32s_t>  & data, T const&);

template <typename T>
MAPNIK_DECL void fill (image<gray32f_t>  & data, T const&);

template <typename T>
MAPNIK_DECL void fill (image<gray64_t>  & data, T const&);

template <typename T>
MAPNIK_DECL void fill (image<gray64s_t>  & data, T const&);

template <typename T>
MAPNIK_DECL void fill (image<gray64f_t> & data, T const&);

// CHECK BOUNDS
template <typename T>
inline bool check_bounds (T const& data, std::size_t x, std::size_t y)
{
    return (x < data.width() && y < data.height());
}

// COMPOSITE_PIXEL
MAPNIK_DECL void composite_pixel(image_any & data, composite_mode_e comp_op, std::size_t x, std::size_t y, unsigned c, unsigned cover, double opacity );

template <typename T>
MAPNIK_DECL void composite_pixel(T & data, composite_mode_e comp_op, std::size_t x, std::size_t y, unsigned c, unsigned cover, double opacity );

// SET PIXEL
template <typename T>
MAPNIK_DECL void set_pixel(image_any & data, std::size_t x, std::size_t y, T const& val);

template <typename T>
MAPNIK_DECL void set_pixel(image<rgba8_t> & data, std::size_t x, std::size_t y, T const& val);

template <typename T>
MAPNIK_DECL void set_pixel(image<gray8_t> & data, std::size_t x, std::size_t y, T const& val);

template <typename T>
MAPNIK_DECL void set_pixel(image<gray8s_t> & data, std::size_t x, std::size_t y, T const& val);

template <typename T>
MAPNIK_DECL void set_pixel(image<gray16_t> & data, std::size_t x, std::size_t y, T const& val);

template <typename T>
MAPNIK_DECL void set_pixel(image<gray16s_t> & data, std::size_t x, std::size_t y, T const& val);

template <typename T>
MAPNIK_DECL void set_pixel(image<gray32_t> & data, std::size_t x, std::size_t y, T const& val);

template <typename T>
MAPNIK_DECL void set_pixel(image<gray32s_t> & data, std::size_t x, std::size_t y, T const& val);

template <typename T>
MAPNIK_DECL void set_pixel(image<gray32f_t> & data, std::size_t x, std::size_t y, T const& val);

template <typename T>
MAPNIK_DECL void set_pixel(image<gray64_t> & data, std::size_t x, std::size_t y, T const& val);

template <typename T>
MAPNIK_DECL void set_pixel(image<gray64s_t> & data, std::size_t x, std::size_t y, T const& val);

template <typename T>
MAPNIK_DECL void set_pixel(image<gray64f_t> & data, std::size_t x, std::size_t y, T const& val);

// GET PIXEL
template <typename T>
MAPNIK_DECL T get_pixel(image_any const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image_view_any const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image<rgba8_t> const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image<gray8_t> const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image<gray8s_t> const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image<gray16_t> const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image<gray16s_t> const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image<gray32_t> const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image<gray32s_t> const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image<gray32f_t> const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image<gray64_t> const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image<gray64s_t> const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image<gray64f_t> const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image_view<image<rgba8_t> > const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image_view<image<gray8_t> > const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image_view<image<gray8s_t> > const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image_view<image<gray16_t> > const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image_view<image<gray16s_t> > const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image_view<image<gray32_t> > const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image_view<image<gray32s_t> > const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image_view<image<gray32f_t> > const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image_view<image<gray64_t> > const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image_view<image<gray64s_t> > const& data, std::size_t x, std::size_t y);

template <typename T>
MAPNIK_DECL T get_pixel(image_view<image<gray64f_t> > const& data, std::size_t x, std::size_t y);

// VIEW TO OUTPUT STREAM
template <typename Out>
MAPNIK_DECL void view_to_stream (image_view_any const& view, Out & os);

// CREATE VIEW
MAPNIK_DECL image_view_any create_view (image_any const& data, std::size_t x, std::size_t y, std::size_t w, std::size_t h);

// COMPARE
template <typename T>
MAPNIK_DECL std::size_t compare(T const& im1, T const& im2, double threshold = 0.0, bool alpha = true);

inline bool is_png(std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".png"));
}

inline bool is_jpeg(std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".jpg")) ||
        boost::algorithm::iends_with(filename,std::string(".jpeg"));
}

inline bool is_tiff(std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".tif")) ||
        boost::algorithm::iends_with(filename,std::string(".tiff"));
}

inline bool is_pdf(std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".pdf"));
}

inline bool is_svg(std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".svg"));
}

inline bool is_ps(std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".ps"));
}

inline bool is_webp(std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".webp"));
}

inline boost::optional<std::string> type_from_filename(std::string const& filename)

{
    using result_type = boost::optional<std::string>;
    if (is_png(filename)) return result_type("png");
    if (is_jpeg(filename)) return result_type("jpeg");
    if (is_tiff(filename)) return result_type("tiff");
    if (is_pdf(filename)) return result_type("pdf");
    if (is_svg(filename)) return result_type("svg");
    if (is_ps(filename)) return result_type("ps");
    if (is_webp(filename)) return result_type("webp");
    return result_type();
}

inline std::string guess_type( std::string const& filename )
{
    std::string::size_type idx = filename.find_last_of(".");
    if ( idx != std::string::npos ) {
        return filename.substr( idx + 1 );
    }
    return "<unknown>";
}

// add 1-px border around image - useful for debugging alignment issues
template <typename T>
void add_border(T & image)
{
    for (unsigned  x = 0; x < image.width();++x)
    {
        image(x,0) = 0xff0000ff; // red
        image(x,image.height()-1) = 0xff00ff00; //green
    }
    for (unsigned y = 0; y < image.height();++y)
    {
        image(0,y) = 0xff00ffff; //yellow
        image(image.width()-1,y) = 0xffff0000; // blue
    }
}
}

#endif // MAPNIK_IMAGE_UTIL_HPP
