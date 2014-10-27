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

#ifndef MAPNIK_IMAGE_UTIL_HPP
#define MAPNIK_IMAGE_UTIL_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/image_view.hpp>

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/optional.hpp>
#pragma GCC diagnostic pop

// stl
#include <string>
#include <cmath>
#include <exception>

namespace mapnik {

// fwd declares
class Map;
class rgba_palette;
class image_32;

class ImageWriterException : public std::exception
{
private:
    std::string message_;
public:
    ImageWriterException(std::string const& message)
        : message_(message) {}

    ~ImageWriterException() throw() {}

    virtual const char* what() const throw()
    {
        return message_.c_str();
    }
};

#if defined(HAVE_CAIRO)
MAPNIK_DECL void save_to_cairo_file(mapnik::Map const& map,
                                    std::string const& filename,
                                    double scale_factor=1.0,
                                    double scale_denominator=0.0);
MAPNIK_DECL void save_to_cairo_file(mapnik::Map const& map,
                                    std::string const& filename,
                                    std::string const& type,
                                    double scale_factor=1.0,
                                    double scale_denominator=0.0);
#endif

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

template <typename T>
void save_as_png(T const& image,
                 std::string const& filename,
                 rgba_palette const& palette);

#if defined(HAVE_JPEG)
template <typename T>
void save_as_jpeg(std::string const& filename,
                  int quality,
                  T const& image);
#endif

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



/////////// save_to_file //////////////////////////////////////////////////

MAPNIK_DECL void save_to_file(image_32 const& image,
                              std::string const& file);

MAPNIK_DECL void save_to_file (image_32 const& image,
                               std::string const& file,
                               std::string const& type);

MAPNIK_DECL void save_to_file (image_32 const& image,
                               std::string const& file,
                               std::string const& type,
                               rgba_palette const& palette);

///////////////////////////////////////////////////////////////////////////


MAPNIK_DECL std::string save_to_string(image_32 const& image,
                                       std::string const& type);

MAPNIK_DECL std::string save_to_string(image_32 const& image,
                                       std::string const& type,
                                       rgba_palette const& palette);

///////////////////////////////////////////////////////////////////////////

MAPNIK_DECL void save_to_stream(image_32 const& image,
                                std::ostream & stream,
                                std::string const& type,
                                rgba_palette const& palette);

MAPNIK_DECL void save_to_stream(image_32 const& image,
                                std::ostream & stream,
                                std::string const& type);

///////////////////////////////////////////////////////////////////////////

extern template MAPNIK_DECL void save_to_file<image_data_32>(image_data_32 const&,
                                                      std::string const&,
                                                      std::string const&,
                                                      rgba_palette const&);

extern template MAPNIK_DECL void save_to_file<image_data_32>(image_data_32 const&,
                                                      std::string const&,
                                                      std::string const&);

extern template MAPNIK_DECL void save_to_file<image_data_32>(image_data_32 const&,
                                                      std::string const&,
                                                      rgba_palette const&);

extern template MAPNIK_DECL void save_to_file<image_data_32>(image_data_32 const&,
                                                      std::string const&);


extern template MAPNIK_DECL void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                    std::string const&,
                                                                    std::string const&,
                                                                    rgba_palette const&);

extern template MAPNIK_DECL void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                    std::string const&,
                                                                    std::string const&);

extern template MAPNIK_DECL void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                    std::string const&,
                                                                    rgba_palette const&);

extern template MAPNIK_DECL void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                    std::string const&);

extern template MAPNIK_DECL std::string save_to_string<image_data_32>(image_data_32 const&,
                                                               std::string const&);

extern template MAPNIK_DECL std::string save_to_string<image_data_32>(image_data_32 const&,
                                                               std::string const&,
                                                               rgba_palette const&);

extern template MAPNIK_DECL std::string save_to_string<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                             std::string const&);

extern template MAPNIK_DECL std::string save_to_string<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                             std::string const&,
                                                                             rgba_palette const&);
#ifdef _MSC_VER

template MAPNIK_DECL void save_to_stream<image_data_32>(
    image_data_32 const& image,
    std::ostream & stream,
    std::string const& type,
    rgba_palette const& palette
);

template MAPNIK_DECL void save_to_stream<image_data_32>(
    image_data_32 const& image,
    std::ostream & stream,
    std::string const& type
);

template MAPNIK_DECL void save_to_stream<image_view<image_data_32> > (
    image_view<image_data_32> const& image,
    std::ostream & stream,
    std::string const& type,
    rgba_palette const& palette
);

template MAPNIK_DECL void save_to_stream<image_view<image_data_32> > (
    image_view<image_data_32> const& image,
    std::ostream & stream,
    std::string const& type
);
#endif

}

#endif // MAPNIK_IMAGE_UTIL_HPP
