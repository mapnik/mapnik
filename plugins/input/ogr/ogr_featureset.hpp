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

#ifndef OGR_FEATURESET_HPP
#define OGR_FEATURESET_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/geom_util.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <ogrsf_frmts.h>
MAPNIK_DISABLE_WARNING_POP

class ogr_featureset : public mapnik::Featureset
{
  public:
    ogr_featureset(mapnik::context_ptr const& ctx, OGRLayer& layer, OGRGeometry& extent, std::string const& encoding);

    ogr_featureset(mapnik::context_ptr const& ctx,
                   OGRLayer& layer,
                   mapnik::box2d<double> const& extent,
                   std::string const& encoding);

    virtual ~ogr_featureset();
    mapnik::feature_ptr next();

  private:
    mapnik::context_ptr ctx_;
    OGRLayer& layer_;
    OGRFeatureDefn* layerdef_;
    std::unique_ptr<mapnik::transcoder> const tr_;
    char const* fidcolumn_;
    mutable int count_;
};

#endif // OGR_FEATURESET_HPP
