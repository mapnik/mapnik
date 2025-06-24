/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_FEATURESET_BUFFER_HPP
#define MAPNIK_FEATURESET_BUFFER_HPP

// mapnik
#include <mapnik/featureset.hpp>

#include <vector>

namespace mapnik {

class featureset_buffer : public Featureset
{
  public:
    featureset_buffer()
        : features_()
        , pos_()
        , end_()
    {}

    virtual ~featureset_buffer() {}

    feature_ptr next()
    {
        if (pos_ != end_)
        {
            return *pos_++;
        }
        return feature_ptr();
    }

    void push(feature_ptr const& feature) { features_.push_back(feature); }

    void prepare()
    {
        pos_ = features_.begin();
        end_ = features_.end();
    }

    void clear() { features_.clear(); }

  private:
    std::vector<feature_ptr> features_;
    std::vector<feature_ptr>::iterator pos_;
    std::vector<feature_ptr>::iterator end_;
};

} // namespace mapnik

#endif // MAPNIK_FEATURESET_BUFFER_HPP
