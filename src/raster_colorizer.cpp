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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/value.hpp> // for to_double
#include <mapnik/feature.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/enumeration.hpp>

// stl
#include <limits>
#include <cmath>

namespace mapnik {

//! \brief Strings for the colorizer_mode enumeration
using colorizer_mode_str = detail::EnumStringT<colorizer_mode_enum>;
constexpr detail::EnumMapT<colorizer_mode_enum, 7> colorizer_mode_map{{
  colorizer_mode_str{colorizer_mode_enum::COLORIZER_INHERIT, "inherit"},
  colorizer_mode_str{colorizer_mode_enum::COLORIZER_LINEAR, "linear"},
  colorizer_mode_str{colorizer_mode_enum::COLORIZER_DISCRETE, "discrete"},
  colorizer_mode_str{colorizer_mode_enum::COLORIZER_EXACT, "exact"},
  colorizer_mode_str{colorizer_mode_enum::COLORIZER_LINEAR_RGBA, "linear-rgba"},
  colorizer_mode_str{colorizer_mode_enum::COLORIZER_LINEAR_BGRA, "linear-bgra"},
  colorizer_mode_str{colorizer_mode_enum::colorizer_mode_enum_MAX, ""},
}};
IMPLEMENT_ENUM(colorizer_mode, colorizer_mode_enum)

colorizer_stop::colorizer_stop(float val, colorizer_mode mode, color const& _color, std::string const& label)
    : value_(val)
    , mode_(mode)
    , color_(_color)
    , label_(label)
{}

colorizer_stop::colorizer_stop(colorizer_stop const& stop)
    : value_(stop.value_)
    , mode_(stop.mode_)
    , color_(stop.color_)
    , label_(stop.label_)
{}

colorizer_stop::~colorizer_stop() {}

bool colorizer_stop::operator==(colorizer_stop const& other) const
{
    return (value_ == other.value_) && (color_ == other.color_) && (mode_ == other.mode_) && (label_ == other.label_);
}

std::string colorizer_stop::to_string() const
{
    std::stringstream ss;
    ss << color_.to_string() << " " << value_ << " " << mode_.as_string();
    return ss.str();
}

raster_colorizer::raster_colorizer(colorizer_mode mode, color const& _color)
    : default_mode_(mode)
    , default_color_(_color)
    , epsilon_(std::numeric_limits<float>::epsilon())
{}

raster_colorizer::~raster_colorizer() {}

bool raster_colorizer::add_stop(colorizer_stop const& stop)
{
    // make sure stops are added in order of value
    if (stops_.size())
    {
        if (stop.get_value() <= stops_.back().get_value())
        {
            return false;
        }
    }

    stops_.push_back(stop);

    return true;
}

namespace {
template<typename PixelType>
float get_nodata_color()
{
    return 0; // rgba(0,0,0,0)
}

template<>
float get_nodata_color<std::uint8_t>()
{
    // GDAL specific interpretation of nodata
    // for GDT_Byte single band -> rgba(0,0,0,255)
    return static_cast<float>(0xff000000);
}

} // namespace

template<typename T>
void raster_colorizer::colorize(image_rgba8& out,
                                T const& in,
                                std::optional<double> const& nodata,
                                feature_impl const& f) const
{
    using image_type = T;
    using pixel_type = typename image_type::pixel_type;

    const std::size_t width = std::min(in.width(), out.width());
    const std::size_t height = std::min(in.height(), out.height());

    for (std::size_t y = 0; y < height; ++y)
    {
        pixel_type const* in_row = in.get_row(y);
        image_rgba8::pixel_type* out_row = out.get_row(y);
        for (std::size_t x = 0; x < width; ++x)
        {
            pixel_type val = in_row[x];
            if (nodata && (std::fabs(val - *nodata) < epsilon_))
            {
                out_row[x] = get_nodata_color<pixel_type>();
            }
            else
            {
                out_row[x] = get_color(static_cast<float>(val));
            }
        }
    }
}

inline unsigned interpolate(unsigned start, unsigned end, float fraction)
{
    return static_cast<unsigned>(fraction * (static_cast<float>(end) - static_cast<float>(start)) +
                                 static_cast<float>(start));
}

unsigned raster_colorizer::get_color(float val) const
{
    int stopCount = stops_.size();

    // use default color if no stops
    if (stopCount == 0)
    {
        return default_color_.rgba();
    }

    // 1 - Find the stop that the val is in
    int stopIdx = -1;
    bool foundStopIdx = false;

    for (int i = 0; i < stopCount; ++i)
    {
        if (val < stops_[i].get_value())
        {
            stopIdx = i - 1;
            foundStopIdx = true;
            break;
        }
    }

    if (!foundStopIdx)
    {
        stopIdx = stopCount - 1;
    }

    // 2 - Find the next stop
    int nextStopIdx = stopIdx + 1;
    if (nextStopIdx >= stopCount)
    {
        // there is no next stop
        nextStopIdx = stopCount - 1;
    }

    // 3 - Work out the mode
    colorizer_mode stopMode;
    if (stopIdx == -1)
    {
        // before the first stop
        stopMode = default_mode_;
    }
    else
    {
        stopMode = stops_[stopIdx].get_mode();
        if (stopMode == colorizer_mode_enum::COLORIZER_INHERIT)
        {
            stopMode = default_mode_;
        }
    }

    // 4 - Calculate the colour
    color stopColor;
    color nextStopColor;
    float stopValue = 0;
    float nextStopValue = 0;
    color outputColor = get_default_color();
    if (stopIdx == -1)
    {
        stopColor = default_color_;
        nextStopColor = stops_[nextStopIdx].get_color();
        stopValue = val;
        nextStopValue = stops_[nextStopIdx].get_value();
    }
    else
    {
        stopColor = stops_[stopIdx].get_color();
        nextStopColor = stops_[nextStopIdx].get_color();
        stopValue = stops_[stopIdx].get_value();
        nextStopValue = stops_[nextStopIdx].get_value();
    }

    switch (stopMode)
    {
        case colorizer_mode_enum::COLORIZER_LINEAR: {
            // deal with this separately so we don't have to worry about div0
            if (nextStopValue == stopValue)
            {
                outputColor = stopColor;
            }
            else
            {
                float fraction = (val - stopValue) / (nextStopValue - stopValue);

                unsigned r = interpolate(stopColor.red(), nextStopColor.red(), fraction);
                unsigned g = interpolate(stopColor.green(), nextStopColor.green(), fraction);
                unsigned b = interpolate(stopColor.blue(), nextStopColor.blue(), fraction);
                unsigned a = interpolate(stopColor.alpha(), nextStopColor.alpha(), fraction);

                outputColor.set_red(r);
                outputColor.set_green(g);
                outputColor.set_blue(b);
                outputColor.set_alpha(a);
            }
        }
        break;
        case colorizer_mode_enum::COLORIZER_LINEAR_RGBA: {
            if (nextStopValue == stopValue)
            {
                return stopColor.rgba();
            }

            double fraction = (val - stopValue) / (nextStopValue - stopValue);
            double colorStart = static_cast<double>(stopColor.rgba());
            double colorEnd = static_cast<double>(nextStopColor.rgba());
            outputColor = color(colorStart + fraction * (colorEnd - colorStart));
        }
        break;
        case colorizer_mode_enum::COLORIZER_LINEAR_BGRA: {
            if (nextStopValue == stopValue)
            {
                return stopColor.rgba();
            }

            double fraction = (val - stopValue) / (nextStopValue - stopValue);
            std::swap(stopColor.red_, stopColor.blue_);
            std::swap(nextStopColor.red_, nextStopColor.blue_);
            double colorStart = static_cast<double>(stopColor.rgba());
            double colorEnd = static_cast<double>(nextStopColor.rgba());
            outputColor = color(colorStart + fraction * (colorEnd - colorStart));
            std::swap(outputColor.red_, outputColor.blue_);
        }
        break;
        case colorizer_mode_enum::COLORIZER_DISCRETE:
            outputColor = stopColor;
            break;
        case colorizer_mode_enum::COLORIZER_EXACT:
        default:
            // approximately equal (within epsilon)
            if (std::fabs(val - stopValue) < epsilon_)
            {
                outputColor = stopColor;
            }
            else
            {
                outputColor = default_color_;
            }
            break;
    }

    /*
      MAPNIK_LOG_DEBUG(raster_colorizer) << "raster_colorizer: get_color " << val;
      MAPNIK_LOG_DEBUG(raster_colorizer) << "\tstopIdx: " << stopIdx;
      MAPNIK_LOG_DEBUG(raster_colorizer) << "\tnextStopIdx: " << nextStopIdx;
      MAPNIK_LOG_DEBUG(raster_colorizer) << "\tstopValue: " << stopValue;
      MAPNIK_LOG_DEBUG(raster_colorizer) << "\tnextStopValue: " << nextStopValue;
      MAPNIK_LOG_DEBUG(raster_colorizer) << "\tstopColor: " << stopColor.to_string();
      MAPNIK_LOG_DEBUG(raster_colorizer) << "\tnextStopColor: " << nextStopColor.to_string();
      MAPNIK_LOG_DEBUG(raster_colorizer) << "\tstopMode: " << stopMode.as_string();
      MAPNIK_LOG_DEBUG(raster_colorizer) << "\toutputColor: " << outputColor.to_string();
    */

    return outputColor.rgba();
}

template void raster_colorizer::colorize(image_rgba8& out,
                                         image_gray8 const& in,
                                         std::optional<double> const& nodata,
                                         feature_impl const& f) const;
template void raster_colorizer::colorize(image_rgba8& out,
                                         image_gray8s const& in,
                                         std::optional<double> const& nodata,
                                         feature_impl const& f) const;
template void raster_colorizer::colorize(image_rgba8& out,
                                         image_gray16 const& in,
                                         std::optional<double> const& nodata,
                                         feature_impl const& f) const;
template void raster_colorizer::colorize(image_rgba8& out,
                                         image_gray16s const& in,
                                         std::optional<double> const& nodata,
                                         feature_impl const& f) const;
template void raster_colorizer::colorize(image_rgba8& out,
                                         image_gray32 const& in,
                                         std::optional<double> const& nodata,
                                         feature_impl const& f) const;
template void raster_colorizer::colorize(image_rgba8& out,
                                         image_gray32s const& in,
                                         std::optional<double> const& nodata,
                                         feature_impl const& f) const;
template void raster_colorizer::colorize(image_rgba8& out,
                                         image_gray32f const& in,
                                         std::optional<double> const& nodata,
                                         feature_impl const& f) const;
template void raster_colorizer::colorize(image_rgba8& out,
                                         image_gray64 const& in,
                                         std::optional<double> const& nodata,
                                         feature_impl const& f) const;
template void raster_colorizer::colorize(image_rgba8& out,
                                         image_gray64s const& in,
                                         std::optional<double> const& nodata,
                                         feature_impl const& f) const;
template void raster_colorizer::colorize(image_rgba8& out,
                                         image_gray64f const& in,
                                         std::optional<double> const& nodata,
                                         feature_impl const& f) const;

} // namespace mapnik
