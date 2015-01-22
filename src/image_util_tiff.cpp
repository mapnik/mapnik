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

#include <mapnik/image_util.hpp>
#include <mapnik/image_util_tiff.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/util/conversions.hpp>

// boost
#include <boost/tokenizer.hpp>

// stl
#include <string>

namespace mapnik
{

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

tiff_saver::tiff_saver(std::ostream & stream, std::string const& t):
    stream_(stream), t_(t) {}
template<>
void tiff_saver::operator()<image_null> (image_null const& image) const
{
    throw ImageWriterException("null images not supported");
}

template <typename T>
void tiff_saver::operator() (T const& image) const
{
#if defined(HAVE_TIFF)
    tiff_config opts;
    handle_tiff_options(t_, opts);
    save_as_tiff(stream_, image, opts);
#else
    throw ImageWriterException("tiff output is not enabled in your build of Mapnik");
#endif
}

template void tiff_saver::operator() (image_rgba8 const& image) const;
template void tiff_saver::operator() (image_gray8 const& image) const;
template void tiff_saver::operator() (image_gray16 const& image) const;
template void tiff_saver::operator() (image_gray32f const& image) const;
template void tiff_saver::operator() (image_view_rgba8 const& image) const;
template void tiff_saver::operator() (image_view_gray8 const& image) const;
template void tiff_saver::operator() (image_view_gray16 const& image) const;
template void tiff_saver::operator() (image_view_gray32f const& image) const;

} // end ns
