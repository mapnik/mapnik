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

#ifndef POSTGIS_FEATURESET_HPP
#define POSTGIS_FEATURESET_HPP

// mapnik
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>

using mapnik::box2d;
using mapnik::context_ptr;
using mapnik::feature_ptr;
using mapnik::Featureset;
using mapnik::transcoder;

class IResultSet;

class postgis_featureset : public mapnik::Featureset
{
  public:
    postgis_featureset(std::shared_ptr<IResultSet> const& rs,
                       context_ptr const& ctx,
                       std::string const& encoding,
                       bool key_field,
                       bool key_field_as_attribute,
                       bool twkb_encoding);
    feature_ptr next();
    ~postgis_featureset();

  private:
    std::shared_ptr<IResultSet> rs_;
    context_ptr ctx_;
    const std::unique_ptr<mapnik::transcoder> tr_;
    unsigned totalGeomSize_;
    mapnik::value_integer feature_id_;
    bool key_field_;
    bool key_field_as_attribute_;
    bool twkb_encoding_;
};

#endif // POSTGIS_FEATURESET_HPP
