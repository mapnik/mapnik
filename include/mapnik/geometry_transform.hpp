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

template <typename V, typename T, typename Transformer>
inline point<V> transform_geometry(point<T> const& geom, Transformer const& transformer)
{
    point<V> geom_transformed;
    if (!boost::geometry::transform(geom, geom_transformed, transformer))
    {
        throw std::runtime_error("Can't transformm geometry");
    }
    return geom_transformed;
}

template <typename V, typename T, typename Transformer>
inline multi_point<V> transform_geometry(multi_point<T> const& geom, Transformer const& transformer)
{
    multi_point<V> geom_transformed;
    if (!boost::geometry::transform(geom, geom_transformed, transformer))
    {
        throw std::runtime_error("Can't transformm geometry");
    }
    return geom_transformed;
}

template <typename V, typename T, typename Transformer>
inline line_string<V> transform_geometry(line_string<T> const& geom, Transformer const& transformer)
{
    line_string<V> geom_transformed;
    if (!boost::geometry::transform(geom, geom_transformed, transformer))
    {
        throw std::runtime_error("Can't transformm geometry");
    }
    return geom_transformed;
}

template <typename V, typename T, typename Transformer>
inline multi_line_string<V> transform_geometry(multi_line_string<T> const& geom, Transformer const& transformer)
{
    multi_line_string<V> geom_transformed;
    if (!boost::geometry::transform(geom, geom_transformed, transformer))
    {
        throw std::runtime_error("Can't transformm geometry");
    }
    return geom_transformed;
}

template <typename V, typename T, typename Transformer>
inline polygon<V> transform_geometry(polygon<T> const& geom, Transformer const& transformer)
{
    polygon<V> geom_transformed;
    if (!boost::geometry::transform(geom, geom_transformed, transformer))
    {
        throw std::runtime_error("Can't transformm geometry");
    }
    return geom_transformed;
}

template <typename V, typename T, typename Transformer>
inline multi_polygon<V> transform_geometry(multi_polygon<T> const& geom, Transformer const& transformer)
{
    multi_polygon<V> geom_transformed;
    if (!boost::geometry::transform(geom, geom_transformed, transformer))
    {
        throw std::runtime_error("Can't transformm geometry");
    }
    return geom_transformed;
}

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
            collection_out.push_back((*this)(geom));
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
        return transform_geometry<V>(geom, transformer_);
    }

    template <typename T>
    geometry<V> operator() (line_string<T> const& geom) const
    {
        return transform_geometry<V>(geom, transformer_);
    }

    template <typename T>
    geometry<V> operator() (polygon<T> const& geom) const
    {
        return transform_geometry<V>(geom, transformer_);
    }

    template <typename T>
    geometry<V> operator() (multi_point<T> const& geom) const
    {
        return transform_geometry<V>(geom, transformer_);
    }

    template <typename T>
    geometry<V> operator() (multi_line_string<T> const& geom) const
    {
        return transform_geometry<V>(geom, transformer_);
    }

    template <typename T>
    geometry<V> operator() (multi_polygon<T> const& geom) const
    {
        return transform_geometry<V>(geom, transformer_);
    }

    Transformer const& transformer_;
};

} // ns detail

template <typename T0, typename T1, typename T2>
geometry<T0> transform(geometry<T1> const& geom, T2 const& transformer)
{
    return detail::geometry_transform<T2, T0>(transformer)(geom);
}

template <typename T0, typename T1, typename T2>
geometry<T0> transform(geometry_collection<T1> const& geom, T2 const& transformer)
{
    return detail::geometry_transform<T2, T0>(transformer)(geom);
}

template <typename T0, typename T1, typename T2>
point<T0> transform(point<T1> const& geom, T2 const& transformer)
{
    return detail::transform_geometry<T0>(geom, transformer);
}

template <typename T0, typename T1, typename T2>
multi_point<T0> transform(multi_point<T1> const& geom, T2 const& transformer)
{
    return detail::transform_geometry<T0>(geom, transformer);
}

template <typename T0, typename T1, typename T2>
line_string<T0> transform(line_string<T1> const& geom, T2 const& transformer)
{
    return detail::transform_geometry<T0>(geom, transformer);
}

template <typename T0, typename T1, typename T2>
multi_line_string<T0> transform(multi_line_string<T1> const& geom, T2 const& transformer)
{
    return detail::transform_geometry<T0>(geom, transformer);
}

template <typename T0, typename T1, typename T2>
polygon<T0> transform(polygon<T1> const& geom, T2 const& transformer)
{
    return detail::transform_geometry<T0>(geom, transformer);
}

template <typename T0, typename T1, typename T2>
multi_polygon<T0> transform(multi_polygon<T1> const& geom, T2 const& transformer)
{
    return detail::transform_geometry<T0>(geom, transformer);
}


}}

#endif // MAPNIK_GEOMETRY_TRANSFORM_HPP
