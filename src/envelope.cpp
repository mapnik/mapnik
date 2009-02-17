/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
//$Id: envelope.cpp 17 2005-03-08 23:58:43Z pavlenko $

#include <mapnik/envelope.hpp>

namespace mapnik
{
    template <typename T>
    Envelope<T>::Envelope()
        :minx_(0),miny_(0),maxx_(-1),maxy_(-1) {}

    template <typename T>
    Envelope<T>::Envelope(T minx_,T miny_,T maxx_,T maxy_)
    {
        init(minx_,miny_,maxx_,maxy_);
    }

    template <typename T>
    Envelope<T>::Envelope(const coord<T,2> &c0,const coord<T,2> &c1)
    {
        init(c0.x,c0.y,c1.x,c1.y);
    }

    template <typename T>
    Envelope<T>::Envelope(const Envelope &rhs)
    {
        init(rhs.minx_,rhs.miny_,rhs.maxx_,rhs.maxy_);
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    bool Envelope<T>::operator==(const Envelope<T>& other) const
    {
        return minx_==other.minx_ &&
            miny_==other.miny_ &&
            maxx_==other.maxx_ &&
            maxy_==other.maxy_;
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    T Envelope<T>::minx() const
    {
        return minx_;
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    T Envelope<T>::maxx() const
    {
        return maxx_;
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    T Envelope<T>::miny() const
    {
        return miny_;
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    T Envelope<T>::maxy() const
    {
        return maxy_;
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    T Envelope<T>::width() const
    {
        return maxx_-minx_;
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    T Envelope<T>::height() const
    {
        return maxy_-miny_;
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    void Envelope<T>::width(T w)
    {
        T cx=center().x;
        minx_=static_cast<T>(cx-w*0.5);
        maxx_=static_cast<T>(cx+w*0.5);
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    void Envelope<T>::height(T h)
    {
        T cy=center().y;
        miny_=static_cast<T>(cy-h*0.5);
        maxy_=static_cast<T>(cy+h*0.5);
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    coord<T,2> Envelope<T>::center() const
    {
        return coord<T,2>(static_cast<T>(0.5*(minx_+maxx_)),
                          static_cast<T>(0.5*(miny_+maxy_)));
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    void Envelope<T>::expand_to_include(const coord<T,2>& c)
    {
        expand_to_include(c.x,c.y);
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    void Envelope<T>::expand_to_include(T x,T y)
    {
        if (x<minx_) minx_=x;
        if (x>maxx_) maxx_=x;
        if (y<miny_) miny_=y;
        if (y>maxy_) maxy_=y;
    }

    template <typename T>
    void Envelope<T>::expand_to_include(const Envelope<T> &other)
    {
        if (other.minx_<minx_) minx_=other.minx_;
        if (other.maxx_>maxx_) maxx_=other.maxx_;
        if (other.miny_<miny_) miny_=other.miny_;
        if (other.maxy_>maxy_) maxy_=other.maxy_;
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    bool Envelope<T>::contains(const coord<T,2> &c) const
    {
        return contains(c.x,c.y);
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    bool Envelope<T>::contains(T x,T y) const
    {
        return x>=minx_ && x<=maxx_ && y>=miny_ && y<=maxy_;
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    bool Envelope<T>::contains(const Envelope<T> &other) const
    {
        return other.minx_>=minx_ &&
            other.maxx_<=maxx_ &&
            other.miny_>=miny_ &&
            other.maxy_<=maxy_;
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    bool Envelope<T>::intersects(const coord<T,2> &c) const
    {
        return intersects(c.x,c.y);
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    bool Envelope<T>::intersects(T x,T y) const
    {
        return !(x>maxx_ || x<minx_ || y>maxy_ || y<miny_);
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    bool Envelope<T>::intersects(const Envelope<T> &other) const
    {
        return !(other.minx_>maxx_ || other.maxx_<minx_ ||
                 other.miny_>maxy_ || other.maxy_<miny_);
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    Envelope<T> Envelope<T>::intersect(const EnvelopeType& other) const
    {
        if (intersects(other)) {
            T x0=std::max(minx_,other.minx_);
            T y0=std::max(miny_,other.miny_);

            T x1=std::min(maxx_,other.maxx_);
            T y1=std::min(maxy_,other.maxy_);

            return Envelope<T>(x0,y0,x1,y1);
        } else {
            return Envelope<T>();
        }
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    void Envelope<T>::re_center(T cx,T cy)
    {
        T dx=cx-center().x;
        T dy=cy-center().y;
        minx_+=dx;
        miny_+=dy;
        maxx_+=dx;
        maxy_+=dy;
    }

    template <typename T>
#if !defined(__SUNPRO_CC)
    inline
#endif
    void Envelope<T>::init(T x0,T y0,T x1,T y1)
    {
        if (x0<x1)
        {
            minx_=x0;maxx_=x1;
        }
        else
        {
            minx_=x1;maxx_=x0;
        }
        if (y0<y1)
        {
            miny_=y0;maxy_=y1;
        }
        else
        {
            miny_=y1;maxy_=y0;
        }
    }
    
    template <typename T>
    Envelope<T>&  Envelope<T>::operator+=(Envelope<T> const& other)
    {
        expand_to_include(other);
        return *this;
    }
    
    template <typename T>    
    Envelope<T>& Envelope<T>::operator-=(Envelope<T> const& other)
    {
        // not sure what to do here. intersect?
        return *this;
    }
    
    template <typename T>    
    Envelope<T>& Envelope<T>::operator*=(T t)
    {
       coord<T,2> c = center();
       T sx = static_cast<T>(0.5 * width()  * t);
       T sy = static_cast<T>(0.5 * height() * t);
       minx_ = c.x - sx;
       maxx_ = c.x + sx;
       miny_ = c.y - sy;
       maxy_ = c.y + sy;
       return *this;
    }
   
    template <typename T>    
    Envelope<T>& Envelope<T>::operator/=(T t)
    {
       coord<T,2> c = center();
       T sx = static_cast<T>(0.5 * width() / t);
       T sy = static_cast<T>(0.5 * height() / t);
       minx_ = c.x - sx;
       maxx_ = c.x + sx;
       miny_ = c.y - sy;
       maxy_ = c.y + sy;
       return *this;
    }
    
    template class Envelope<int>;
    template class Envelope<double>;
}
