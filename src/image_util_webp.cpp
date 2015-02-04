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
#if defined(HAVE_WEBP)
#include <mapnik/webp_io.hpp>
#endif

#include <mapnik/image_util.hpp>
#include <mapnik/image_util_webp.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/util/conversions.hpp>

// boost
#include <boost/tokenizer.hpp>

// stl
#include <string>
#include <iostream>

namespace mapnik
{

#if defined(HAVE_WEBP)
void handle_webp_options(std::string const& type,
                        WebPConfig & config,
                        bool & alpha)
{
    if (type == "webp")
    {
        return;
    }
    if (type.length() > 4){
        boost::char_separator<char> sep(":");
        boost::tokenizer< boost::char_separator<char> > tokens(type, sep);
        for (auto const& t : tokens)
        {
            if (t == "webp")
            {
                continue;
            }
            else if (boost::algorithm::starts_with(t, "quality="))
            {
                std::string val = t.substr(8);
                if (!val.empty())
                {
                    double quality = 90;
                    if (!mapnik::util::string2double(val,quality) || quality < 0.0 || quality > 100.0)
                    {
                        throw ImageWriterException("invalid webp quality: '" + val + "'");
                    }
                    config.quality = static_cast<float>(quality);
                }
            }
            else if (boost::algorithm::starts_with(t, "method="))
            {
                std::string val = t.substr(7);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.method) || config.method < 0 || config.method > 6)
                    {
                        throw ImageWriterException("invalid webp method: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "lossless="))
            {
                std::string val = t.substr(9);
                if (!val.empty())
                {
                    #if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1 // >= v0.1.99 / 0x0100
                    if (!mapnik::util::string2int(val,config.lossless) || config.lossless < 0 || config.lossless > 1)
                    {
                        throw ImageWriterException("invalid webp lossless: '" + val + "'");
                    }
                    #else
                        #ifdef _MSC_VER
                          #pragma NOTE(compiling against webp that does not support the lossless flag)
                        #else
                          #warning "compiling against webp that does not support the lossless flag"
                        #endif
                    throw ImageWriterException("your webp version does not support the lossless option");
                    #endif
                }
            }
            else if (boost::algorithm::starts_with(t, "image_hint="))
            {
                std::string val = t.substr(11);
                if (!val.empty())
                {
                    #if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1 // >= v0.1.99 / 0x0100
                    int image_hint = 0;
                    if (!mapnik::util::string2int(val,image_hint) || image_hint < 0 || image_hint > 3)
                    {
                        throw ImageWriterException("invalid webp image_hint: '" + val + "'");
                    }
                    config.image_hint = static_cast<WebPImageHint>(image_hint);
                    #else
                        #ifdef _MSC_VER
                          #pragma NOTE(compiling against webp that does not support the image_hint flag)
                        #else
                          #warning "compiling against webp that does not support the image_hint flag"
                        #endif
                    throw ImageWriterException("your webp version does not support the image_hint option");
                    #endif
                }
            }
            else if (boost::algorithm::starts_with(t, "alpha="))
            {
                std::string val = t.substr(6);
                if (!val.empty())
                {
                    if (!mapnik::util::string2bool(val,alpha))
                    {
                        throw ImageWriterException("invalid webp alpha: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "target_size="))
            {
                std::string val = t.substr(12);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.target_size))
                    {
                        throw ImageWriterException("invalid webp target_size: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "target_psnr="))
            {
                std::string val = t.substr(12);
                if (!val.empty())
                {
                    double psnr = 0;
                    if (!mapnik::util::string2double(val,psnr))
                    {
                        throw ImageWriterException("invalid webp target_psnr: '" + val + "'");
                    }
                    config.target_PSNR = psnr;
                }
            }
            else if (boost::algorithm::starts_with(t, "segments="))
            {
                std::string val = t.substr(9);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.segments))
                    {
                        throw ImageWriterException("invalid webp segments: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "sns_strength="))
            {
                std::string val = t.substr(13);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.sns_strength))
                    {
                        throw ImageWriterException("invalid webp sns_strength: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "filter_strength="))
            {
                std::string val = t.substr(16);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.filter_strength))
                    {
                        throw ImageWriterException("invalid webp filter_strength: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "filter_sharpness="))
            {
                std::string val = t.substr(17);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.filter_sharpness))
                    {
                        throw ImageWriterException("invalid webp filter_sharpness: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "filter_type="))
            {
                std::string val = t.substr(12);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.filter_type))
                    {
                        throw ImageWriterException("invalid webp filter_type: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "autofilter="))
            {
                std::string val = t.substr(11);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.autofilter))
                    {
                        throw ImageWriterException("invalid webp autofilter: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "alpha_compression="))
            {
                std::string val = t.substr(18);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.alpha_compression))
                    {
                        throw ImageWriterException("invalid webp alpha_compression: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "alpha_filtering="))
            {
                std::string val = t.substr(16);
                if (!val.empty())
                {
                    #if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1 // >= v0.1.99 / 0x0100
                    if (!mapnik::util::string2int(val,config.alpha_filtering))
                    {
                        throw ImageWriterException("invalid webp alpha_filtering: '" + val + "'");
                    }
                    #else
                        #ifdef _MSC_VER
                          #pragma NOTE(compiling against webp that does not support the alpha_filtering flag)
                        #else
                          #warning "compiling against webp that does not support the alpha_filtering flag"
                        #endif
                    throw ImageWriterException("your webp version does not support the alpha_filtering option");
                    #endif
                }
            }
            else if (boost::algorithm::starts_with(t, "alpha_quality="))
            {
                std::string val = t.substr(14);
                if (!val.empty())
                {
                    #if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1 // >= v0.1.99 / 0x0100
                    if (!mapnik::util::string2int(val,config.alpha_quality))
                    {
                        throw ImageWriterException("invalid webp alpha_quality: '" + val + "'");
                    }
                    #else
                        #ifdef _MSC_VER
                          #pragma NOTE(compiling against webp that does not support the alpha_quality flag)
                        #else
                          #warning "compiling against webp that does not support the alpha_quality flag"
                        #endif
                    throw ImageWriterException("your webp version does not support the alpha_quality option");
                    #endif
                }
            }
            else if (boost::algorithm::starts_with(t, "pass="))
            {
                std::string val = t.substr(5);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.pass))
                    {
                        throw ImageWriterException("invalid webp pass: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "preprocessing="))
            {
                std::string val = t.substr(14);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.preprocessing))
                    {
                        throw ImageWriterException("invalid webp preprocessing: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "partitions="))
            {
                std::string val = t.substr(11);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.partitions))
                    {
                        throw ImageWriterException("invalid webp partitions: '" + val + "'");
                    }
                }
            }
            else if (boost::algorithm::starts_with(t, "partition_limit="))
            {
                std::string val = t.substr(16);
                if (!val.empty())
                {
                    if (!mapnik::util::string2int(val,config.partition_limit))
                    {
                        throw ImageWriterException("invalid webp partition_limit: '" + val + "'");
                    }
                }
            }
            else
            {
                throw ImageWriterException("unhandled webp option: " + t);
            }
        }
    }
}
#endif

webp_saver::webp_saver(std::ostream & stream, std::string const& t):
    stream_(stream), t_(t) {}

template<>
void webp_saver::operator()<image_null> (image_null const& image) const
{
    throw ImageWriterException("null images not supported");
}

template <typename T>
void process_rgba8_webp(T const& image, std::string const& t, std::ostream & stream)
{
#if defined(HAVE_WEBP)
    WebPConfig config;
    // Default values set here will be lossless=0 and quality=75 (as least as of webp v0.3.1)
    if (!WebPConfigInit(&config))
    {
        throw std::runtime_error("version mismatch");
    }
    // see for more details: https://github.com/mapnik/mapnik/wiki/Image-IO#webp-output-options
    bool alpha = true;
    handle_webp_options(t,config,alpha);
    save_as_webp(stream,image,config,alpha);
#else
    throw ImageWriterException("webp output is not enabled in your build of Mapnik");
#endif
}

template <>
void webp_saver::operator()<image_rgba8> (image_rgba8 const& image) const
{
    process_rgba8_webp(image, t_, stream_);
}

template <>
void webp_saver::operator()<image_view_rgba8> (image_view_rgba8 const& image) const
{
    process_rgba8_webp(image, t_, stream_);
}

template <typename T>
void webp_saver::operator() (T const& image) const
{
    throw ImageWriterException("Mapnik does not support webp grayscale images");
}

template void webp_saver::operator()<image_rgba8> (image_rgba8 const& image) const;
template void webp_saver::operator()<image_gray8> (image_gray8 const& image) const;
template void webp_saver::operator()<image_gray8s> (image_gray8s const& image) const;
template void webp_saver::operator()<image_gray16> (image_gray16 const& image) const;
template void webp_saver::operator()<image_gray16s> (image_gray16s const& image) const;
template void webp_saver::operator()<image_gray32> (image_gray32 const& image) const;
template void webp_saver::operator()<image_gray32s> (image_gray32s const& image) const;
template void webp_saver::operator()<image_gray32f> (image_gray32f const& image) const;
template void webp_saver::operator()<image_gray64> (image_gray64 const& image) const;
template void webp_saver::operator()<image_gray64s> (image_gray64s const& image) const;
template void webp_saver::operator()<image_gray64f> (image_gray64f const& image) const;
template void webp_saver::operator()<image_view_rgba8> (image_view_rgba8 const& image) const;
template void webp_saver::operator()<image_view_gray8> (image_view_gray8 const& image) const;
template void webp_saver::operator()<image_view_gray8s> (image_view_gray8s const& image) const;
template void webp_saver::operator()<image_view_gray16> (image_view_gray16 const& image) const;
template void webp_saver::operator()<image_view_gray16s> (image_view_gray16s const& image) const;
template void webp_saver::operator()<image_view_gray32> (image_view_gray32 const& image) const;
template void webp_saver::operator()<image_view_gray32s> (image_view_gray32s const& image) const;
template void webp_saver::operator()<image_view_gray32f> (image_view_gray32f const& image) const;
template void webp_saver::operator()<image_view_gray64> (image_view_gray64 const& image) const;
template void webp_saver::operator()<image_view_gray64s> (image_view_gray64s const& image) const;
template void webp_saver::operator()<image_view_gray64f> (image_view_gray64f const& image) const;

} // end ns
