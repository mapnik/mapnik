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

#ifndef OGR_INDEX_FEATURESET_HPP
#define OGR_INDEX_FEATURESET_HPP

#include <set>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include "ogr_featureset.hpp"

template <typename filterT>
class ogr_index_featureset : public mapnik::Featureset
{
public:
    ogr_index_featureset(OGRDataSource& dataset,
                         OGRLayer& layer,
                         const filterT& filter,
                         const std::string& index_file,
                         const std::string& encoding,
                         const bool multiple_geometries);
    virtual ~ogr_index_featureset();
    mapnik::feature_ptr next();

private:
    ogr_index_featureset(const ogr_index_featureset&);
    ogr_index_featureset& operator=(const ogr_index_featureset&);

    OGRDataSource& dataset_;
    OGRLayer& layer_;
    OGRFeatureDefn* layerdef_;
    filterT filter_;
    std::vector<int> ids_;
    std::vector<int>::iterator itr_;
    boost::scoped_ptr<mapnik::transcoder> tr_;
    const char* fidcolumn_;
    bool multiple_geometries_;
};

#endif // OGR_INDEX_FEATURESET_HPP
