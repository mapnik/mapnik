/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef OGR_INDEX_FEATURESET_HPP
#define OGR_INDEX_FEATURESET_HPP

#include <set>
#include <vector>

#include <mapnik/feature.hpp>
#include "ogr_featureset.hpp"

template <typename filterT>
class ogr_index_featureset : public mapnik::Featureset
{
public:
    ogr_index_featureset(mapnik::context_ptr const& ctx,
                         OGRLayer& layer,
                         filterT const& filter,
                         std::string const& index_file,
                         std::string const& encoding);

    virtual ~ogr_index_featureset();
    mapnik::feature_ptr next();
private:
    mapnik::context_ptr ctx_;
    OGRLayer& layer_;
    OGRFeatureDefn* layerdef_;
    filterT filter_;
    std::vector<int> ids_;
    std::vector<int>::iterator itr_;
    const std::unique_ptr<mapnik::transcoder> tr_;
    const char* fidcolumn_;
    OGREnvelope feature_envelope_;
};

#endif // OGR_INDEX_FEATURESET_HPP
