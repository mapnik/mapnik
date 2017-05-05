/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <mapnik/coord.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/operators.hpp>
#pragma GCC diagnostic pop

// agg
// forward declare so that apps using mapnik do not need agg headers
namespace agg {
struct trans_affine;
}

namespace mapnik {

// A spatial envelope (i.e. bounding box) which also defines some basic operators.

template <typename T> class MAPNIK_DECL box2d
: boost::equality_comparable<box2d<T> ,
                             boost::addable<box2d<T>,
                                            boost::dividable2<box2d<T>, T,
                                                              boost::multipliable2<box2d<T>, T > > > >
{
public:
    using value_type = T;
    using box2d_type = box2d<value_type>;
    T minx_;
    T miny_;
    T maxx_;
    T maxy_;
private:
    friend inline void swap(box2d_type & lhs, box2d_type & rhs)
    {
        using std::swap;
        swap(lhs.minx_, rhs.minx_);
        swap(lhs.miny_, rhs.miny_);
        swap(lhs.maxx_, rhs.maxx_);
        swap(lhs.maxy_, rhs.maxy_);
    }
public:

    box2d();
    box2d(T minx,T miny,T maxx,T maxy);
    box2d(coord<T,2> const& c0, coord<T,2> const& c1);
    box2d(box2d_type const& rhs);
    box2d(box2d_type const& rhs, agg::trans_affine const& tr);
    // move
    box2d(box2d_type&& rhs);
    // converting ctor
    template <typename T1>
    explicit box2d(box2d<T1> other)
        : minx_(static_cast<value_type>(other.minx())),
        miny_(static_cast<value_type>(other.miny())),
        maxx_(static_cast<value_type>(other.maxx())),
        maxy_(static_cast<value_type>(other.maxy()))
        {}
    box2d_type& operator=(box2d_type other);
    T minx() const;
    T miny() const;
    T maxx() const;
    T maxy() const;
    void set_minx(T v);
    void set_miny(T v);
    void set_maxx(T v);
    void set_maxy(T v);
    T width() const;
    T height() const;
    void width(T w);
    void height(T h);
    coord<T,2> center() const;
    void expand_to_include(T x,T y);
    void expand_to_include(coord<T,2> const& c);
    void expand_to_include(box2d_type const& other);
    bool contains(coord<T,2> const& c) const;
    bool contains(T x,T y) const;
    bool contains(box2d_type const& other) const;
    bool intersects(coord<T,2> const& c) const;
    bool intersects(T x,T y) const;
    bool intersects(box2d_type const& other) const;
    box2d_type intersect(box2d_type const& other) const;
    bool operator==(box2d_type const& other) const;
    void re_center(T cx,T cy);
    void re_center(coord<T,2> const& c);
    void init(T x0,T y0,T x1,T y1);
    void init(T x, T y);
    void clip(box2d_type const& other);
    void pad(T padding);
    bool from_string(std::string const& str);
    bool valid() const;
    void move(T x, T y);
    std::string to_string() const;
    T area() const;

    // define some operators
    box2d_type& operator+=(box2d_type const& other);
    box2d_type& operator*=(T);
    box2d_type& operator/=(T);
    T operator[](int index) const;
    box2d_type operator +(T other) const; //enlarge box by given amount
    box2d_type& operator +=(T other); //enlarge box by given amount

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
