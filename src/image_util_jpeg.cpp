/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#if defined(HAVE_JPEG)
#include <mapnik/jpeg_io.hpp>
#endif

#include <mapnik/image_util.hpp>
#include <mapnik/image_util_jpeg.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/image_options.hpp>
#include <mapnik/util/conversions.hpp>

// stl
#include <string>

namespace mapnik
{

jpeg_saver::jpeg_saver(std::ostream & stream, std::string const& t):
    stream_(stream), t_(t) {}

template <typename T>
void process_rgba8_jpeg(T const& image, std::string const& type, std::ostream & stream)
{
#if defined(HAVE_JPEG)
    int quality = 85;
    if (type != "jpeg")
    {
        for (auto const& kv : parse_image_options(type))
        {
            auto const& key = kv.first;
            auto const& val = kv.second;

            if ( key == "jpeg" ) continue;
            else if ( key == "quality")
            {
                if (val && ! (*val).empty())
                {
                    if (!mapnik::util::string2int(*val, quality) || quality < 0 || quality > 100)
                    {
                        throw image_writer_exception("invalid jpeg quality: '" + *val + "'");
                    }
                }
            }
        }
    }
    save_as_jpeg(stream, quality, image);
#else
    throw image_writer_exception("jpeg output is not enabled in your build of Mapnik");
#endif
}

template<>
void jpeg_saver::operator()<image_rgba8> (image_rgba8 const& image) const
{
    process_rgba8_jpeg(image, t_, stream_);
}

template<>
void jpeg_saver::operator()<image_view_rgba8> (image_view_rgba8 const& image) const
{
    process_rgba8_jpeg(image, t_, stream_);
}

template<>
void jpeg_saver::operator()<image_null> (image_null const& image) const
{
    throw image_writer_exception("Can not save a null image to jpeg");
}

template<>
void jpeg_saver::operator()<image_view_null> (image_view_null const& image) const
{
    throw image_writer_exception("Can not save a null image to jpeg");
}

template <typename T>
void jpeg_saver::operator() (T const& image) const
{
    throw image_writer_exception("Mapnik does not support jpeg grayscale images");
}

template void jpeg_saver::operator()<image_rgba8> (image_rgba8 const& image) const;
template void jpeg_saver::operator()<image_gray8> (image_gray8 const& image) const;
template void jpeg_saver::operator()<image_gray8s> (image_gray8s const& image) const;
template void jpeg_saver::operator()<image_gray16> (image_gray16 const& image) const;
template void jpeg_saver::operator()<image_gray16s> (image_gray16s const& image) const;
template void jpeg_saver::operator()<image_gray32> (image_gray32 const& image) const;
template void jpeg_saver::operator()<image_gray32s> (image_gray32s const& image) const;
template void jpeg_saver::operator()<image_gray32f> (image_gray32f const& image) const;
template void jpeg_saver::operator()<image_gray64> (image_gray64 const& image) const;
template void jpeg_saver::operator()<image_gray64s> (image_gray64s const& image) const;
template void jpeg_saver::operator()<image_gray64f> (image_gray64f const& image) const;
template void jpeg_saver::operator()<image_view_rgba8> (image_view_rgba8 const& image) const;
template void jpeg_saver::operator()<image_view_gray8> (image_view_gray8 const& image) const;
template void jpeg_saver::operator()<image_view_gray8s> (image_view_gray8s const& image) const;
template void jpeg_saver::operator()<image_view_gray16> (image_view_gray16 const& image) const;
template void jpeg_saver::operator()<image_view_gray16s> (image_view_gray16s const& image) const;
template void jpeg_saver::operator()<image_view_gray32> (image_view_gray32 const& image) const;
template void jpeg_saver::operator()<image_view_gray32s> (image_view_gray32s const& image) const;
template void jpeg_saver::operator()<image_view_gray32f> (image_view_gray32f const& image) const;
template void jpeg_saver::operator()<image_view_gray64> (image_view_gray64 const& image) const;
template void jpeg_saver::operator()<image_view_gray64s> (image_view_gray64s const& image) const;
template void jpeg_saver::operator()<image_view_gray64f> (image_view_gray64f const& image) const;

} // end ns
