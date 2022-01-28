/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_UTIL_ROUNDING_CAST_HPP
#define MAPNIK_UTIL_ROUNDING_CAST_HPP

#include <boost/numeric/conversion/converter.hpp>

namespace mapnik {
namespace util {

template<typename Target, typename Source>
inline Target rounding_cast(Source arg)
{
    typedef boost::numeric::conversion_traits<Target, Source> Traits;

    typedef boost::numeric::
      converter<Target, Source, Traits, boost::numeric::def_overflow_handler, boost::numeric::RoundEven<Source>>
        Converter;

    return Converter::convert(arg);
}

} // namespace util
} // namespace mapnik

#endif // MAPNIK_UTIL_ROUNDING_CAST_HPP
