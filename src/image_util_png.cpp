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

#if defined(HAVE_PNG)
extern "C" {
#include <png.h>
}
#ifndef PNG_FAST_FILTERS // libpng < 1.6
#define PNG_FAST_FILTERS (PNG_FILTER_NONE | PNG_FILTER_SUB | PNG_FILTER_UP)
#endif
#endif // HAVE_PNG

// mapnik
#if defined(HAVE_PNG)
#include <mapnik/png_io.hpp>
#endif

#include <mapnik/image_util.hpp>
#include <mapnik/image_util_png.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/image_options.hpp>
#include <mapnik/util/conversions.hpp>

// stl
#include <string>
#include <iostream>

namespace mapnik {

#if defined(HAVE_PNG)

void handle_png_options(std::string const& type, png_options& opts)
{
    if (type == "png" || type == "png24" || type == "png32")
    {
        opts.paletted = false;
        return;
    }
    else if (type == "png8" || type == "png256")
    {
        opts.paletted = true;
        return;
    }

    bool set_colors = false;
    bool set_gamma = false;

    for (auto const& kv : parse_image_options(type))
    {
        auto const& key = kv.first;
        auto const& val = kv.second;
        if (key == "png8" || key == "png256")
        {
            opts.paletted = true;
        }
        else if (key == "png" || key == "png24" || key == "png32")
        {
            opts.paletted = false;
        }
        else if (key == "m" && val)
        {
            if (*val == "o")
                opts.use_hextree = false;
            else if (*val == "h")
                opts.use_hextree = true;
        }
        else if (key == "e" && val && *val == "miniz")
        {
            throw image_writer_exception("miniz support has been removed from Mapnik");
        }
        else if (key == "c")
        {
            set_colors = true;
            if (!val || !mapnik::util::string2int(*val, opts.colors) || opts.colors < 1 || opts.colors > 256)
            {
                throw image_writer_exception("invalid color parameter: " + to_string(val));
            }
        }
        else if (key == "t")
        {
            if (!val || !mapnik::util::string2int(*val, opts.trans_mode) || opts.trans_mode < 0 || opts.trans_mode > 2)
            {
                throw image_writer_exception("invalid trans_mode parameter: " + to_string(val));
            }
        }
        else if (key == "g")
        {
            set_gamma = true;
            if (!val || !mapnik::util::string2double(*val, opts.gamma) || opts.gamma < 0)
            {
                throw image_writer_exception("invalid gamma parameter: " + to_string(val));
            }
        }
        else if (key == "z")
        {
            /*
              #define Z_NO_COMPRESSION         0
              #define Z_BEST_SPEED             1
              #define Z_BEST_COMPRESSION       9
              #define Z_DEFAULT_COMPRESSION  (-1)
            */
            if (!val || !mapnik::util::string2int(*val, opts.compression) || opts.compression < Z_DEFAULT_COMPRESSION ||
                opts.compression >
                  10) // use 10 here rather than Z_BEST_COMPRESSION (9) to allow for MZ_UBER_COMPRESSION
            {
                throw image_writer_exception("invalid compression parameter: " + to_string(val) +
                                             " (only -1 through 10 are valid)");
            }
        }
        else if (key == "s")
        {
            if (!val)
                throw image_writer_exception("invalid compression parameter: <uninitialised>");

            if (*val == "default")
            {
                opts.strategy = Z_DEFAULT_STRATEGY;
            }
            else if (*val == "filtered")
            {
                opts.strategy = Z_FILTERED;
            }
            else if (*val == "huff")
            {
                opts.strategy = Z_HUFFMAN_ONLY;
            }
            else if (*val == "rle")
            {
                opts.strategy = Z_RLE;
            }
            else if (*val == "fixed")
            {
                opts.strategy = Z_FIXED;
            }
            else
            {
                throw image_writer_exception("invalid compression strategy parameter: " + *val);
            }
        }
        else if (key == "f")
        {
            // filters = PNG_NO_FILTERS;
            // filters = PNG_ALL_FILTERS;
            // filters = PNG_FAST_FILTERS;
            // filters = PNG_FILTER_NONE | PNG_FILTER_SUB | PNG_FILTER_UP | PNG_FILTER_AVG | PNG_FILTER_PAETH;

            if (!val)
                throw image_writer_exception("invalid filters parameter: <uninitialised>");
            if (*val == "no")
                opts.filters = PNG_NO_FILTERS;
            else if (*val == "all")
                opts.filters = PNG_ALL_FILTERS;
            else if (*val == "fast")
                opts.filters = PNG_FAST_FILTERS;
            else
                opts.filters = parse_png_filters(*val); // none | sub | up | avg | paeth
        }
        else
        {
            throw image_writer_exception("unhandled png option: " + key);
        }
    }
    // validation
    if (!opts.paletted && set_colors)
    {
        throw image_writer_exception("invalid color parameter: unavailable for true color (non-paletted) images");
    }
    if (!opts.paletted && set_gamma)
    {
        throw image_writer_exception("invalid gamma parameter: unavailable for true color (non-paletted) images");
    }
    if (opts.compression > Z_BEST_COMPRESSION)
    {
        throw image_writer_exception("invalid compression value: (only -1 through 9 are valid)");
    }
}
#endif

png_saver::png_saver(std::ostream& stream, std::string const& t)
    : stream_(stream),
      t_(t)
{}

png_saver_pal::png_saver_pal(std::ostream& stream, std::string const& t, rgba_palette const& pal)
    : stream_(stream),
      t_(t),
      pal_(pal)
{}

template<>
void png_saver::operator()<image_null>(image_null const& image) const
{
    throw image_writer_exception("null images not supported for png");
}

template<>
void png_saver_pal::operator()<image_null>(image_null const& image) const
{
    throw image_writer_exception("null images not supported for png");
}

template<>
void png_saver::operator()<image_view_null>(image_view_null const& image) const
{
    throw image_writer_exception("null image views not supported for png");
}

template<>
void png_saver_pal::operator()<image_view_null>(image_view_null const& image) const
{
    throw image_writer_exception("null image views not supported for png");
}

template<typename T>
void process_rgba8_png_pal(T const& image, std::string const& t, std::ostream& stream, rgba_palette const& pal)
{
#if defined(HAVE_PNG)
    png_options opts;
    handle_png_options(t, opts);
    if (pal.valid())
    {
        save_as_png8_pal(stream, image, pal, opts);
    }
    else if (opts.paletted)
    {
        if (opts.use_hextree)
        {
            save_as_png8_hex(stream, image, opts);
        }
        else
        {
            save_as_png8_oct(stream, image, opts);
        }
    }
    else
    {
        save_as_png(stream, image, opts);
    }
#else
    throw image_writer_exception("png output is not enabled in your build of Mapnik");
#endif
}

template<typename T>
void process_rgba8_png(T const& image, std::string const& t, std::ostream& stream)
{
#if defined(HAVE_PNG)
    png_options opts;
    handle_png_options(t, opts);
    if (opts.paletted)
    {
        if (opts.use_hextree)
        {
            save_as_png8_hex(stream, image, opts);
        }
        else
        {
            save_as_png8_oct(stream, image, opts);
        }
    }
    else
    {
        save_as_png(stream, image, opts);
    }
#else
    throw image_writer_exception("png output is not enabled in your build of Mapnik");
#endif
}

template<>
void png_saver_pal::operator()<image_rgba8>(image_rgba8 const& image) const
{
    process_rgba8_png_pal(image, t_, stream_, pal_);
}

template<>
void png_saver_pal::operator()<image_view_rgba8>(image_view_rgba8 const& image) const
{
    process_rgba8_png_pal(image, t_, stream_, pal_);
}

template<>
void png_saver::operator()<image_rgba8>(image_rgba8 const& image) const
{
    process_rgba8_png(image, t_, stream_);
}

template<>
void png_saver::operator()<image_view_rgba8>(image_view_rgba8 const& image) const
{
    process_rgba8_png(image, t_, stream_);
}

template<typename T>
void png_saver::operator()(T const& image) const
{
#if defined(HAVE_PNG)
    throw image_writer_exception("Mapnik does not support grayscale images for png");
    // png_options opts;
    // handle_png_options(t_, opts);
    // save_as_png(stream_, image, opts);
#else
    throw image_writer_exception("png output is not enabled in your build of Mapnik");
#endif
}

template<typename T>
void png_saver_pal::operator()(T const& image) const
{
#if defined(HAVE_PNG)
    throw image_writer_exception("Mapnik does not support grayscale images for png");
    // png_options opts;
    // handle_png_options(t_, opts);
    // save_as_png(stream_, image, opts);
#else
    throw image_writer_exception("png output is not enabled in your build of Mapnik");
#endif
}

template void png_saver::operator()<image_gray8>(image_gray8 const& image) const;
template void png_saver::operator()<image_gray8s>(image_gray8s const& image) const;
template void png_saver::operator()<image_gray16>(image_gray16 const& image) const;
template void png_saver::operator()<image_gray16s>(image_gray16s const& image) const;
template void png_saver::operator()<image_gray32>(image_gray32 const& image) const;
template void png_saver::operator()<image_gray32s>(image_gray32s const& image) const;
template void png_saver::operator()<image_gray32f>(image_gray32f const& image) const;
template void png_saver::operator()<image_gray64>(image_gray64 const& image) const;
template void png_saver::operator()<image_gray64s>(image_gray64s const& image) const;
template void png_saver::operator()<image_gray64f>(image_gray64f const& image) const;
template void png_saver::operator()<image_view_gray8>(image_view_gray8 const& image) const;
template void png_saver::operator()<image_view_gray8s>(image_view_gray8s const& image) const;
template void png_saver::operator()<image_view_gray16>(image_view_gray16 const& image) const;
template void png_saver::operator()<image_view_gray16s>(image_view_gray16s const& image) const;
template void png_saver::operator()<image_view_gray32>(image_view_gray32 const& image) const;
template void png_saver::operator()<image_view_gray32s>(image_view_gray32s const& image) const;
template void png_saver::operator()<image_view_gray32f>(image_view_gray32f const& image) const;
template void png_saver::operator()<image_view_gray64>(image_view_gray64 const& image) const;
template void png_saver::operator()<image_view_gray64s>(image_view_gray64s const& image) const;
template void png_saver::operator()<image_view_gray64f>(image_view_gray64f const& image) const;
template void png_saver_pal::operator()<image_gray8>(image_gray8 const& image) const;
template void png_saver_pal::operator()<image_gray8s>(image_gray8s const& image) const;
template void png_saver_pal::operator()<image_gray16>(image_gray16 const& image) const;
template void png_saver_pal::operator()<image_gray16s>(image_gray16s const& image) const;
template void png_saver_pal::operator()<image_gray32>(image_gray32 const& image) const;
template void png_saver_pal::operator()<image_gray32s>(image_gray32s const& image) const;
template void png_saver_pal::operator()<image_gray32f>(image_gray32f const& image) const;
template void png_saver_pal::operator()<image_gray64>(image_gray64 const& image) const;
template void png_saver_pal::operator()<image_gray64s>(image_gray64s const& image) const;
template void png_saver_pal::operator()<image_gray64f>(image_gray64f const& image) const;
template void png_saver_pal::operator()<image_view_gray8>(image_view_gray8 const& image) const;
template void png_saver_pal::operator()<image_view_gray8s>(image_view_gray8s const& image) const;
template void png_saver_pal::operator()<image_view_gray16>(image_view_gray16 const& image) const;
template void png_saver_pal::operator()<image_view_gray16s>(image_view_gray16s const& image) const;
template void png_saver_pal::operator()<image_view_gray32>(image_view_gray32 const& image) const;
template void png_saver_pal::operator()<image_view_gray32s>(image_view_gray32s const& image) const;
template void png_saver_pal::operator()<image_view_gray32f>(image_view_gray32f const& image) const;
template void png_saver_pal::operator()<image_view_gray64>(image_view_gray64 const& image) const;
template void png_saver_pal::operator()<image_view_gray64s>(image_view_gray64s const& image) const;
template void png_saver_pal::operator()<image_view_gray64f>(image_view_gray64f const& image) const;

} // namespace mapnik
