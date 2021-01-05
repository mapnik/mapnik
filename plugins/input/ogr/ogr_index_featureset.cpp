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

// mapnik
#include <mapnik/value/types.hpp>
#include <mapnik/global.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry/correct.hpp>

// boost
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#include <mapnik/mapped_memory_cache.hpp>
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
MAPNIK_DISABLE_WARNING_POP
#endif

// ogr
#include "ogr_index_featureset.hpp"
#include "ogr_converter.hpp"
#include "ogr_index.hpp"

#include <gdal_version.h>

using mapnik::query;
using mapnik::box2d;
using mapnik::feature_ptr;
using mapnik::geometry_utils;
using mapnik::transcoder;
using mapnik::feature_factory;

template <typename filterT>
ogr_index_featureset<filterT>::ogr_index_featureset(mapnik::context_ptr const & ctx,
                                                    OGRLayer & layer,
                                                    filterT const& filter,
                                                    std::string const& index_file,
                                                    std::string const& encoding)
    : ctx_(ctx),
      layer_(layer),
      layerdef_(layer.GetLayerDefn()),
      filter_(filter),
      tr_(new transcoder(encoding)),
      fidcolumn_(layer_.GetFIDColumn()),
      feature_envelope_()
{

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    boost::optional<mapnik::mapped_region_ptr> memory = mapnik::mapped_memory_cache::instance().find(index_file, true);
    if (memory)
    {
        boost::interprocess::ibufferstream file(static_cast<char*>((*memory)->get_address()),(*memory)->get_size());
        ogr_index<filterT,boost::interprocess::ibufferstream >::query(filter,file,ids_);
    }
#else
  #if defined (WINDOWS)
      std::ifstream file(mapnik::utf8_to_utf16(index_file), std::ios::in | std::ios::binary);
  #else
      std::ifstream file(index_file.c_str(), std::ios::in | std::ios::binary);
  #endif
      ogr_index<filterT,std::ifstream>::query(filter,file,ids_);
#endif

    std::sort(ids_.begin(),ids_.end());

    MAPNIK_LOG_DEBUG(ogr) << "ogr_index_featureset: Query size=" << ids_.size();

    itr_ = ids_.begin();

    // reset reading
    layer_.ResetReading();
}

template <typename filterT>
ogr_index_featureset<filterT>::~ogr_index_featureset() {}

template <typename filterT>
feature_ptr ogr_index_featureset<filterT>::next()
{
    while (itr_ != ids_.end())
    {
        int pos = *itr_++;
        layer_.SetNextByIndex (pos);

        OGRFeature *poFeature = layer_.GetNextFeature();
        if (poFeature == nullptr)
        {
            return feature_ptr();
        }

        // ogr feature ids start at 0, so add one to stay
        // consistent with other mapnik datasources that start at 1
        mapnik::value_integer feature_id = (poFeature->GetFID() + 1);
        feature_ptr feature(feature_factory::create(ctx_,feature_id));

        OGRGeometry* geom=poFeature->GetGeometryRef();
        if (geom && !geom->IsEmpty())
        {
            geom->getEnvelope(&feature_envelope_);
            if (!filter_.pass(mapnik::box2d<double>(feature_envelope_.MinX,feature_envelope_.MinY,
                                            feature_envelope_.MaxX,feature_envelope_.MaxY))) continue;
            auto geom_corrected = ogr_converter::convert_geometry(geom);
            mapnik::geometry::correct(geom_corrected);
            feature->set_geometry(std::move(geom_corrected));
        }
        else
        {
            MAPNIK_LOG_DEBUG(ogr) << "ogr_index_featureset: Feature with null geometry="
                << poFeature->GetFID();
            OGRFeature::DestroyFeature( poFeature );
            continue;
        }

        int fld_count = layerdef_->GetFieldCount();
        for (int i = 0; i < fld_count; i++)
        {
            OGRFieldDefn* fld = layerdef_->GetFieldDefn (i);
            OGRFieldType type_oid = fld->GetType ();
            std::string fld_name = fld->GetNameRef ();

            switch (type_oid)
            {
            case OFTInteger:
            {
                feature->put<mapnik::value_integer>(fld_name,poFeature->GetFieldAsInteger (i));
                break;
            }
#if GDAL_VERSION_MAJOR >= 2
            case OFTInteger64:
            {
                feature->put<mapnik::value_integer>( fld_name, poFeature->GetFieldAsInteger64(i));
                break;
            }
#endif

            case OFTReal:
            {
                feature->put(fld_name,poFeature->GetFieldAsDouble (i));
                break;
            }

            case OFTString:
            case OFTWideString:     // deprecated !
            {
                feature->put(fld_name,tr_->transcode(poFeature->GetFieldAsString (i)));
                break;
            }

            case OFTIntegerList:
#if GDAL_VERSION_MAJOR >= 2
            case OFTInteger64List:
#endif
            case OFTRealList:
            case OFTStringList:
            case OFTWideStringList: // deprecated !
            {
                MAPNIK_LOG_WARN(ogr) << "ogr_index_featureset: Unhandled type_oid=" << type_oid;
                break;
            }

            case OFTBinary:
            {
                MAPNIK_LOG_WARN(ogr) << "ogr_index_featureset: Unhandled type_oid=" << type_oid;
                //feature->put(name,feat->GetFieldAsBinary (i, size));
                break;
            }

            case OFTDate:
            case OFTTime:
            case OFTDateTime:       // unhandled !
            {
                MAPNIK_LOG_WARN(ogr) << "ogr_index_featureset: Unhandled type_oid=" << type_oid;
                break;
            }
            }
        }
        OGRFeature::DestroyFeature( poFeature );
        return feature;
    }

    return feature_ptr();
}

template class ogr_index_featureset<mapnik::filter_in_box>;
template class ogr_index_featureset<mapnik::filter_at_point>;
