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
#include <mapnik/palette.hpp>

#ifdef _MSC_VER
#include <mapnik/graphics.hpp>
#endif

// boost
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>

// stl
#include <string>

namespace mapnik {

class Map;
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
                                    std::string const& type,
                                    double scale_factor=1.0);
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
void save_as_png(T const& image,
                 std::string const& filename,
                 rgba_palette const& palette);

#if defined(HAVE_JPEG)
template <typename T>
void save_as_jpeg(std::string const& filename,
                  int quality,
                  T const& image);
#endif

inline bool is_png (std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".png"));
}

inline bool is_jpeg (std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".jpg")) ||
        boost::algorithm::iends_with(filename,std::string(".jpeg"));
}

inline bool is_tiff (std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".tif")) ||
        boost::algorithm::iends_with(filename,std::string(".tiff"));
}

inline bool is_pdf (std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".pdf"));
}

inline bool is_svg (std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".svg"));
}

inline bool is_ps (std::string const& filename)
{
    return boost::algorithm::iends_with(filename,std::string(".ps"));
}

inline boost::optional<std::string> type_from_filename(std::string const& filename)

{
    typedef boost::optional<std::string> result_type;
    if (is_png(filename)) return result_type("png");
    if (is_jpeg(filename)) return result_type("jpeg");
    if (is_tiff(filename)) return result_type("tiff");
    if (is_pdf(filename)) return result_type("pdf");
    if (is_svg(filename)) return result_type("svg");
    if (is_ps(filename)) return result_type("ps");
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

template <typename T>
double distance(T x0,T y0,T x1,T y1)
{
    double dx = x1-x0;
    double dy = y1-y0;
    return std::sqrt(dx * dx + dy * dy);
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



/////////// save_to_file ////////////////////////////////////////////////
class image_32;

MAPNIK_DECL void save_to_file(image_32 const& image,
                              std::string const& file);

MAPNIK_DECL void save_to_file (image_32 const& image,
                               std::string const& file,
                               std::string const& type);

MAPNIK_DECL void save_to_file (image_32 const& image,
                               std::string const& file,
                               std::string const& type,
                               rgba_palette const& palette);

//////////////////////////////////////////////////////////////////////////


MAPNIK_DECL std::string save_to_string(image_32 const& image,
                                       std::string const& type);

MAPNIK_DECL std::string save_to_string(image_32 const& image,
                                       std::string const& type,
                                       rgba_palette const& palette);

///////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
template MAPNIK_DECL void save_to_file<image_data_32>(image_data_32 const&,
                                                      std::string const&,
                                                      std::string const&,
                                                      rgba_palette const&);

template MAPNIK_DECL void save_to_file<image_data_32>(image_data_32 const&,
                                                      std::string const&,
                                                      std::string const&);

template MAPNIK_DECL void save_to_file<image_data_32>(image_data_32 const&,
                                                      std::string const&,
                                                      rgba_palette const&);

template MAPNIK_DECL void save_to_file<image_data_32>(image_data_32 const&,
                                                      std::string const&);


template MAPNIK_DECL void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                    std::string const&,
                                                                    std::string const&,
                                                                    rgba_palette const&);

template MAPNIK_DECL void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                    std::string const&,
                                                                    std::string const&);

template MAPNIK_DECL void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                    std::string const&,
                                                                    rgba_palette const&);

template MAPNIK_DECL void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                    std::string const&);


template MAPNIK_DECL std::string save_to_string<image_data_32>(image_data_32 const&,
                                                               std::string const&,
                                                               rgba_palette const&);

template MAPNIK_DECL std::string save_to_string<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                             std::string const&);

template MAPNIK_DECL std::string save_to_string<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                             std::string const&,
                                                                             rgba_palette const&);
#endif

}

#endif // MAPNIK_IMAGE_UTIL_HPP
