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

#ifndef MAPNIK_SPAN_IMAGE_FILTER_INCLUDED
#define MAPNIK_SPAN_IMAGE_FILTER_INCLUDED

#include "agg_span_image_filter_gray.h"
#include "agg_span_image_filter_rgba.h"

#include <limits>

namespace mapnik
{

template<class Source>
class span_image_resample_gray_affine : public agg::span_image_resample_affine<Source>
{
public:
    using source_type = Source;
    using color_type = typename source_type::color_type;
    using base_type = agg::span_image_resample_affine<source_type>;
    using interpolator_type = typename base_type::interpolator_type;
    using value_type = typename color_type::value_type;
    using long_type = typename color_type::long_type;

    enum base_scale_e
    {
        downscale_shift = agg::image_filter_shift,
        base_mask       = color_type::base_mask
    };

    span_image_resample_gray_affine(source_type & src,
                                    interpolator_type & inter,
                                    agg::image_filter_lut const & filter,
                                    boost::optional<value_type> const & nodata_value) :
        base_type(src, inter, filter),
        nodata_value_(nodata_value)
    { }

    void generate(color_type* span, int x, int y, unsigned len)
    {
        base_type::interpolator().begin(x + base_type::filter_dx_dbl(),
                                        y + base_type::filter_dy_dbl(), len);

        long_type fg;

        int diameter     = base_type::filter().diameter();
        int filter_scale = diameter << agg::image_subpixel_shift;
        int radius_x     = (diameter * base_type::m_rx) >> 1;
        int radius_y     = (diameter * base_type::m_ry) >> 1;
        int len_x_lr     =
            (diameter * base_type::m_rx + agg::image_subpixel_mask) >>
                agg::image_subpixel_shift;

        const agg::int16* weight_array = base_type::filter().weight_array();

        do
        {
            base_type::interpolator().coordinates(&x, &y);

            int src_x = x >> agg::image_subpixel_shift;
            int src_y = y >> agg::image_subpixel_shift;
            const value_type* pix = reinterpret_cast<const value_type*>(base_type::source().span(src_x, src_y, 0));
            if (nodata_value_ && *nodata_value_ == *pix)
            {
                span->v = *nodata_value_;
                span->a = base_mask;
                ++span;
                ++base_type::interpolator();
                continue;
            }

            x += base_type::filter_dx_int() - radius_x;
            y += base_type::filter_dy_int() - radius_y;

            fg = 0;

            int y_lr = y >> agg::image_subpixel_shift;
            int y_hr = ((agg::image_subpixel_mask - (y & agg::image_subpixel_mask)) *
                            base_type::m_ry_inv) >>
                                agg::image_subpixel_shift;
            int total_weight = 0;
            int x_lr = x >> agg::image_subpixel_shift;
            int x_hr = ((agg::image_subpixel_mask - (x & agg::image_subpixel_mask)) *
                            base_type::m_rx_inv) >>
                                agg::image_subpixel_shift;

            int x_hr2 = x_hr;
            const value_type* fg_ptr = reinterpret_cast<const value_type*>(base_type::source().span(x_lr, y_lr, len_x_lr));
            for(;;)
            {
                int weight_y = weight_array[y_hr];
                x_hr = x_hr2;
                for(;;)
                {
                    int weight = (weight_y * weight_array[x_hr] +
                                 agg::image_filter_scale) >>
                                 downscale_shift;
                    if (!nodata_value_ || *nodata_value_ != *fg_ptr)
                    {
                        fg += *fg_ptr * weight;
                        total_weight += weight;
                    }
                    x_hr  += base_type::m_rx_inv;
                    if (x_hr >= filter_scale) break;
                    fg_ptr = reinterpret_cast<const value_type*>(base_type::source().next_x());
                }
                y_hr += base_type::m_ry_inv;
                if (y_hr >= filter_scale) break;
                fg_ptr = reinterpret_cast<const value_type*>(base_type::source().next_y());
            }

            fg /= total_weight;
            if (fg < std::numeric_limits<value_type>::min())
            {
                span->v = std::numeric_limits<value_type>::min();
            }
            else if (fg > std::numeric_limits<value_type>::max())
            {
                span->v = std::numeric_limits<value_type>::max();
            }
            else
            {
                span->v = static_cast<value_type>(fg);
            }
            span->a = base_mask;

            ++span;
            ++base_type::interpolator();
        } while(--len);
    }

private:
    boost::optional<value_type> nodata_value_;
};

template<class Source>
class span_image_resample_rgba_affine : public agg::span_image_resample_rgba_affine<Source>
{
public:
    using source_type = Source;
    using color_type = typename source_type::color_type;
    using order_type = typename source_type::order_type;
    using base_type = agg::span_image_resample_rgba_affine<source_type>;
    using interpolator_type = typename base_type::interpolator_type;
    using value_type = typename color_type::value_type;
    using long_type = typename color_type::long_type;

    span_image_resample_rgba_affine(source_type & src,
                                    interpolator_type & inter,
                                    agg::image_filter_lut const & filter,
                                    boost::optional<value_type> const & nodata_value) :
        agg::span_image_resample_rgba_affine<Source>(src, inter, filter)
    { }
};

}

#endif
