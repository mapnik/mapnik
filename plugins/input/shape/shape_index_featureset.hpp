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

#ifndef SHAPE_INDEX_FEATURESET_HPP
#define SHAPE_INDEX_FEATURESET_HPP

// stl
#include <set>
#include <vector>

// mapnik
#include <mapnik/geom_util.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/value_types.hpp>

// boost

#include <boost/utility.hpp>

#include "shape_datasource.hpp"
#include "shape_io.hpp"

using mapnik::Featureset;
using mapnik::box2d;
using mapnik::feature_ptr;
using mapnik::context_ptr;

namespace mapnik { namespace detail
{
struct node
{
    node() = default;
    node(int offset_, int start_, int end_)
        : offset(offset_),
          start(start_),
          end(end_) {}
    int offset;
    int start;
    int end;
};
}} // ns

template <typename filterT>
class shape_index_featureset : public Featureset
{
public:
    shape_index_featureset(filterT const& filter,
                           std::unique_ptr<shape_io> && shape_ptr,
                           std::set<std::string> const& attribute_names,
                           std::string const& encoding,
                           std::string const& shape_name,
                           int row_limit);
    virtual ~shape_index_featureset();
    feature_ptr next();

private:
    filterT filter_;
    context_ptr ctx_;
    std::unique_ptr<shape_io> shape_ptr_;
    const std::unique_ptr<mapnik::transcoder> tr_;
    std::vector<mapnik::detail::node> offsets_;
    std::vector<mapnik::detail::node>::iterator itr_;
    std::vector<int> attr_ids_;
    mapnik::value_integer row_limit_;
    mutable int count_;
    mutable box2d<double> feature_bbox_;
};

#endif // SHAPE_INDEX_FEATURESET_HPP
