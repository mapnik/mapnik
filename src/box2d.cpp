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

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/util/trim.hpp>

// stl
#include <stdexcept>

// boost
// fusion
#include <boost/fusion/include/adapt_adt.hpp>
// spirit
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_adapt_adt_attributes.hpp>

// agg
#include "agg_trans_affine.h"

BOOST_FUSION_ADAPT_TPL_ADT(
    (T),
    (mapnik::box2d)(T),
    (T, T, obj.minx(), obj.set_minx(val))
    (T, T, obj.miny(), obj.set_miny(val))
    (T, T, obj.maxx(), obj.set_maxx(val))
    (T, T, obj.maxy(), obj.set_maxy(val)))

namespace mapnik
{
template <typename T>
box2d<T>::box2d()
    :minx_(0),miny_(0),maxx_(-1),maxy_(-1) {}

template <typename T>
box2d<T>::box2d(T minx,T miny,T maxx,T maxy)
{
    init(minx,miny,maxx,maxy);
}

template <typename T>
box2d<T>::box2d(coord<T,2> const& c0, coord<T,2> const& c1)
{
    init(c0.x,c0.y,c1.x,c1.y);
}

template <typename T>
box2d<T>::box2d(box2d_type const& rhs)
    : minx_(rhs.minx_),
      miny_(rhs.miny_),
      maxx_(rhs.maxx_),
      maxy_(rhs.maxy_) {}

template <typename T>
box2d<T>::box2d(box2d_type && rhs)
    : minx_(std::move(rhs.minx_)),
      miny_(std::move(rhs.miny_)),
      maxx_(std::move(rhs.maxx_)),
      maxy_(std::move(rhs.maxy_)) {}

template <typename T>
box2d<T>& box2d<T>::operator=(box2d_type other)
{
    swap(other);
    return *this;
}

template <typename T>
void box2d<T>::swap(box2d_type & other)
{
    std::swap(minx_, other.minx_);
    std::swap(miny_, other.miny_);
    std::swap(maxx_, other.maxx_);
    std::swap(maxy_, other.maxy_);
}

template <typename T>
box2d<T>::box2d(box2d_type const& rhs, agg::trans_affine const& tr)
{
    double x0 = rhs.minx_, y0 = rhs.miny_;
    double x1 = rhs.maxx_, y1 = rhs.miny_;
    double x2 = rhs.maxx_, y2 = rhs.maxy_;
    double x3 = rhs.minx_, y3 = rhs.maxy_;
    tr.transform(&x0, &y0);
    tr.transform(&x1, &y1);
    tr.transform(&x2, &y2);
    tr.transform(&x3, &y3);
    init(static_cast<T>(x0), static_cast<T>(y0),
         static_cast<T>(x2), static_cast<T>(y2));
    expand_to_include(static_cast<T>(x1), static_cast<T>(y1));
    expand_to_include(static_cast<T>(x3), static_cast<T>(y3));
}

template <typename T>
bool box2d<T>::operator==(box2d<T> const& other) const
{
    return minx_==other.minx_ &&
        miny_==other.miny_ &&
        maxx_==other.maxx_ &&
        maxy_==other.maxy_;
}

template <typename T>
T box2d<T>::minx() const
{
    return minx_;
}

template <typename T>
T box2d<T>::maxx() const
{
    return maxx_;
}

template <typename T>
T box2d<T>::miny() const
{
    return miny_;
}

template <typename T>
T box2d<T>::maxy() const
{
    return maxy_;
}

template<typename T>
void box2d<T>::set_minx(T v)
{
    minx_ = v;
}

template<typename T>
void box2d<T>::set_miny(T v)
{
    miny_ = v;
}

template<typename T>
void box2d<T>::set_maxx(T v)
{
    maxx_ = v;
}

template<typename T>
void box2d<T>::set_maxy(T v)
{
    maxy_ = v;
}

template <typename T>
T box2d<T>::width() const
{
    return maxx_-minx_;
}

template <typename T>
T box2d<T>::height() const
{
    return maxy_-miny_;
}

template <typename T>
void box2d<T>::width(T w)
{
    T cx=center().x;
    minx_=static_cast<T>(cx-w*0.5);
    maxx_=static_cast<T>(cx+w*0.5);
}

template <typename T>
void box2d<T>::height(T h)
{
    T cy=center().y;
    miny_=static_cast<T>(cy-h*0.5);
    maxy_=static_cast<T>(cy+h*0.5);
}

template <typename T>
coord<T,2> box2d<T>::center() const
{
    return coord<T,2>(static_cast<T>(0.5*(minx_+maxx_)),
                      static_cast<T>(0.5*(miny_+maxy_)));
}

template <typename T>
void box2d<T>::expand_to_include(coord<T,2> const& c)
{
    expand_to_include(c.x,c.y);
}

template <typename T>
void box2d<T>::expand_to_include(T x,T y)
{
    if (x<minx_) minx_=x;
    if (x>maxx_) maxx_=x;
    if (y<miny_) miny_=y;
    if (y>maxy_) maxy_=y;
}

template <typename T>
void box2d<T>::expand_to_include(box2d<T> const& other)
{
    if (other.minx_<minx_) minx_=other.minx_;
    if (other.maxx_>maxx_) maxx_=other.maxx_;
    if (other.miny_<miny_) miny_=other.miny_;
    if (other.maxy_>maxy_) maxy_=other.maxy_;
}

template <typename T>
bool box2d<T>::contains(coord<T,2> const& c) const
{
    return contains(c.x,c.y);
}

template <typename T>
bool box2d<T>::contains(T x,T y) const
{
    return x>=minx_ && x<=maxx_ && y>=miny_ && y<=maxy_;
}

template <typename T>
bool box2d<T>::contains(box2d<T> const& other) const
{
    return other.minx_>=minx_ &&
        other.maxx_<=maxx_ &&
        other.miny_>=miny_ &&
        other.maxy_<=maxy_;
}

template <typename T>
bool box2d<T>::intersects(coord<T,2> const& c) const
{
    return intersects(c.x,c.y);
}

template <typename T>
bool box2d<T>::intersects(T x,T y) const
{
    return !(x>maxx_ || x<minx_ || y>maxy_ || y<miny_);
}

template <typename T>
bool box2d<T>::intersects(box2d<T> const& other) const
{
    return !(other.minx_>maxx_ || other.maxx_<minx_ ||
             other.miny_>maxy_ || other.maxy_<miny_);
}

template <typename T>
box2d<T> box2d<T>::intersect(box2d_type const& other) const
{
    if (intersects(other)) {
        T x0=std::max(minx_,other.minx_);
        T y0=std::max(miny_,other.miny_);

        T x1=std::min(maxx_,other.maxx_);
        T y1=std::min(maxy_,other.maxy_);

        return box2d<T>(x0,y0,x1,y1);
    } else {
        return box2d<T>();
    }
}

template <typename T>
void box2d<T>::re_center(T cx,T cy)
{
    T dx=cx-center().x;
    T dy=cy-center().y;
    minx_+=dx;
    miny_+=dy;
    maxx_+=dx;
    maxy_+=dy;
}

template <typename T>
void box2d<T>::re_center(coord<T,2> const& c)
{
    re_center(c.x,c.y);
}

template <typename T>
void box2d<T>::init(T x0,T y0,T x1,T y1)
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
void box2d<T>::clip(box2d_type const& other)
{
    minx_ = std::max(minx_,other.minx());
    miny_ = std::max(miny_,other.miny());
    maxx_ = std::min(maxx_,other.maxx());
    maxy_ = std::min(maxy_,other.maxy());
}

template <typename T>
void box2d<T>::pad(T padding)
{
    minx_ -= padding;
    miny_ -= padding;
    maxx_ += padding;
    maxy_ += padding;
}


template <typename T>
bool box2d<T>::from_string(std::string const& str)
{
    boost::spirit::qi::lit_type lit;
    boost::spirit::qi::double_type double_;
    boost::spirit::ascii::space_type space;
    bool r = boost::spirit::qi::phrase_parse(str.begin(),
                                             str.end(),
                                             double_ >> -lit(',') >> double_ >> -lit(',')
                                             >> double_ >> -lit(',') >> double_,
                                             space,
                                             *this);
    return r;
}

template <typename T>
bool box2d<T>::valid() const
{
    return (minx_ <= maxx_ && miny_ <= maxy_) ;
}

template <typename T>
void box2d<T>::move(T x, T y)
{
    minx_ += x;
    maxx_ += x;
    miny_ += y;
    maxy_ += y;
}

template <typename T>
box2d<T>&  box2d<T>::operator+=(box2d<T> const& other)
{
    expand_to_include(other);
    return *this;
}

template <typename T>
box2d<T> box2d<T>::operator+ (T other) const
{
    return box2d<T>(minx_ - other, miny_ - other, maxx_ + other, maxy_ + other);
}

template <typename T>
box2d<T>& box2d<T>::operator+= (T other)
{
    minx_ -= other;
    miny_ -= other;
    maxx_ += other;
    maxy_ += other;
    return *this;
}


template <typename T>
box2d<T>& box2d<T>::operator*=(T t)
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
box2d<T>& box2d<T>::operator/=(T t)
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

template <typename T>
T box2d<T>::operator[] (int index) const
{
    switch(index)
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

template <typename T>
box2d<T> box2d<T>::operator*(agg::trans_affine const& tr) const
{
    return box2d<T>(*this, tr);
}

template <typename T>
box2d<T>& box2d<T>::operator*=(agg::trans_affine const& tr)
{
    double x0 = minx_, y0 = miny_;
    double x1 = maxx_, y1 = miny_;
    double x2 = maxx_, y2 = maxy_;
    double x3 = minx_, y3 = maxy_;
    tr.transform(&x0, &y0);
    tr.transform(&x1, &y1);
    tr.transform(&x2, &y2);
    tr.transform(&x3, &y3);
    init(static_cast<T>(x0), static_cast<T>(y0),
         static_cast<T>(x2), static_cast<T>(y2));
    expand_to_include(static_cast<T>(x1), static_cast<T>(y1));
    expand_to_include(static_cast<T>(x3), static_cast<T>(y3));
    return *this;
}

template class box2d<int>;
template class box2d<double>;
}
