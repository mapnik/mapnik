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

#ifndef MAPNIK_SQLITE_FEATURESET_HPP
#define MAPNIK_SQLITE_FEATURESET_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/wkb.hpp>

// boost

#include <memory>

// sqlite
#include "sqlite_resultset.hpp"

class sqlite_featureset : public mapnik::Featureset
{
  public:
    sqlite_featureset(std::shared_ptr<sqlite_resultset> rs,
                      mapnik::context_ptr const& ctx,
                      std::string const& encoding,
                      mapnik::box2d<double> const& bbox,
                      mapnik::wkbFormat format,
                      bool twkb_encoding,
                      bool spatial_index,
                      bool using_subquery);
    virtual ~sqlite_featureset();
    mapnik::feature_ptr next();

  private:
    std::shared_ptr<sqlite_resultset> rs_;
    mapnik::context_ptr ctx_;
    const std::unique_ptr<mapnik::transcoder> tr_;
    mapnik::box2d<double> bbox_;
    mapnik::wkbFormat format_;
    bool twkb_encoding_;
    bool spatial_index_;
    bool using_subquery_;
};

#endif // MAPNIK_SQLITE_FEATURESET_HPP
