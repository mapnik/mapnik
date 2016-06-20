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

#ifndef MAPNIK_BOX2D_HPP
#define MAPNIK_BOX2D_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/geometry/point.hpp>
// stl
#include <limits>
#include <string>
#include <stdexcept>
// agg
// forward declare so that apps using mapnik do not need agg headers
namespace agg {
struct trans_affine;
}

namespace mapnik {

// A spatial envelope (i.e. bounding box) which also defines some basic operators.

template <typename T> class MAPNIK_DECL box2d
{
public:
    using value_type = T;
    using box2d_type = box2d<value_type>;
private:
    T minx_;
    T miny_;
    T maxx_;
    T maxy_;
    friend inline void swap(box2d_type & lhs, box2d_type & rhs)
    {
        using std::swap;
        swap(lhs.minx_, rhs.minx_);
        swap(lhs.miny_, rhs.miny_);
        swap(lhs.maxx_, rhs.maxx_);
        swap(lhs.maxy_, rhs.maxy_);
    }
public:

    box2d()
      : minx_(std::numeric_limits<T>::max()),
        miny_(std::numeric_limits<T>::max()),
        maxx_(-std::numeric_limits<T>::max()),
        maxy_(-std::numeric_limits<T>::max()) {}

    box2d(T minx,T miny,T maxx,T maxy)
    {
        init(minx, miny, maxx, maxy);
    }
    box2d(geometry::point<T> const& c0, geometry::point<T> const& c1)
    {
        init(c0.x,c0.y,c1.x,c1.y);
    }
    // move ctor
    box2d(box2d_type&& rhs)
      : minx_(std::move(rhs.minx_)),
        miny_(std::move(rhs.miny_)),
        maxx_(std::move(rhs.maxx_)),
        maxy_(std::move(rhs.maxy_)) {}

    // copy ctor
    box2d(box2d_type const& rhs)
      : minx_(rhs.minx_),
        miny_(rhs.miny_),
        maxx_(rhs.maxx_),
        maxy_(rhs.maxy_) {}

    box2d(box2d_type const& rhs, agg::trans_affine const& tr);

    // converting ctor
    template <typename T1>
    explicit box2d(box2d<T1> other)
      : minx_(static_cast<value_type>(other.minx())),
        miny_(static_cast<value_type>(other.miny())),
        maxx_(static_cast<value_type>(other.maxx())),
        maxy_(static_cast<value_type>(other.maxy()))
        {}

    box2d_type& operator=(box2d_type other)
    {
        swap(*this, other);
        return *this;
    }

    T minx() const {return minx_;}
    T miny() const {return miny_;}
    T maxx() const {return maxx_;}
    T maxy() const {return maxy_;}
    void set_minx(T v) { minx_ = v;}
    void set_miny(T v) { miny_ = v;}
    void set_maxx(T v) { maxx_ = v;}
    void set_maxy(T v) { maxy_ = v;}
    T width() const {return maxx_ - minx_;}
    T height() const {return maxy_ - miny_;}
    void width(T w)
    {
        T cx = center().x;
        minx_ = static_cast<T>(cx - w * 0.5);
        maxx_ = static_cast<T>(cx + w * 0.5);
    }
    void height(T h)
    {
        T cy = center().y;
        miny_ = static_cast<T>(cy - h * 0.5);
        maxy_ = static_cast<T>(cy + h * 0.5);
    }

    geometry::point<T> center() const
    {
        return geometry::point<T>(static_cast<T>(0.5 * (minx_ + maxx_)),
                                  static_cast<T>(0.5 * (miny_ + maxy_)));
    }

    void expand_to_include(T x,T y)
    {
        if (x < minx_) minx_ = x;
        if (x > maxx_) maxx_ = x;
        if (y < miny_) miny_ = y;
        if (y > maxy_) maxy_ = y;
    }

    void expand_to_include(geometry::point<T> const& c)
    {
        expand_to_include(c.x,c.y);
    }

    void expand_to_include(box2d_type const& other)
    {
        if (other.minx_ < minx_) minx_ = other.minx_;
        if (other.maxx_ > maxx_) maxx_ = other.maxx_;
        if (other.miny_ < miny_) miny_ = other.miny_;
        if (other.maxy_ > maxy_) maxy_ = other.maxy_;
    }

    bool contains(T x,T y) const
    {
        return x >= minx_ && x <= maxx_ && y >= miny_ && y <= maxy_;
    }

    bool contains(geometry::point<T> const& c) const
    {
        return contains(c.x,c.y);
    }

    bool contains(box2d_type const& other) const
    {
        return other.minx_ >= minx_ &&
            other.maxx_ <= maxx_ &&
            other.miny_ >= miny_ &&
            other.maxy_ <= maxy_;
    }

    bool intersects(T x,T y) const
    {
        return !(x > maxx_ || x < minx_ || y > maxy_ || y < miny_);
    }

    bool intersects(geometry::point<T> const& c) const
    {
        return intersects(c.x,c.y);
    }

    bool intersects(box2d_type const& other) const
    {
        return !(other.minx_ > maxx_ || other.maxx_ < minx_ ||
                 other.miny_ > maxy_ || other.maxy_ < miny_);
    }

    box2d_type intersect(box2d_type const& other) const
    {
        if (intersects(other))
        {
            T x0 = std::max(minx_, other.minx_);
            T y0 = std::max(miny_, other.miny_);
            T x1 = std::min(maxx_, other.maxx_);
            T y1 = std::min(maxy_, other.maxy_);
            return box2d<T>(x0, y0, x1, y1);
        }
        else
        {
            return box2d<T>();
        }
    }

    bool operator==(box2d_type const& other) const
    {
        return minx_ == other.minx_ &&
            miny_ == other.miny_ &&
            maxx_ == other.maxx_ &&
            maxy_ == other.maxy_;
    }

    bool operator!=(box2d_type const& other) const
    {
        return !(*this == other);
    }

    void re_center(T cx,T cy)
    {
        T dx = cx - center().x;
        T dy = cy - center().y;
        minx_ += dx;
        miny_ += dy;
        maxx_ += dx;
        maxy_ += dy;
    }

    void re_center(geometry::point<T> const& c)
    {
        re_center(c.x,c.y);
    }

    void init(T x0,T y0,T x1,T y1)
    {
        if (x0 < x1)
        {
            minx_ = x0;
            maxx_ = x1;
        }
        else
        {
            minx_ = x1;
            maxx_ = x0;
        }
        if (y0 < y1)
        {
            miny_ = y0;
            maxy_ = y1;
        }
        else
        {
            miny_ = y1;
            maxy_ = y0;
        }
    }

    void init(T x, T y)
    {
        init(x, y, x, y);
    }

    void clip(box2d_type const& other)
    {
        minx_ = std::max(minx_, other.minx());
        miny_ = std::max(miny_, other.miny());
        maxx_ = std::min(maxx_, other.maxx());
        maxy_ = std::min(maxy_, other.maxy());
    }

    void pad(T padding)
    {
        minx_ -= padding;
        miny_ -= padding;
        maxx_ += padding;
        maxy_ += padding;
    }

    bool valid() const
    {
        return (minx_ <= maxx_ && miny_ <= maxy_);
    }

    void move(T x, T y)
    {
        minx_ += x;
        maxx_ += x;
        miny_ += y;
        maxy_ += y;
    }

    // to/from std::string
    bool from_string(std::string const& str);
    std::string to_string() const;

    // operators
    box2d_type& operator+=(box2d_type const& other)
    {
        expand_to_include(other);
        return *this;
    }

    box2d_type& operator*=(T val)
    {
        geometry::point<T> c = center();
        T sx = static_cast<T>(0.5 * width()  * val);
        T sy = static_cast<T>(0.5 * height() * val);
        minx_ = c.x - sx;
        maxx_ = c.x + sx;
        miny_ = c.y - sy;
        maxy_ = c.y + sy;
        return *this;
    }

    box2d_type  operator*(T val) const
    {
        box2d<T> temp(*this);
        return temp *= val;
    }

    box2d_type& operator/=(T val)
    {
        geometry::point<T> c = center();
        T sx = static_cast<T>(0.5 * width() / val);
        T sy = static_cast<T>(0.5 * height() / val);
        minx_ = c.x - sx;
        maxx_ = c.x + sx;
        miny_ = c.y - sy;
        maxy_ = c.y + sy;
        return *this;
    }

    T operator[](int index) const
    {
        switch (index)
        {
        case 0:
            return minx_;
        case 1:
            return miny_;
        case 2:
            return maxx_;
        case 3:
            return maxy_;
        case -4:
            return minx_;
        case -3:
            return miny_;
        case -2:
            return maxx_;
        case -1:
            return maxy_;
        default:
            throw std::out_of_range("index out of range, max value is 3, min value is -4 ");
        }
    }

    box2d_type operator +(T val) const //enlarge box by given amount
    {
        return box2d<T>(minx_ - val, miny_ - val, maxx_ + val, maxy_ + val);
    }

    box2d_type& operator +=(T val) //enlarge box by given amount
    {
        minx_ -= val;
        miny_ -= val;
        maxx_ += val;
        maxy_ += val;
        return *this;
    }

    // compute the bounding box of this one transformed
    box2d_type  operator* (agg::trans_affine const& tr) const;
    box2d_type& operator*=(agg::trans_affine const& tr);
};

template <class charT,class traits,class T>
inline std::basic_ostream<charT,traits>&
operator << (std::basic_ostream<charT,traits>& out,
             const box2d<T>& e)
{
    out << e.to_string();
    return out;
}
}

#endif // MAPNIK_BOX2D_HPP
