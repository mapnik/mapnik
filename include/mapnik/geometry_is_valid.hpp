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

#ifndef MAPNIK_GEOMETRY_IS_VALID_HPP
#define MAPNIK_GEOMETRY_IS_VALID_HPP

#include <boost/version.hpp>

// only Boost >= 1.58 contains the is_valid function
#if BOOST_VERSION >= 105800

#include <mapnik/geometry.hpp>
#include <mapnik/geometry_adapters.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/algorithms/validity_failure_type.hpp>

namespace mapnik { namespace geometry {

namespace detail {

struct geometry_is_valid
{
    using result_type = bool;

    template <typename T>
    result_type operator() (geometry<T> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    result_type operator() (geometry_empty const& ) const
    {
        return true;
    }

    template <typename T>
    result_type operator() (geometry_collection<T> const& collection) const
    {
        for (auto const& geom : collection)
        {
            if ( !(*this)(geom)) return false;
        }
        return true;
    }

    template <typename T>
    result_type operator() (point<T> const& pt) const
    {
        return boost::geometry::is_valid(pt);
    }

    template <typename T>
    result_type operator() (line_string<T> const& line) const
    {
        return boost::geometry::is_valid(line);
    }

    template <typename T>
    result_type operator() (polygon<T> const& poly) const
    {
        return boost::geometry::is_valid(poly);
    }

    template <typename T>
    result_type operator() (multi_point<T> const& multi_pt) const
    {
        return boost::geometry::is_valid(multi_pt);
    }

    template <typename T>
    result_type operator() (multi_line_string<T> const& multi_line) const
    {
        return boost::geometry::is_valid(multi_line);
    }

    template <typename T>
    result_type operator() (multi_polygon<T> const& multi_poly) const
    {
        return boost::geometry::is_valid(multi_poly);
    }
};

struct geometry_is_valid_reason
{
    using result_type = bool;
    
    boost::geometry::validity_failure_type & failure_;

    geometry_is_valid_reason(boost::geometry::validity_failure_type & failure):
        failure_(failure) {}

    template <typename T>
    result_type operator() (geometry<T> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    result_type operator() (geometry_empty const& ) const
    {
        failure_ = boost::geometry::no_failure;
        return true;
    }

    template <typename T>
    result_type operator() (geometry_collection<T> const& collection) const
    {
        for (auto const& geom : collection)
        {
            if ( !(*this)(geom)) return false;
        }
        return true;
    }

    template <typename T>
    result_type operator() (point<T> const& pt) const
    {
        return boost::geometry::is_valid(pt, failure_);
    }

    template <typename T>
    result_type operator() (line_string<T> const& line) const
    {
        return boost::geometry::is_valid(line, failure_);
    }

    template <typename T>
    result_type operator() (polygon<T> const& poly) const
    {
        return boost::geometry::is_valid(poly, failure_);
    }

    template <typename T>
    result_type operator() (multi_point<T> const& multi_pt) const
    {
        return boost::geometry::is_valid(multi_pt, failure_);
    }

    template <typename T>
    result_type operator() (multi_line_string<T> const& multi_line) const
    {
        return boost::geometry::is_valid(multi_line, failure_);
    }

    template <typename T>
    result_type operator() (multi_polygon<T> const& multi_poly) const
    {
        return boost::geometry::is_valid(multi_poly, failure_);
    }
};

struct geometry_is_valid_string
{
    using result_type = bool;
    
    std::string & message_;

    geometry_is_valid_string(std::string & message):
        message_(message) {}

    template <typename T>
    result_type operator() (geometry<T> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    result_type operator() (geometry_empty const& ) const
    {
        message_ = "Geometry is valid";
        return true;
    }

    template <typename T>
    result_type operator() (geometry_collection<T> const& collection) const
    {
        for (auto const& geom : collection)
        {
            if ( !(*this)(geom)) return false;
        }
        return true;
    }

    template <typename T>
    result_type operator() (point<T> const& pt) const
    {
        return boost::geometry::is_valid(pt, message_);
    }

    template <typename T>
    result_type operator() (line_string<T> const& line) const
    {
        return boost::geometry::is_valid(line, message_);
    }

    template <typename T>
    result_type operator() (polygon<T> const& poly) const
    {
        return boost::geometry::is_valid(poly, message_);
    }

    template <typename T>
    result_type operator() (multi_point<T> const& multi_pt) const
    {
        return boost::geometry::is_valid(multi_pt, message_);
    }

    template <typename T>
    result_type operator() (multi_line_string<T> const& multi_line) const
    {
        return boost::geometry::is_valid(multi_line, message_);
    }

    template <typename T>
    result_type operator() (multi_polygon<T> const& multi_poly) const
    {
        return boost::geometry::is_valid(multi_poly, message_);
    }
};


}

template <typename T>
inline bool is_valid(T const& geom)
{
    return detail::geometry_is_valid() (geom);
}

template <typename T>
inline bool is_valid(mapnik::geometry::geometry<T> const& geom)
{
    return util::apply_visitor(detail::geometry_is_valid(), geom);
}

template <typename T>
inline bool is_valid(T const& geom, boost::geometry::validity_failure_type & failure)
{
    return detail::geometry_is_valid_reason(failure) (geom);
}

template <typename T>
inline bool is_valid(mapnik::geometry::geometry<T> const& geom, 
                     boost::geometry::validity_failure_type & failure)
{
    return util::apply_visitor(detail::geometry_is_valid_reason(failure), geom);
}

template <typename T>
inline bool is_valid(T const& geom, std::string & message)
{
    return detail::geometry_is_valid_string(message) (geom);
}

template <typename T>
inline bool is_valid(mapnik::geometry::geometry<T> const& geom, 
                     std::string & message)
{
    return util::apply_visitor(detail::geometry_is_valid_string(message), geom);
}

}}

#endif // BOOST_VERSION >= 1.58
#endif // MAPNIK_GEOMETRY_IS_VALID_HPP
