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

#ifndef MAPNIK_FEATURE_FACTORY_HPP
#define MAPNIK_FEATURE_FACTORY_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/value_types.hpp>

// boost
//#include <boost/pool/pool_alloc.hpp>

namespace mapnik
{
struct feature_factory
{
    static std::shared_ptr<feature_impl> create (context_ptr const& ctx, mapnik::value_integer fid)
    {
        //return boost::allocate_shared<feature_impl>(boost::pool_allocator<feature_impl>(),fid);
        //return boost::allocate_shared<feature_impl>(boost::fast_pool_allocator<feature_impl>(),fid);
        return std::make_shared<feature_impl>(ctx,fid);
    }
};
}

#endif // MAPNIK_FEATURE_FACTORY_HPP
