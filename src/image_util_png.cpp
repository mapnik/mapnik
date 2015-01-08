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

#if defined(HAVE_PNG)
extern "C"
{
#include <png.h>
}
#endif

// mapnik
#if defined(HAVE_PNG)
#include <mapnik/png_io.hpp>
#endif

#include <mapnik/image_util_png.hpp>
#include <mapnik/image_data.hpp>

// boost
#include <boost/tokenizer.hpp>

// stl
#include <string>
#include <iostream>

namespace mapnik
{

#if defined(HAVE_PNG)

void handle_png_options(std::string const& type,
                        png_options & opts)
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
    boost::char_separator<char> sep(":");
    boost::tokenizer< boost::char_separator<char> > tokens(type, sep);
    bool set_colors = false;
    bool set_gamma = false;
    for (std::string const& t : tokens)
    {
        if (t == "png8" || t == "png256")
        {
            opts.paletted = true;
        }
        else if (t == "png" || t == "png24" || t == "png32")
        {
            opts.paletted = false;
        }
        else if (t == "m=o")
        {
            opts.use_hextree = false;
        }
        else if (t == "m=h")
        {
            opts.use_hextree = true;
        }
        else if (t == "e=miniz")
        {
            opts.use_miniz = true;
        }
        else if (boost::algorithm::starts_with(t, "c="))
        {
            set_colors = true;
            if (!mapnik::util::string2int(t.substr(2),opts.colors) || opts.colors < 1 || opts.colors > 256)
            {
                throw ImageWriterException("invalid color parameter: " + t.substr(2));
            }
        }
        else if (boost::algorithm::starts_with(t, "t="))
        {
            if (!mapnik::util::string2int(t.substr(2),opts.trans_mode) || opts.trans_mode < 0 || opts.trans_mode > 2)
            {
                throw ImageWriterException("invalid trans_mode parameter: " + t.substr(2));
            }
        }
        else if (boost::algorithm::starts_with(t, "g="))
        {
            set_gamma = true;
            if (!mapnik::util::string2double(t.substr(2),opts.gamma) || opts.gamma < 0)
            {
                throw ImageWriterException("invalid gamma parameter: " + t.substr(2));
            }
        }
        else if (boost::algorithm::starts_with(t, "z="))
        {
            /*
              #define Z_NO_COMPRESSION         0
              #define Z_BEST_SPEED             1
              #define Z_BEST_COMPRESSION       9
              #define Z_DEFAULT_COMPRESSION  (-1)
            */
            if (!mapnik::util::string2int(t.substr(2),opts.compression)
                || opts.compression < Z_DEFAULT_COMPRESSION
                || opts.compression > 10) // use 10 here rather than Z_BEST_COMPRESSION (9) to allow for MZ_UBER_COMPRESSION
            {
                throw ImageWriterException("invalid compression parameter: " + t.substr(2) + " (only -1 through 10 are valid)");
            }
        }
        else if (boost::algorithm::starts_with(t, "s="))
        {
            std::string s = t.substr(2);
            if (s == "default")
            {
                opts.strategy = Z_DEFAULT_STRATEGY;
            }
            else if (s == "filtered")
            {
                opts.strategy = Z_FILTERED;
            }
            else if (s == "huff")
            {
                opts.strategy = Z_HUFFMAN_ONLY;
            }
            else if (s == "rle")
            {
                opts.strategy = Z_RLE;
            }
            else if (s == "fixed")
            {
                opts.strategy = Z_FIXED;
            }
            else
            {
                throw ImageWriterException("invalid compression strategy parameter: " + s);
            }
        }
        else
        {
            throw ImageWriterException("unhandled png option: " + t);
        }
    }
    // validation
    if (!opts.paletted && set_colors)
    {
        throw ImageWriterException("invalid color parameter: unavailable for true color (non-paletted) images");
    }
    if (!opts.paletted && set_gamma)
    {
        throw ImageWriterException("invalid gamma parameter: unavailable for true color (non-paletted) images");
    }
    if ((opts.use_miniz == false) && opts.compression > Z_BEST_COMPRESSION)
    {
        throw ImageWriterException("invalid compression value: (only -1 through 9 are valid)");
    }
}
#endif

png_saver::png_saver(std::ostream & stream, std::string const& t, rgba_palette_ptr const& pal):
    stream_(stream), t_(t), pal_(pal) {}

void png_saver::operator() (image_data_null const& image) const
{
    throw ImageWriterException("null images not supported");
}

void png_saver::operator() (image_data_rgba8 const& image) const
{
#if defined(HAVE_PNG)
    png_options opts;
    handle_png_options(t_, opts);
    if (pal_ && pal_->valid())
    {
        png_options opts;
        handle_png_options(t,opts);
        save_as_png8_pal(stream, image, pal_, opts);
    }
    else if (opts.paletted)
    {
        if (opts.use_hextree)
        {
            save_as_png8_hex(stream_, image, opts);
        }
        else
        {
            save_as_png8_oct(stream_, image, opts);
        }
    }
    else
    {
        save_as_png(stream_, image, opts);
    }
#else
    throw ImageWriterException("png output is not enabled in your build of Mapnik");
#endif
}

void png_saver::operator() (image_view<image_data_rgba8> const& image) const
{
#if defined(HAVE_PNG)
    png_options opts;
    handle_png_options(t_, opts);
    if (pal_ && pal_->valid())
    {
        png_options opts;
        handle_png_options(t,opts);
        save_as_png8_pal(stream, image, pal_, opts);
    }
    else if (opts.paletted)
    {
        if (opts.use_hextree)
        {
            save_as_png8_hex(stream_, image, opts);
        }
        else
        {
            save_as_png8_oct(stream_, image, opts);
        }
    }
    else
    {
        save_as_png(stream_, image, opts);
    }
#else
    throw ImageWriterException("png output is not enabled in your build of Mapnik");
#endif
}

void png_saver::operator() (image_data_gray8 const& image) const
{
#if defined(HAVE_PNG)
    png_options opts;
    handle_png_options(t_, opts);
    save_as_png(stream_, image, opts);
#else
    throw ImageWriterException("png output is not enabled in your build of Mapnik");
#endif
}

void png_saver::operator() (image_view<image_data_gray8> const& image) const
{
#if defined(HAVE_PNG)
    png_options opts;
    handle_png_options(t_, opts);
    save_as_png(stream_, image, opts);
#else
    throw ImageWriterException("png output is not enabled in your build of Mapnik");
#endif
}

void png_saver::operator() (image_data_gray16 const& image) const
{
#if defined(HAVE_PNG)
    png_options opts;
    handle_png_options(t_, opts);
    save_as_png(stream_, image, opts);
#else
    throw ImageWriterException("png output is not enabled in your build of Mapnik");
#endif
}

void png_saver::operator() (image_view<image_data_gray16> const& image) const
{
#if defined(HAVE_PNG)
    png_options opts;
    handle_png_options(t_, opts);
    save_as_png(stream_, image, opts);
#else
    throw ImageWriterException("png output is not enabled in your build of Mapnik");
#endif
}

void png_saver::operator() (mapnik::image_data_gray32f const& image) const
{
#if defined(HAVE_PNG)
    png_options opts;
    handle_png_options(t_, opts);
    save_as_png(stream_, image, opts);
#else
    throw ImageWriterException("png output is not enabled in your build of Mapnik");
#endif
}
void png_saver::operator() (mapnik::image_data_gray32f const& image) const
{
#if defined(HAVE_PNG)
    png_options opts;
    handle_png_options(t_, opts);
    save_as_png(stream_, image, opts);
#else
    throw ImageWriterException("png output is not enabled in your build of Mapnik");
#endif
}

} // end ns
