/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
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
#include <mapnik/safe_cast.hpp>

// stl
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <mapnik/config.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/include/adapt_adt.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_adapt_adt_attributes.hpp>
#pragma GCC diagnostic pop

// agg
#include "agg_trans_affine.h"

BOOST_FUSION_ADAPT_TPL_ADT(
    (T),
    (mapnik::box2d)(T),
    (T, T, obj.minx(), obj.set_minx(mapnik::safe_cast<T>(val)))
    (T, T, obj.miny(), obj.set_miny(mapnik::safe_cast<T>(val)))
    (T, T, obj.maxx(), obj.set_maxx(mapnik::safe_cast<T>(val)))
    (T, T, obj.maxy(), obj.set_maxy(mapnik::safe_cast<T>(val))))

namespace mapnik {

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
bool box2d<T>::from_string(std::string const& str)
{
    boost::spirit::qi::lit_type lit;
    boost::spirit::qi::double_type double_;
    boost::spirit::ascii::space_type space;
    bool r = boost::spirit::qi::phrase_parse(str.begin(),
                                             str.end(),
                                             double_ >> -lit(',') >> double_ >> -lit(',') >> double_ >> -lit(',') >> double_,
                                             space,
                                             *this);
    return r;
}

template <typename T>
std::string box2d<T>::to_string() const
{
    std::ostringstream s;
    if (valid())
    {
        s << "box2d(" << std::fixed << std::setprecision(16)
          << minx_ << ',' << miny_ << ','
          << maxx_ << ',' << maxy_ << ')';
    }
    else
    {
        s << "box2d(INVALID)";
    }
    return s.str();
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
}
