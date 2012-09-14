/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_GRID_PIXEL_HPP
#define MAPNIK_GRID_PIXEL_HPP

#include "agg_basics.h"

namespace mapnik
{

//==================================================================gray16
struct gray16
{
    typedef agg::int16u value_type;
    typedef agg::int32u calc_type;
    typedef agg::int64  long_type;
    enum base_scale_e
    {
        base_shift = 16,
        base_scale = 1 << base_shift,
        base_mask  = base_scale - 1
    };
    typedef gray16 self_type;

    value_type v;
    value_type a;

    //--------------------------------------------------------------------
    gray16() {}

    //--------------------------------------------------------------------
    gray16(unsigned v_, unsigned a_=base_mask) :
        v(agg::int16u(v_)), a(agg::int16u(a_)) {}

    //--------------------------------------------------------------------
    gray16(const self_type& c, unsigned a_) :
        v(c.v), a(value_type(a_)) {}

    //--------------------------------------------------------------------
    void clear()
    {
        v = a = 0;
    }

    //--------------------------------------------------------------------
    const self_type& transparent()
    {
        a = 0;
        return *this;
    }

    //--------------------------------------------------------------------
    void opacity(double a_)
    {
        if(a_ < 0.0) a_ = 0.0;
        if(a_ > 1.0) a_ = 1.0;
        a = (value_type)agg::uround(a_ * double(base_mask));
    }

    //--------------------------------------------------------------------
    double opacity() const
    {
        return double(a) / double(base_mask);
    }


    //--------------------------------------------------------------------
    const self_type& premultiply()
    {
        if(a == base_mask) return *this;
        if(a == 0)
        {
            v = 0;
            return *this;
        }
        v = value_type((calc_type(v) * a) >> base_shift);
        return *this;
    }

    //--------------------------------------------------------------------
    const self_type& premultiply(unsigned a_)
    {
        if(a == base_mask && a_ >= base_mask) return *this;
        if(a == 0 || a_ == 0)
        {
            v = a = 0;
            return *this;
        }
        calc_type v_ = (calc_type(v) * a_) / a;
        v = value_type((v_ > a_) ? a_ : v_);
        a = value_type(a_);
        return *this;
    }

    //--------------------------------------------------------------------
    const self_type& demultiply()
    {
        if(a == base_mask) return *this;
        if(a == 0)
        {
            v = 0;
            return *this;
        }
        calc_type v_ = (calc_type(v) * base_mask) / a;
        v = value_type((v_ > base_mask) ? base_mask : v_);
        return *this;
    }

    //--------------------------------------------------------------------
    self_type gradient(self_type c, double k) const
    {
        self_type ret;
        calc_type ik = agg::uround(k * base_scale);
        ret.v = value_type(calc_type(v) + (((calc_type(c.v) - v) * ik) >> base_shift));
        ret.a = value_type(calc_type(a) + (((calc_type(c.a) - a) * ik) >> base_shift));
        return ret;
    }

    //--------------------------------------------------------------------
    AGG_INLINE void add(const self_type& c, unsigned cover)
    {
        calc_type cv, ca;
        if(cover == agg::cover_mask)
        {
            if(c.a == base_mask)
            {
                *this = c;
            }
            else
            {
                cv = v + c.v; v = (cv > calc_type(base_mask)) ? calc_type(base_mask) : cv;
                ca = a + c.a; a = (ca > calc_type(base_mask)) ? calc_type(base_mask) : ca;
            }
        }
        else
        {
            cv = v + ((c.v * cover + agg::cover_mask/2) >> agg::cover_shift);
            ca = a + ((c.a * cover + agg::cover_mask/2) >> agg::cover_shift);
            v = (cv > calc_type(base_mask)) ? calc_type(base_mask) : cv;
            a = (ca > calc_type(base_mask)) ? calc_type(base_mask) : ca;
        }
    }

    //--------------------------------------------------------------------
    static self_type no_color() { return self_type(0,0); }
};

//==================================================================gray16
struct gray32
{
    typedef agg::int32 value_type;
    typedef agg::int64u calc_type;
    typedef agg::int64  long_type;
    enum base_scale_e
    {
        base_shift = 16,
        base_scale = 1 << base_shift,
        base_mask  = base_scale - 1
    };
    typedef gray32 self_type;

    value_type v;
    value_type a;

    //--------------------------------------------------------------------
    gray32() {}

    //--------------------------------------------------------------------
    gray32(unsigned v_, unsigned a_=base_mask) :
        v(agg::int32(v_)), a(agg::int32(a_)) {}

    //--------------------------------------------------------------------
    gray32(const self_type& c, unsigned a_) :
        v(c.v), a(value_type(a_)) {}

    //--------------------------------------------------------------------
    void clear()
    {
        v = a = 0;
    }

    //--------------------------------------------------------------------
    const self_type& transparent()
    {
        a = 0;
        return *this;
    }

    //--------------------------------------------------------------------
    void opacity(double a_)
    {
        if(a_ < 0.0) a_ = 0.0;
        if(a_ > 1.0) a_ = 1.0;
        a = (value_type)agg::uround(a_ * double(base_mask));
    }

    //--------------------------------------------------------------------
    double opacity() const
    {
        return double(a) / double(base_mask);
    }


    //--------------------------------------------------------------------
    const self_type& premultiply()
    {
        if(a == base_mask) return *this;
        if(a == 0)
        {
            v = 0;
            return *this;
        }
        v = value_type((calc_type(v) * a) >> base_shift);
        return *this;
    }

    //--------------------------------------------------------------------
    const self_type& premultiply(unsigned a_)
    {
        if(a == base_mask && a_ >= base_mask) return *this;
        if(a == 0 || a_ == 0)
        {
            v = a = 0;
            return *this;
        }
        calc_type v_ = (calc_type(v) * a_) / a;
        v = value_type((v_ > a_) ? a_ : v_);
        a = value_type(a_);
        return *this;
    }

    //--------------------------------------------------------------------
    const self_type& demultiply()
    {
        if(a == base_mask) return *this;
        if(a == 0)
        {
            v = 0;
            return *this;
        }
        calc_type v_ = (calc_type(v) * base_mask) / a;
        v = value_type((v_ > base_mask) ? base_mask : v_);
        return *this;
    }

    //--------------------------------------------------------------------
    self_type gradient(self_type c, double k) const
    {
        self_type ret;
        calc_type ik = agg::uround(k * base_scale);
        ret.v = value_type(calc_type(v) + (((calc_type(c.v) - v) * ik) >> base_shift));
        ret.a = value_type(calc_type(a) + (((calc_type(c.a) - a) * ik) >> base_shift));
        return ret;
    }

    //--------------------------------------------------------------------
    AGG_INLINE void add(const self_type& c, unsigned cover)
    {
        calc_type cv, ca;
        if(cover == agg::cover_mask)
        {
            if(c.a == base_mask)
            {
                *this = c;
            }
            else
            {
                cv = v + c.v; v = (cv > calc_type(base_mask)) ? calc_type(base_mask) : cv;
                ca = a + c.a; a = (ca > calc_type(base_mask)) ? calc_type(base_mask) : ca;
            }
        }
        else
        {
            cv = v + ((c.v * cover + agg::cover_mask/2) >> agg::cover_shift);
            ca = a + ((c.a * cover + agg::cover_mask/2) >> agg::cover_shift);
            v = (cv > calc_type(base_mask)) ? calc_type(base_mask) : cv;
            a = (ca > calc_type(base_mask)) ? calc_type(base_mask) : ca;
        }
    }

    //--------------------------------------------------------------------
    static self_type no_color() { return self_type(0,0); }
};

}

#endif
