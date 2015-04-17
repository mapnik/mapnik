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

#ifndef MAPNIK_GEOMETRY_TRANSFORM_HPP
#define MAPNIK_GEOMETRY_TRANSFORM_HPP

#include <mapnik/geometry.hpp>
#include <mapnik/geometry_adapters.hpp>
#include <boost/geometry/algorithms/transform.hpp>

namespace mapnik { namespace geometry { namespace detail {

template <typename Transformer, typename V>
struct geometry_transform
{
    geometry_transform(Transformer const& transformer)
        : transformer_(transformer) {}

    using result_type = geometry<V>;

    geometry<V> operator() (geometry_empty const& empty) const
    {
        return empty;
    }

    template <typename T>
    geometry<V> operator() (geometry_collection<T> const& collection) const
    {
        geometry_collection<V> collection_out;
        for (auto const& geom :  collection)
        {
            collection_out.push_back(std::move((*this)(geom)));
        }
        return collection_out;
    }

    template <typename T>
    geometry<V> operator() (geometry<T> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    template <typename T>
    geometry<V> operator() (point<T> const& geom) const
    {
        point<V> geom_transformed;
        if (!boost::geometry::transform(geom, geom_transformed, transformer_))
        {
            throw std::runtime_error("Can't transformm geometry");
        }
        return geom_transformed;
    }
    
    template <typename T>
    geometry<V> operator() (line_string<T> const& geom) const
    {
        line_string<V> geom_transformed;
        if (!boost::geometry::transform(geom, geom_transformed, transformer_))
        {
            throw std::runtime_error("Can't transformm geometry");
        }
        return geom_transformed;
    }
    
    template <typename T>
    geometry<V> operator() (polygon<T> const& geom) const
    {
        polygon<V> geom_transformed;
        if (!boost::geometry::transform(geom, geom_transformed, transformer_))
        {
            throw std::runtime_error("Can't transformm geometry");
        }
        return geom_transformed;
    }

    template <typename T>
    geometry<V> operator() (multi_point<T> const& geom) const
    {
        multi_point<V> geom_transformed;
        if (!boost::geometry::transform(geom, geom_transformed, transformer_))
        {
            throw std::runtime_error("Can't transformm geometry");
        }
        return geom_transformed;
    }
    
    template <typename T>
    geometry<V> operator() (multi_line_string<T> const& geom) const
    {
        multi_line_string<V> geom_transformed;
        if (!boost::geometry::transform(geom, geom_transformed, transformer_))
        {
            throw std::runtime_error("Can't transformm geometry");
        }
        return geom_transformed;
    }
    
    template <typename T>
    geometry<V> operator() (multi_polygon<T> const& geom) const
    {
        multi_polygon<V> geom_transformed;
        if (!boost::geometry::transform(geom, geom_transformed, transformer_))
        {
            throw std::runtime_error("Can't transformm geometry");
        }
        return geom_transformed;
    }
    
    /*template <typename T>
    geometry<V> operator() (T const& geom) const
    {
        using geometry_type = T;
        geometry_type geom_transformed;
        if (!boost::geometry::transform(geom, geom_transformed, transformer_))
        {
            throw std::runtime_error("Can't transformm geometry");
        }
        return geom_transformed;
    }*/
    Transformer const& transformer_;
};

} // ns detail

template <typename T0, typename T1, typename T2>
geometry<T0> transform(T1 const& geom, T2 const& transformer)
{
    return detail::geometry_transform<T2, T0>(transformer)(geom);
}

}}

#endif // MAPNIK_GEOMETRY_TRANSFORM_HPP
