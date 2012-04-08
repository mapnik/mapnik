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

#ifndef OGR_LAYER_PTR_HPP
#define OGR_LAYER_PTR_HPP

// mapnik
#include <mapnik/debug.hpp>

// stl
#include <iostream>
#include <fstream>
#include <stdexcept>

// ogr
#include <ogrsf_frmts.h>

class ogr_layer_ptr
{
public:
    ogr_layer_ptr()
        : datasource_(NULL),
          layer_(NULL),
          owns_layer_(false),
          is_valid_(false)
    {
    }

    ~ogr_layer_ptr()
    {
        free_layer();
    }

    void free_layer()
    {
        if (owns_layer_ && layer_ != NULL && datasource_ != NULL)
        {
            datasource_->ReleaseResultSet(layer_);
        }

        datasource_ = NULL;
        layer_ = NULL;
        layer_name_ = "";
        owns_layer_ = false;
        is_valid_ = false;
    }

    void layer_by_name(OGRDataSource* const datasource,
                       const std::string& layer_name)
    {
        free_layer();

        datasource_ = datasource;

        OGRLayer* ogr_layer = datasource_->GetLayerByName(layer_name.c_str());
        if (ogr_layer)
        {
            layer_name_ = layer_name;
            layer_ = ogr_layer;
            is_valid_ = true;

#ifdef MAPNIK_LOG
            mapnik::log() << "ogr_layer_ptr: layer_from_name layer=" << layer_name_;
#endif
        }

#ifdef MAPNIK_LOG
        debug_print_last_error();
#endif
    }

    void layer_by_index(OGRDataSource* const datasource,
                        int layer_index)
    {
        free_layer();

        datasource_ = datasource;

        OGRLayer* ogr_layer = datasource_->GetLayer(layer_index);
        if (ogr_layer)
        {
            OGRFeatureDefn* def = ogr_layer->GetLayerDefn();
            if (def != 0)
            {
                layer_ = ogr_layer;
                layer_name_ = def->GetName();
                is_valid_ = true;

#ifdef MAPNIK_LOG
                mapnik::log() << "ogr_layer_ptr: layer_from_index layer=" << layer_name_;
#endif
            }
        }

#ifdef MAPNIK_LOG
        debug_print_last_error();
#endif
    }

    void layer_by_sql(OGRDataSource* const datasource,
                      const std::string& layer_sql)
    {
        free_layer();

        datasource_ = datasource;
        owns_layer_ = true;

        // TODO - actually filter fields!
        // http://trac.osgeo.org/gdal/wiki/rfc29_desired_fields
        // http://trac.osgeo.org/gdal/wiki/rfc28_sqlfunc

        OGRGeometry* spatial_filter = NULL;
        const char* sql_dialect = NULL;
        OGRLayer* ogr_layer = datasource_->ExecuteSQL(layer_sql.c_str(), spatial_filter, sql_dialect);

        if (ogr_layer)
        {
            OGRFeatureDefn* def = ogr_layer->GetLayerDefn();
            if (def != 0)
            {
                layer_ = ogr_layer;
                layer_name_ = def->GetName();
                is_valid_ = true;

#ifdef MAPNIK_LOG
                mapnik::log() << "ogr_layer_ptr: layer_from_sql layer=" << layer_name_;
#endif
            }
        }

#ifdef MAPNIK_LOG
        debug_print_last_error();
#endif
    }

    const std::string& layer_name() const
    {
        return layer_name_;
    }

    OGRLayer* layer() const
    {
        return layer_;
    }

    bool is_valid() const
    {
        return is_valid_;
    }

private:

#ifdef MAPNIK_LOG
    void debug_print_last_error()
    {
        if (! is_valid_)
        {
            const std::string err = CPLGetLastErrorMsg();
            if (err.size() == 0)
            {
                mapnik::log() << "ogr_layer_ptr: Error getting layer";
            }
            else
            {
                mapnik::log() << "ogr_layer_ptr: " << err;
            }
        }
    }
#endif

    OGRDataSource* datasource_;
    OGRLayer* layer_;
    std::string layer_name_;
    bool owns_layer_;
    bool is_valid_;
};

#endif // OGR_LAYER_PTR_HPP
