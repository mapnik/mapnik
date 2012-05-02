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

#ifndef MAPNIK_OFFSET_CONVERTER_HPP
#define MAPNIK_OFFSET_CONVERTER_HPP

#include <mapnik/debug.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/coord_array.hpp>
#include <mapnik/proj_transform.hpp>

// boost
#include <boost/math/constants/constants.hpp>

namespace mapnik
{

const double pi = boost::math::constants::pi<double>();
const double half_pi = pi/2.0;

template <typename Geometry>
struct MAPNIK_DECL offset_converter
{
    typedef std::size_t size_type;
    //typedef typename Geometry::value_type value_type;

    offset_converter(Geometry & geom)
        : geom_(geom),
        offset_(0.0),
        threshold_(10),
        status_(initial) {}
    
    enum status
    {
        initial,
        start,
        first,
        process,
        last_vertex,
        angle_joint,
        end
    };

    double get_offset() const
    {
        return offset_;
    }

    void set_offset(double offset)
    {
        offset_ = offset;
    }

    unsigned int get_threshold() const
    {
        return threshold_;
    }

    void set_threshold(unsigned int t)
    {
        threshold_ = t;
    }

    unsigned vertex(double * x , double  * y)
    {
        if (offset_==0.0)
        {
            unsigned command = geom_.vertex(x,y);          
            return command;
        }
        else
        {
            while(true)
            {
                switch(status_)
                {
                case end:
                    return SEG_END;
                    break;
                case initial:
                    pre_cmd_ = geom_.vertex(x,y);
                    pre_x_ = *x;
                    pre_y_ = *y;
                    //status_ = (pre_cmd_!=SEG_END)?start:end; //
                case start:
                    cur_cmd_ = geom_.vertex(&cur_x_, &cur_y_);
                case first:
                    angle_a = atan2((pre_y_-cur_y_),(pre_x_-cur_x_));
                    dx_pre = cos(angle_a + half_pi);
                    dy_pre = sin(angle_a + half_pi);

                    MAPNIK_LOG_DEBUG(ctrans) << "coord_transform_parallel: Offsetting line by=" << offset_;
                    MAPNIK_LOG_DEBUG(ctrans) << "coord_transform_parallel: Initial dx=" << (dx_pre * offset_) << ",dy=" << (dy_pre * offset_);

                    *x = pre_x_ + (dx_pre * offset_);
                    *y = pre_y_ + (dy_pre * offset_);
                    status_ = process;
                    return SEG_MOVETO;
                case process:
                    switch(cur_cmd_)
                    {
                    case SEG_LINETO:
                        next_cmd_ = geom_.vertex(&next_x_, &next_y_);
                        switch(next_cmd_)
                        {
                        case SEG_LINETO:
                            status_ = angle_joint;
                            break;
                        default:
                            status_ = last_vertex;
                            break;
                        }
                        break;
                    case SEG_END:
                        status_ = end;
                        return SEG_END;
                    }
                    break;
                case last_vertex:
                    dx_curr = cos(angle_a + half_pi);
                    dy_curr = sin(angle_a + half_pi);
                    *x = cur_x_ + (dx_curr * offset_);
                    *y = cur_y_ + (dy_curr * offset_);
                    status_ = end;
                    return cur_cmd_;
                case angle_joint:
                    angle_b = atan2((cur_y_-next_y_),(cur_x_-next_x_));
                    h = tan((angle_b - angle_a)/2.0);

                    if (fabs(h) < threshold_)
                    {
                        dx_curr = cos(angle_a + half_pi);
                        dy_curr = sin(angle_a + half_pi);
                        *x = cur_x_ + (dx_curr * offset_) - h * (dy_curr * offset_);
                        *y = cur_y_ + (dy_curr * offset_) + h * (dx_curr * offset_);
                    }
                    else // skip sharp spikes
                    {
#ifdef MAPNIK_LOG
                        dx_curr = cos(angle_a + half_pi);
                        dy_curr = sin(angle_a + half_pi);
                        sin_curve = dx_curr*dy_pre-dy_curr*dx_pre;
                        MAPNIK_LOG_DEBUG(ctrans) << "coord_transform_parallel: angle a=" << angle_a;
                        MAPNIK_LOG_DEBUG(ctrans) << "coord_transform_parallel: angle b=" << angle_b;
                        MAPNIK_LOG_DEBUG(ctrans) << "coord_transform_parallel: h=" << h;
                        MAPNIK_LOG_DEBUG(ctrans) << "coord_transform_parallel: sin_curve=" << sin_curve;
#endif
                        status_ = process;
                        break;
                    }

                    // alternate sharp spike fix, but suboptimal...

                    /*
                      sin_curve = dx_curr*dy_pre-dy_curr*dx_pre;
                      cos_curve = -dx_pre*dx_curr-dy_pre*dy_curr;

                      MAPNIK_LOG_DEBUG(ctrans) << "coord_transform_parallel: sin_curve value=" << sin_curve;
                      if(sin_curve > -0.3 && sin_curve < 0.3) {
                        angle_b = atan2((cur_y_-next_y_),(cur_x_-next_x_));
                        h = tan((angle_b - angle_a)/2.0);
                        *x = cur_x_ + (dx_curr * offset_) - h * (dy_curr * offset_);
                        *y = cur_y_ + (dy_curr * offset_) + h * (dx_curr * offset_);
                      } else {
                        if (angle_b - angle_a > 0)
                          h = -1.0*(1.0+cos_curve)/sin_curve;
                        else
                          h = (1.0+cos_curve)/sin_curve;
                        *x = cur_x_ + (dx_curr + base_shift*dy_curr)*offset_;
                        *y = cur_y_ + (dy_curr - base_shift*dx_curr)*offset_;
                      }
                    */

                    pre_x_ = *x;
                    pre_x_ = *y;
                    cur_x_ = next_x_;
                    cur_y_ = next_y_;
                    angle_a = angle_b;
                    pre_cmd_ = cur_cmd_;
                    cur_cmd_ = next_cmd_;
                    status_ = process;
                    return pre_cmd_;
                }
            }
        }
    }

    void rewind (unsigned pos)
    {
        geom_.rewind(pos);
        status_ = initial;
    }

private:
    Geometry & geom_;
    int           offset_;
    unsigned int  threshold_;
    status        status_;
    double        dx_pre;
    double        dy_pre;
    double        dx_curr;
    double        dy_curr;
    double        sin_curve;
    double        cos_curve;
    double        angle_a;
    double        angle_b;
    double        h;
    unsigned      pre_cmd_;
    double        pre_x_;
    double        pre_y_;
    unsigned      cur_cmd_;
    double        cur_x_;
    double        cur_y_;
    unsigned      next_cmd_;
    double        next_x_;
    double        next_y_;
};

}

#endif // MAPNIK_OFFSET_CONVERTER_HPP
