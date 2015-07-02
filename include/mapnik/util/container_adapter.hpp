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

#ifndef CONTAINER_ADAPTER_HPP
#define CONTAINER_ADAPTER_HPP

// mapnik


#include <mapnik/util/path_iterator.hpp>

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include <boost/spirit/include/support_container.hpp>
#pragma GCC diagnostic pop

namespace boost { namespace spirit { namespace traits {

template <>
struct is_container<mapnik::path_type const> : mpl::true_ {} ;

template <>
struct is_container<mapnik::vertex_adapter const> : mpl::true_ {} ;

// make gcc and darwin toolsets happy.
// FIXME
//template <>
//struct is_container<mapnik::geometry_container const> : mpl::false_ {} ;

//
template <>
struct container_iterator<mapnik::path_type const>
{
    using type = mapnik::util::path_iterator<mapnik::path_type>;
};

template <>
struct container_iterator<mapnik::vertex_adapter const>
{
    using type = mapnik::util::path_iterator<mapnik::vertex_adapter>;
};

template <>
struct begin_container<mapnik::path_type const>
{
    static mapnik::util::path_iterator<mapnik::path_type>
    call (mapnik::path_type const& p)
    {
        return mapnik::util::path_iterator<mapnik::path_type>(p);
    }
};

template <>
struct begin_container<mapnik::vertex_adapter const>
{
    static mapnik::util::path_iterator<mapnik::vertex_adapter>
    call (mapnik::vertex_adapter const& p)
    {
        return mapnik::util::path_iterator<mapnik::vertex_adapter>(p);
    }
};

template <>
struct end_container<mapnik::path_type const>
{
    static mapnik::util::path_iterator<mapnik::path_type>
    call (mapnik::path_type const&)
    {
        return mapnik::util::path_iterator<mapnik::path_type>();
    }
};

template <>
struct end_container<mapnik::vertex_adapter const>
{
    static mapnik::util::path_iterator<mapnik::vertex_adapter>
    call (mapnik::vertex_adapter const&)
    {
        return mapnik::util::path_iterator<mapnik::vertex_adapter>();
    }
};

}}}

#endif // CONTAINER_ADAPTER_HPP
