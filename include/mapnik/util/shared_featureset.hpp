/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2026 Artem Pavlenko
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

#ifndef MAPNIK_SHARED_FEATURESET_HPP
#define MAPNIK_SHARED_FEATURESET_HPP

#include <mapnik/featureset.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace mapnik {

// One render's feature stream, replayed by independent layer/style readers.
class shared_featureset
{
  public:
    explicit shared_featureset(featureset_ptr source)
        : source_(std::move(source))
    {}

    feature_ptr at(std::size_t pos)
    {
        if (pos < features_.size())
            return features_[pos];
        if (source_)
        {
            if (auto feature = source_->next())
            {
                features_.push_back(feature);
                return feature;
            }
            source_.reset();
        }
        return {};
    }

  private:
    featureset_ptr source_;
    std::vector<feature_ptr> features_;
};

class shared_featureset_reader : public Featureset
{
  public:
    explicit shared_featureset_reader(std::shared_ptr<shared_featureset> cache)
        : cache_(std::move(cache))
    {}

    feature_ptr next() override
    {
        if (cache_)
        {
            if (auto feature = cache_->at(pos_))
            {
                ++pos_;
                return feature;
            }
            // Completed materials must not retain the cache until the map finishes.
            cache_.reset();
        }
        return {};
    }

  private:
    std::shared_ptr<shared_featureset> cache_;
    std::size_t pos_ = 0;
};

} // namespace mapnik

#endif
