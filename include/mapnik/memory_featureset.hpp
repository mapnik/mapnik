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

#ifndef MAPNIK_MEMORY_FEATURESET_HPP
#define MAPNIK_MEMORY_FEATURESET_HPP

// mapnik
#include <mapnik/memory_datasource.hpp>

// boost
#include <boost/utility.hpp>

namespace mapnik {

class memory_featureset : public Featureset
{
public:
    memory_featureset(box2d<double> const& bbox, memory_datasource const& ds)
        : bbox_(bbox),
          pos_(ds.features_.begin()),
          end_(ds.features_.end()),
          type_(ds.type())
    {}

    memory_featureset(box2d<double> const& bbox, std::vector<feature_ptr> const& features)
        : bbox_(bbox),
          pos_(features.begin()),
          end_(features.end()),
          type_(datasource::Vector)
    {}

    virtual ~memory_featureset() {}

    feature_ptr next()
    {
        while (pos_ != end_)
        {
            if (type_ == datasource::Raster)
            {
                return *pos_++;
            }
            else
            {
                for (unsigned i=0; i<(*pos_)->num_geometries();++i)
                {
                    geometry_type & geom = (*pos_)->get_geometry(i);
                    if (bbox_.intersects(geom.envelope()))
                    {
                        return *pos_++;
                    }
                }
            }
            ++pos_;
        }
        return feature_ptr();
    }

private:
    box2d<double> bbox_;
    std::vector<feature_ptr>::const_iterator pos_;
    std::vector<feature_ptr>::const_iterator end_;
    datasource::datasource_t type_;
};
}

#endif // MAPNIK_MEMORY_FEATURESET_HPP
