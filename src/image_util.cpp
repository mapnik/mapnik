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

} // end detail ns

template <typename T>
bool is_solid(T const& image)
{
    return util::apply_visitor(detail::is_solid_visitor(), image);
}

// Temporary until image_data_rgba8 is removed from passing
template <>
bool is_solid<image_data_rgba8>(image_data_rgba8 const& image)
{
    detail::is_solid_visitor visitor;
    return visitor(image);
}

// Temporary until image_view_rgba8 is removed from passing
template <>
bool is_solid<image_view_rgba8>(image_view_rgba8 const& image)
{
    detail::is_solid_visitor visitor;
    return visitor(image);
}

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

}
