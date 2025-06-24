/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/image.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/image_options.hpp>
#include <mapnik/util/conversions.hpp>

// stl
#include <string>

namespace mapnik {

#if defined(HAVE_TIFF)
void handle_tiff_options(std::string const& type, tiff_config& config)
{
    if (type == "tiff")
    {
        return;
    }
    if (type.length() > 4)
    {
        for (auto const& kv : parse_image_options(type))
        {
            auto const& key = kv.first;
            auto const& val = kv.second;
            if (key == "tiff")
                continue;
            else if (key == "compression")
            {
                if (val && !(*val).empty())
                {
                    if (*val == "deflate")
                    {
                        config.compression = COMPRESSION_DEFLATE;
                    }
                    else if (*val == "adobedeflate")
                    {
                        config.compression = COMPRESSION_ADOBE_DEFLATE;
                    }
                    else if (*val == "lzw")
                    {
                        config.compression = COMPRESSION_LZW;
                    }
                    else if (*val == "none")
                    {
                        config.compression = COMPRESSION_NONE;
                    }
                    else
                    {
                        throw image_writer_exception("invalid tiff compression: '" + *val + "'");
                    }
                }
            }
            else if (key == "method")
            {
                if (val && !(*val).empty())
                {
                    if (*val == "scanline")
                    {
                        config.method = TIFF_WRITE_SCANLINE;
                    }
                    else if (*val == "strip" || *val == "stripped")
                    {
                        config.method = TIFF_WRITE_STRIPPED;
                    }
                    else if (*val == "tiled")
                    {
                        config.method = TIFF_WRITE_TILED;
                    }
                    else
                    {
                        throw image_writer_exception("invalid tiff method: '" + *val + "'");
                    }
                }
            }
            else if (key == "zlevel")
            {
                if (val && !(*val).empty())
                {
                    if (!mapnik::util::string2int(*val, config.zlevel) || config.zlevel < 0 || config.zlevel > 9)
                    {
                        throw image_writer_exception("invalid tiff zlevel: '" + *val + "'");
                    }
                }
            }
            else if (key == "tile_height")
            {
                if (val && !(*val).empty())
                {
                    if (!mapnik::util::string2int(*val, config.tile_height) || config.tile_height < 0)
                    {
                        throw image_writer_exception("invalid tiff tile_height: '" + *val + "'");
                    }
                }
            }
            else if (key == "tile_width")
            {
                if (val && !(*val).empty())
                {
                    if (!mapnik::util::string2int(*val, config.tile_width) || config.tile_width < 0)
                    {
                        throw image_writer_exception("invalid tiff tile_width: '" + *val + "'");
                    }
                }
            }
            else if (key == "rows_per_strip")
            {
                if (val && !(*val).empty())
                {
                    if (!mapnik::util::string2int(*val, config.rows_per_strip) || config.rows_per_strip < 0)
                    {
                        throw image_writer_exception("invalid tiff rows_per_strip: '" + *val + "'");
                    }
                }
            }
            else
            {
                throw image_writer_exception("unhandled tiff option: " + key);
            }
        }
    }
}
#endif

tiff_saver::tiff_saver(std::ostream& stream, std::string const& t)
    : stream_(stream)
    , t_(t)
{}

template<>
void tiff_saver::operator()<image_null>(image_null const& image) const
{
    throw image_writer_exception("null images not supported");
}

template<>
void tiff_saver::operator()<image_view_null>(image_view_null const& image) const
{
    throw image_writer_exception("null image views not supported");
}

template<typename T>
void tiff_saver::operator()(T const& image) const
{
#if defined(HAVE_TIFF)
    tiff_config opts;
    handle_tiff_options(t_, opts);
    save_as_tiff(stream_, image, opts);
#else
    throw image_writer_exception("tiff output is not enabled in your build of Mapnik");
#endif
}

template void tiff_saver::operator()<image_rgba8>(image_rgba8 const& image) const;
template void tiff_saver::operator()<image_gray8>(image_gray8 const& image) const;
template void tiff_saver::operator()<image_gray8s>(image_gray8s const& image) const;
template void tiff_saver::operator()<image_gray16>(image_gray16 const& image) const;
template void tiff_saver::operator()<image_gray16s>(image_gray16s const& image) const;
template void tiff_saver::operator()<image_gray32>(image_gray32 const& image) const;
template void tiff_saver::operator()<image_gray32s>(image_gray32s const& image) const;
template void tiff_saver::operator()<image_gray32f>(image_gray32f const& image) const;
template void tiff_saver::operator()<image_gray64>(image_gray64 const& image) const;
template void tiff_saver::operator()<image_gray64s>(image_gray64s const& image) const;
template void tiff_saver::operator()<image_gray64f>(image_gray64f const& image) const;
template void tiff_saver::operator()<image_view_rgba8>(image_view_rgba8 const& image) const;
template void tiff_saver::operator()<image_view_gray8>(image_view_gray8 const& image) const;
template void tiff_saver::operator()<image_view_gray8s>(image_view_gray8s const& image) const;
template void tiff_saver::operator()<image_view_gray16>(image_view_gray16 const& image) const;
template void tiff_saver::operator()<image_view_gray16s>(image_view_gray16s const& image) const;
template void tiff_saver::operator()<image_view_gray32>(image_view_gray32 const& image) const;
template void tiff_saver::operator()<image_view_gray32s>(image_view_gray32s const& image) const;
template void tiff_saver::operator()<image_view_gray32f>(image_view_gray32f const& image) const;
template void tiff_saver::operator()<image_view_gray64>(image_view_gray64 const& image) const;
template void tiff_saver::operator()<image_view_gray64s>(image_view_gray64s const& image) const;
template void tiff_saver::operator()<image_view_gray64f>(image_view_gray64f const& image) const;

} // namespace mapnik
