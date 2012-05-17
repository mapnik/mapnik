/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#include <boost/cstdint.hpp>
#include "agg_span_image_filter_rgba.h"

namespace mapnik { 

using namespace agg;

template<class Source>
class span_image_resample_rgba_affine :
        public span_image_resample_affine<Source>
{
public:
    typedef Source source_type;
    typedef typename source_type::color_type color_type;
    typedef typename source_type::order_type order_type;
    typedef span_image_resample_affine<source_type> base_type;
    typedef typename base_type::interpolator_type interpolator_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::long_type long_type;
    enum base_scale_e
    {
        base_shift      = color_type::base_shift,
        base_mask       = color_type::base_mask,
        downscale_shift = image_filter_shift
    };

    //--------------------------------------------------------------------
    span_image_resample_rgba_affine() {}
    span_image_resample_rgba_affine(source_type& src,
                                    interpolator_type& inter,
                                    const image_filter_lut& filter) :
        base_type(src, inter, filter)
    {}


    //--------------------------------------------------------------------
    void generate(color_type* span, int x, int y, unsigned len)
    {
        base_type::interpolator().begin(x + base_type::filter_dx_dbl(),
                                        y + base_type::filter_dy_dbl(), len);

        long_type fg[4];

        int diameter     = base_type::filter().diameter();
        int filter_scale = diameter << image_subpixel_shift;
        int radius_x     = (diameter * base_type::m_rx) >> 1;
        int radius_y     = (diameter * base_type::m_ry) >> 1;
        int len_x_lr     =
            (diameter * base_type::m_rx + image_subpixel_mask) >>
            image_subpixel_shift;

        const boost::int16_t* weight_array = base_type::filter().weight_array();

        do
        {
            base_type::interpolator().coordinates(&x, &y);

            x += base_type::filter_dx_int() - radius_x;
            y += base_type::filter_dy_int() - radius_y;

            fg[0] = fg[1] = fg[2] = fg[3] = image_filter_scale / 2;

            int y_lr = y >> image_subpixel_shift;
            int y_hr = ((image_subpixel_mask - (y & image_subpixel_mask)) *
                        base_type::m_ry_inv) >>
                image_subpixel_shift;
            int total_weight = 0;
            int x_lr = x >> image_subpixel_shift;
            int x_hr = ((image_subpixel_mask - (x & image_subpixel_mask)) *
                        base_type::m_rx_inv) >>
                image_subpixel_shift;

            int x_hr2 = x_hr;
            const value_type* fg_ptr =
                (const value_type*)base_type::source().span(x_lr, y_lr, len_x_lr);
            for(;;)
            {
                int weight_y = weight_array[y_hr];
                x_hr = x_hr2;
                for(;;)
                {
                    int weight = (weight_y * weight_array[x_hr] +
                                  image_filter_scale / 2) >>
                        downscale_shift;
                    
                    fg[0] += *fg_ptr++ * weight;
                    fg[1] += *fg_ptr++ * weight;
                    fg[2] += *fg_ptr++ * weight;
                    fg[3] += *fg_ptr   * weight;

                    total_weight += weight;
                    x_hr  += base_type::m_rx_inv;
                    if(x_hr >= filter_scale) break;
                    fg_ptr = (const value_type*)base_type::source().next_x();
                }
                y_hr += base_type::m_ry_inv;
                if(y_hr >= filter_scale) break;
                fg_ptr = (const value_type*)base_type::source().next_y();
            }

            if (total_weight)
            {
                fg[3] /= total_weight;
                fg[0] /= total_weight;
                fg[1] /= total_weight;
                fg[2] /= total_weight;

                if(fg[0] < 0) fg[0] = 0;
                if(fg[1] < 0) fg[1] = 0;
                if(fg[2] < 0) fg[2] = 0;
                if(fg[3] < 0) fg[3] = 0;
            }
            else
            {
                fg[0] = 0;
                fg[1] = 0;
                fg[2] = 0;
                fg[3] = 0;
            }

            if(fg[order_type::R] > base_mask)         fg[order_type::R] = base_mask;
            if(fg[order_type::G] > base_mask)         fg[order_type::G] = base_mask;
            if(fg[order_type::B] > base_mask)         fg[order_type::B] = base_mask;
            if(fg[order_type::A] > base_mask)         fg[order_type::A] = base_mask;
            
            span->r = (value_type)fg[order_type::R];
            span->g = (value_type)fg[order_type::G];
            span->b = (value_type)fg[order_type::B];
            span->a = (value_type)fg[order_type::A];

            ++span;
            ++base_type::interpolator();
        } while(--len);
    }
};
}
