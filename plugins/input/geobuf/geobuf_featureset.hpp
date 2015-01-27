/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef GEOBUF_FEATURESET_HPP
#define GEOBUF_FEATURESET_HPP

#include <mapnik/feature.hpp>
#include "geobuf_datasource.hpp"

#include <vector>
#include <deque>
#include <cstdio>

class geobuf_featureset : public mapnik::Featureset
{
public:
    typedef std::deque<geobuf_datasource::item_type> array_type;
    geobuf_featureset(std::vector<mapnik::feature_ptr> const& features,
                      array_type && index_array);
    virtual ~geobuf_featureset();
    mapnik::feature_ptr next();

private:
    std::vector<mapnik::feature_ptr> const& features_;
    const array_type index_array_;
    array_type::const_iterator index_itr_;
    array_type::const_iterator index_end_;
    mapnik::context_ptr ctx_;
};

#endif // GEOBUF_FEATURESET_HPP
