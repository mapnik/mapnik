/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_FILTER_FEATURESET_HPP
#define MAPNIK_FILTER_FEATURESET_HPP

// mapnik
#include <mapnik/featureset.hpp>
#include <mapnik/feature.hpp>

namespace mapnik {

template <typename T>
class filter_featureset : public Featureset
{
    using filter_type = T;

public:
    filter_featureset(featureset_ptr const& fs, filter_type && filter)
        : fs_(fs), filter_(std::move(filter)) {}

    feature_ptr next()
    {
        feature_ptr feature = fs_->next();
        while (feature && !filter_.pass(*feature))
        {
            feature = fs_->next();
        }
        return feature;
    }

private:
    featureset_ptr fs_;
    filter_type filter_;
};
}

#endif // MAPNIK_FILTER_FEATURESET_HPP
