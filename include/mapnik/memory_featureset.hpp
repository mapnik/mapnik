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

#ifndef MAPNIK_MEMORY_FEATURESET_HPP
#define MAPNIK_MEMORY_FEATURESET_HPP

// mapnik
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/envelope.hpp>
#include <mapnik/featureset.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/raster.hpp>

#include <deque>

namespace mapnik {

class memory_featureset : public Featureset
{
  public:
    memory_featureset(box2d<double> const& bbox, memory_datasource const& ds, bool bbox_check = true)
        : bbox_(bbox),
          pos_(ds.features_.begin()),
          end_(ds.features_.end()),
          type_(ds.type()),
          bbox_check_(bbox_check)
    {}

    memory_featureset(box2d<double> const& bbox, std::deque<feature_ptr> const& features, bool bbox_check = true)
        : bbox_(bbox),
          pos_(features.begin()),
          end_(features.end()),
          type_(datasource::Vector),
          bbox_check_(bbox_check)
    {}

    virtual ~memory_featureset() {}

    feature_ptr next()
    {
        while (pos_ != end_)
        {
            if (!bbox_check_)
            {
                return *pos_++;
            }
            else
            {
                if (type_ == datasource::Raster)
                {
                    raster_ptr const& source = (*pos_)->get_raster();
                    if (source && bbox_.intersects(source->ext_))
                    {
                        return *pos_++;
                    }
                }
                else
                {
                    geometry::geometry<double> const& geom = (*pos_)->get_geometry();
                    if (bbox_.intersects(geometry::envelope(geom)))
                    {
                        return *pos_++;
                    }
                }
                ++pos_;
            }
        }
        return feature_ptr();
    }

  private:
    box2d<double> bbox_;
    std::deque<feature_ptr>::const_iterator pos_;
    std::deque<feature_ptr>::const_iterator end_;
    datasource::datasource_t type_;
    bool bbox_check_;
};
} // namespace mapnik

#endif // MAPNIK_MEMORY_FEATURESET_HPP
