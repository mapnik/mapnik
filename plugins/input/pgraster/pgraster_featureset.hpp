/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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
 *****************************************************************************
 *
 * Initially developed by Sandro Santilli <strk@keybit.net> for CartoDB
 *
 *****************************************************************************/

#ifndef PGRASTER_FEATURESET_HPP
#define PGRASTER_FEATURESET_HPP

// mapnik
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>

// stl
#include <memory>

using mapnik::box2d;
using mapnik::context_ptr;
using mapnik::feature_ptr;
using mapnik::Featureset;
using mapnik::raster_ptr;
using mapnik::transcoder;

class IResultSet;

class pgraster_featureset : public mapnik::Featureset
{
  public:
    /// @param bandno band number (1-based). 0 (default) reads all bands.
    ///               Anything else forces interpretation of colors off
    ///               (values copied verbatim)
    pgraster_featureset(std::shared_ptr<IResultSet> const& rs,
                        context_ptr const& ctx,
                        std::string const& encoding,
                        bool key_field = false,
                        int bandno = 0);
    feature_ptr next();
    ~pgraster_featureset();

  private:
    std::shared_ptr<IResultSet> rs_;
    context_ptr ctx_;
    const std::unique_ptr<mapnik::transcoder> tr_;
    mapnik::value_integer feature_id_;
    bool key_field_;
    int band_;
};

#endif // PGRASTER_FEATURESET_HPP
