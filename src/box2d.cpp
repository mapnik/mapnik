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

// stl
#include <stdexcept>

// boost
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>

// agg
#include "agg_trans_affine.h"

namespace mapnik
{
template <typename T>
box2d<T>::box2d()
    :minx_(0),miny_(0),maxx_(-1),maxy_(-1) {}

template <typename T>
box2d<T>::box2d(T minx_,T miny_,T maxx_,T maxy_)
{
    init(minx_,miny_,maxx_,maxy_);
}

template <typename T>
box2d<T>::box2d(const coord<T,2> &c0,const coord<T,2> &c1)
{
    init(c0.x,c0.y,c1.x,c1.y);
}

template <typename T>
box2d<T>::box2d(const box2d &rhs)
    : minx_(rhs.minx_),
      miny_(rhs.miny_),
      maxx_(rhs.maxx_),
      maxy_(rhs.maxy_) {}
// copy rather than init so dfl ctor (0,0,-1,-1) is not modified
// https://github.com/mapnik/mapnik/issues/749
/*{
  init(rhs.minx_,rhs.miny_,rhs.maxx_,rhs.maxy_);
  }*/

template <typename T>
box2d<T>::box2d(const box2d_type &rhs, const agg::trans_affine& tr)
{
    double x0 = rhs.minx_, y0 = rhs.miny_;
    double x1 = rhs.maxx_, y1 = rhs.miny_;
    double x2 = rhs.maxx_, y2 = rhs.maxy_;
    double x3 = rhs.minx_, y3 = rhs.maxy_;
    tr.transform(&x0, &y0);
    tr.transform(&x1, &y1);
    tr.transform(&x2, &y2);
    tr.transform(&x3, &y3);
    init(x0, y0, x2, y2);
    expand_to_include(x1, y1);
    expand_to_include(x3, y3);
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
bool box2d<T>::operator==(const box2d<T>& other) const
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
T box2d<T>::minx() const
{
    return minx_;
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
T box2d<T>::maxx() const
{
    return maxx_;
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
T box2d<T>::miny() const
{
    return miny_;
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
T box2d<T>::maxy() const
{
    return maxy_;
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
T box2d<T>::width() const
{
    return maxx_-minx_;
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
T box2d<T>::height() const
{
    return maxy_-miny_;
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
void box2d<T>::width(T w)
{
    T cx=center().x;
    minx_=static_cast<T>(cx-w*0.5);
    maxx_=static_cast<T>(cx+w*0.5);
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
void box2d<T>::height(T h)
{
    T cy=center().y;
    miny_=static_cast<T>(cy-h*0.5);
    maxy_=static_cast<T>(cy+h*0.5);
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
coord<T,2> box2d<T>::center() const
{
    return coord<T,2>(static_cast<T>(0.5*(minx_+maxx_)),
                      static_cast<T>(0.5*(miny_+maxy_)));
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
void box2d<T>::expand_to_include(const coord<T,2>& c)
{
    expand_to_include(c.x,c.y);
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
void box2d<T>::expand_to_include(T x,T y)
{
    if (x<minx_) minx_=x;
    if (x>maxx_) maxx_=x;
    if (y<miny_) miny_=y;
    if (y>maxy_) maxy_=y;
}

template <typename T>
void box2d<T>::expand_to_include(const box2d<T> &other)
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
bool box2d<T>::contains(const coord<T,2> &c) const
{
    return contains(c.x,c.y);
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
bool box2d<T>::contains(T x,T y) const
{
    return x>=minx_ && x<=maxx_ && y>=miny_ && y<=maxy_;
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
bool box2d<T>::contains(const box2d<T> &other) const
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
bool box2d<T>::intersects(const coord<T,2> &c) const
{
    return intersects(c.x,c.y);
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
bool box2d<T>::intersects(T x,T y) const
{
    return !(x>maxx_ || x<minx_ || y>maxy_ || y<miny_);
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
bool box2d<T>::intersects(const box2d<T> &other) const
{
    return !(other.minx_>maxx_ || other.maxx_<minx_ ||
             other.miny_>maxy_ || other.maxy_<miny_);
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
box2d<T> box2d<T>::intersect(const box2d_type& other) const
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
#if !defined(__SUNPRO_CC)
inline
#endif
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
#if !defined(__SUNPRO_CC)
inline
#endif
void box2d<T>::re_center(const coord<T,2> &c)
{
    re_center(c.x,c.y);
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
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
#if !defined(__SUNPRO_CC)
inline
#endif
void box2d<T>::clip(const box2d_type& other)
{
    minx_ = std::max(minx_,other.minx());
    miny_ = std::max(miny_,other.miny());
    maxx_ = std::min(maxx_,other.maxx());
    maxy_ = std::min(maxy_,other.maxy());
}


template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
bool box2d<T>::from_string(std::string const& s)
{
    unsigned i = 0;
    double d[4];
    bool success = false;
    boost::char_separator<char> sep(", ");
    boost::tokenizer<boost::char_separator<char> > tok(s, sep);
    for (boost::tokenizer<boost::char_separator<char> >::iterator beg = tok.begin();
         beg != tok.end(); ++beg)
    {
        std::string item(*beg);
        boost::trim(item);
        // note: we intentionally do not use mapnik::util::conversions::string2double
        // here to ensure that shapeindex can statically compile mapnik::box2d without
        // needing to link to libmapnik
        std::string::const_iterator str_beg = item.begin();
        std::string::const_iterator str_end = item.end();
        bool r = boost::spirit::qi::phrase_parse(str_beg,str_end,boost::spirit::qi::double_,boost::spirit::ascii::space,d[i]);
        if (!(r && (str_beg == str_end)))
        {
            break;
        }
        if (i == 3)
        {
            success = true;
            break;
        }
        ++i;
    }

    if (success)
    {
        init(static_cast<T>(d[0]),static_cast<T>(d[1]),static_cast<T>(d[2]),static_cast<T>(d[3]));
    }

    return success;
}

template <typename T>
#if !defined(__SUNPRO_CC)
inline
#endif
bool box2d<T>::valid() const
{
    return (minx_ <= maxx_ && miny_ <= maxy_) ;
}

template <typename T>
box2d<T>&  box2d<T>::operator+=(box2d<T> const& other)
{
    expand_to_include(other);
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
    init(x0, y0, x2, y2);
    expand_to_include(x1, y1);
    expand_to_include(x3, y3);
    return *this;
}

template class box2d<int>;
template class box2d<double>;
}
