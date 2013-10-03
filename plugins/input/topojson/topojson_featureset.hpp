/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#ifndef TOPOJSON_FEATURESET_HPP
#define TOPOJSON_FEATURESET_HPP

#include <mapnik/feature.hpp>
#include "topojson_datasource.hpp"

#include <vector>
#include <deque>

class topojson_featureset : public mapnik::Featureset
{
public:
    topojson_featureset(mapnik::topojson::topology const& topo,
                       std::deque<std::size_t>::const_iterator index_itr,
                       std::deque<std::size_t>::const_iterator index_end);
    virtual ~topojson_featureset();
    mapnik::feature_ptr next();

private:
    mapnik::context_ptr ctx_;
    mapnik::box2d<double> box_;
    mapnik::topojson::topology const& topo_;
    std::deque<std::size_t>::const_iterator index_itr_;
    std::deque<std::size_t>::const_iterator index_end_;
    std::size_t feature_id_;
};

#endif // TOPOJSON_FEATURESET_HPP
