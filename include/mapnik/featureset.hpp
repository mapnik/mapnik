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

#ifndef MAPNIK_FEATURESET_HPP
#define MAPNIK_FEATURESET_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/util/noncopyable.hpp>

// boost
#include <memory>

namespace mapnik {

class feature_impl;
using feature_ptr = std::shared_ptr<feature_impl>;

struct MAPNIK_DECL Featureset : private util::noncopyable
{
    virtual feature_ptr next() = 0;
    virtual ~Featureset() {}
};

struct MAPNIK_DECL invalid_featureset final : Featureset
{
    feature_ptr next() { return feature_ptr(); }
    ~invalid_featureset() {}
};

using featureset_ptr = std::shared_ptr<Featureset>;

inline featureset_ptr make_invalid_featureset()
{
    return std::make_shared<invalid_featureset>();
}

inline bool is_valid(featureset_ptr const& ptr)
{
    return (dynamic_cast<invalid_featureset*>(ptr.get()) == nullptr) ? true : false;
}

} // namespace mapnik

#endif // MAPNIK_FEATURESET_HPP
