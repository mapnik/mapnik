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

#ifndef MAPNIK_BOX2D_HPP
#define MAPNIK_BOX2D_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/coord.hpp>

// boost
#include <boost/operators.hpp>

// stl
#include <iomanip>

// agg
// forward declare so that apps using mapnik do not need agg headers
namespace agg {
struct trans_affine;
}

namespace mapnik {

/*!
 * A spatial envelope (i.e. bounding box) which also defines some basic operators.
 */
template <typename T> class MAPNIK_DECL box2d
: boost::equality_comparable<box2d<T> ,
                             boost::addable<box2d<T>,
                                            boost::dividable2<box2d<T>, T,
                                                              boost::multipliable2<box2d<T>, T > > > >
{
public:
    typedef box2d<T> box2d_type;
private:
    T minx_;
    T miny_;
    T maxx_;
    T maxy_;
public:
    box2d();
    box2d(T minx,T miny,T maxx,T maxy);
    box2d(const coord<T,2>& c0,const coord<T,2>& c1);
    box2d(const box2d_type& rhs);
    box2d(const box2d_type& rhs, const agg::trans_affine& tr);
    T minx() const;
    T miny() const;
    T maxx() const;
    T maxy() const;
    T width() const;
    T height() const;
    void width(T w);
    void height(T h);
    coord<T,2> center() const;
    void expand_to_include(T x,T y);
    void expand_to_include(const coord<T,2>& c);
    void expand_to_include(const box2d_type& other);
    bool contains(const coord<T,2> &c) const;
    bool contains(T x,T y) const;
    bool contains(const box2d_type &other) const;
    bool intersects(const coord<T,2> &c) const;
    bool intersects(T x,T y) const;
    bool intersects(const box2d_type &other) const;
    box2d_type intersect(const box2d_type& other) const;
    bool operator==(const box2d_type &other) const;
    void re_center(T cx,T cy);
    void re_center(const coord<T,2>& c);
    void init(T x0,T y0,T x1,T y1);
    void clip(const box2d_type &other);
    bool from_string(std::string const& s);
    bool valid() const;

    // define some operators
    box2d_type& operator+=(box2d_type const& other);
    box2d_type& operator*=(T);
    box2d_type& operator/=(T);
    T operator[](int index) const;

    // compute the bounding box of this one transformed
    box2d_type  operator* (agg::trans_affine const& tr) const;
    box2d_type& operator*=(agg::trans_affine const& tr);
};

template <class charT,class traits,class T>
inline std::basic_ostream<charT,traits>&
operator << (std::basic_ostream<charT,traits>& out,
             const box2d<T>& e)
{
    std::basic_ostringstream<charT,traits> s;
    s.copyfmt(out);
    s.width(0);
    s << "box2d(" << std::fixed << std::setprecision(16)
      << e.minx() << ',' << e.miny() << ','
      << e.maxx() << ',' << e.maxy() << ')';
    out << s.str();
    return out;
}
}

#endif // MAPNIK_BOX2D_HPP
