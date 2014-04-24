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

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/feature_factory.hpp>

// ogr
#include "occi_featureset.hpp"

using mapnik::query;
using mapnik::box2d;
using mapnik::feature_ptr;
using mapnik::geometry_type;
using mapnik::geometry_utils;
using mapnik::transcoder;
using mapnik::datasource_exception;
using mapnik::feature_factory;

using oracle::occi::Connection;
using oracle::occi::Statement;
using oracle::occi::ResultSet;
using oracle::occi::StatelessConnectionPool;
using oracle::occi::MetaData;
using oracle::occi::SQLException;
using oracle::occi::Type;
using oracle::occi::Number;
using oracle::occi::Blob;

occi_featureset::occi_featureset(StatelessConnectionPool* pool,
                                 Connection* conn,
                                 mapnik::context_ptr const& ctx,
                                 std::string const& sqlstring,
                                 std::string const& encoding,
                                 bool use_connection_pool,
                                 bool use_wkb,
                                 unsigned prefetch_rows)
    : rs_(nullptr),
      tr_(new transcoder(encoding)),
      feature_id_(1),
      ctx_(ctx),
      use_wkb_(use_wkb)
{
    if (use_connection_pool)
    {
        conn_.set_pool(pool);
    }
    else
    {
        conn_.set_connection(conn, false);
    }

    try
    {
        rs_ = conn_.execute_query(sqlstring, prefetch_rows);
    }
    catch (SQLException &ex)
    {
        MAPNIK_LOG_ERROR(occi) << "OCCI Plugin: error processing " << sqlstring << " : " << ex.getMessage();

        rs_ = nullptr;
    }
}

occi_featureset::~occi_featureset()
{
}

feature_ptr occi_featureset::next()
{
    while (rs_ != nullptr && rs_->next() == oracle::occi::ResultSet::DATA_AVAILABLE)
    {
        feature_ptr feature(feature_factory::create(ctx_, feature_id_));

        if (use_wkb_)
        {
            Blob blob = rs_->getBlob(1);
            blob.open(oracle::occi::OCCI_LOB_READONLY);

            unsigned int size = blob.length();
            if (buffer_.size() < size)
            {
                buffer_.resize(size);
            }

            oracle::occi::Stream* instream = blob.getStream(1, 0);
            instream->readBuffer(buffer_.data(), size);
            blob.closeStream(instream);
            blob.close();

            if (! geometry_utils::from_wkb(feature->paths(), buffer_.data(), size))
            {
                continue;
            }
        }
        else
        {
            const std::unique_ptr<SDOGeometry> geom(dynamic_cast<SDOGeometry*>(rs_->getObject(1)));
            if (geom.get())
            {
                convert_geometry(geom.get(), feature);
            }
            else
            {
                continue;
            }
        }

        std::vector<MetaData> listOfColumns = rs_->getColumnListMetaData();

        for (unsigned int i = 1; i < listOfColumns.size(); ++i)
        {
            MetaData columnObj = listOfColumns[i];

            std::string fld_name = columnObj.getString(MetaData::ATTR_NAME);
            int type_oid = columnObj.getInt(MetaData::ATTR_DATA_TYPE);

            /*
              int type_code = columnObj.getInt(MetaData::ATTR_TYPECODE);
              if (type_code == OCCI_TYPECODE_OBJECT)
              {
              continue;
              }
            */

            switch (type_oid)
            {
            case oracle::occi::OCCIBOOL:
                feature->put(fld_name, (rs_->getInt(i + 1) != 0));
                break;
            case oracle::occi::OCCIINT:
            case oracle::occi::OCCIUNSIGNED_INT:
                feature->put(fld_name, static_cast<mapnik::value_integer>(rs_->getInt(i + 1)));
                break;
            case oracle::occi::OCCIFLOAT:
            case oracle::occi::OCCIBFLOAT:
                feature->put(fld_name, (double)rs_->getFloat(i + 1));
                break;
            case oracle::occi::OCCIDOUBLE:
            case oracle::occi::OCCIBDOUBLE:
            case oracle::occi::OCCINUMBER:
            case oracle::occi::OCCI_SQLT_NUM:
                feature->put(fld_name, rs_->getDouble(i + 1));
                break;
            case oracle::occi::OCCICHAR:
            case oracle::occi::OCCISTRING:
            case oracle::occi::OCCI_SQLT_AFC:
            case oracle::occi::OCCI_SQLT_AVC:
            case oracle::occi::OCCI_SQLT_CHR:
            case oracle::occi::OCCI_SQLT_LNG:
            case oracle::occi::OCCI_SQLT_LVC:
            case oracle::occi::OCCI_SQLT_STR:
            case oracle::occi::OCCI_SQLT_VCS:
            case oracle::occi::OCCI_SQLT_VNU:
            case oracle::occi::OCCI_SQLT_VBI:
            case oracle::occi::OCCI_SQLT_VST:
            case oracle::occi::OCCIROWID:
            case oracle::occi::OCCI_SQLT_RDD:
            case oracle::occi::OCCI_SQLT_RID:
            case oracle::occi::OCCIDATE:
            case oracle::occi::OCCI_SQLT_DAT:
            case oracle::occi::OCCI_SQLT_DATE:
            case oracle::occi::OCCI_SQLT_TIME:
            case oracle::occi::OCCI_SQLT_TIME_TZ:
            case oracle::occi::OCCITIMESTAMP:
            case oracle::occi::OCCI_SQLT_TIMESTAMP:
            case oracle::occi::OCCI_SQLT_TIMESTAMP_LTZ:
            case oracle::occi::OCCI_SQLT_TIMESTAMP_TZ:
                feature->put(fld_name, static_cast<mapnik::value_unicode_string>(tr_->transcode(rs_->getString(i + 1).c_str())));
                break;
            case oracle::occi::OCCIINTERVALDS:
            case oracle::occi::OCCIINTERVALYM:
            case oracle::occi::OCCI_SQLT_INTERVAL_YM:
            case oracle::occi::OCCI_SQLT_INTERVAL_DS:
            case oracle::occi::OCCIANYDATA:
            case oracle::occi::OCCIBLOB:
            case oracle::occi::OCCIBFILE:
            case oracle::occi::OCCIBYTES:
            case oracle::occi::OCCICLOB:
            case oracle::occi::OCCIVECTOR:
            case oracle::occi::OCCIMETADATA:
            case oracle::occi::OCCIPOBJECT:
            case oracle::occi::OCCIREF:
            case oracle::occi::OCCIREFANY:
            case oracle::occi::OCCISTREAM:
            case oracle::occi::OCCICURSOR:
            case oracle::occi::OCCI_SQLT_FILE:
            case oracle::occi::OCCI_SQLT_CFILE:
            case oracle::occi::OCCI_SQLT_REF:
            case oracle::occi::OCCI_SQLT_CLOB:
            case oracle::occi::OCCI_SQLT_BLOB:
            case oracle::occi::OCCI_SQLT_RSET:
                {
                    MAPNIK_LOG_WARN(occi) << "occi_featureset: Unsupported datatype "
                                          << occi_enums::resolve_datatype(type_oid)
                                          << " (type_oid=" << type_oid << ")";
                    break;
                }
            default: // shouldn't get here
                {
                    MAPNIK_LOG_WARN(occi) << "occi_featureset: Unknown datatype "
                                          << "(type_oid=" << type_oid << ")";
                    break;
                }
            }
        }

        ++feature_id_;

        return feature;
    }

    return feature_ptr();
}


void occi_featureset::convert_geometry(SDOGeometry* geom, feature_ptr feature)
{
    int gtype = (int)geom->getSdo_gtype();
    int dimensions = gtype / 1000;
    int lrsvalue = (gtype - dimensions * 1000) / 100;
    int geomtype = (gtype - dimensions * 1000 - lrsvalue * 100);

    const std::vector<Number>& elem_info = geom->getSdo_elem_info();
    const std::vector<Number>& ordinates = geom->getSdo_ordinates();
    const int ordinates_size = (int)ordinates.size();

    switch (geomtype)
    {
    case SDO_GTYPE_POINT:
    {
        SDOPointType* sdopoint = geom->getSdo_point();
        if (sdopoint && ! sdopoint->isNull())
        {
            std::unique_ptr<geometry_type> point = std::make_unique<geometry_type>(mapnik::geometry_type::types::Point);
            point->move_to(sdopoint->getX(), sdopoint->getY());
            feature->add_geometry(point.release());
        }
    }
    break;
    case SDO_GTYPE_LINE:
    {
        if (ordinates_size >= dimensions)
        {
            const bool is_single_geom = true;
            const bool is_point_type = false;
            convert_ordinates(feature,
                              mapnik::geometry_type::types::LineString,
                              elem_info,
                              ordinates,
                              dimensions,
                              is_single_geom,
                              is_point_type);
        }
    }
    break;
    case SDO_GTYPE_POLYGON:
    {
        if (ordinates_size >= dimensions)
        {
            const bool is_single_geom = true;
            const bool is_point_type = false;
            convert_ordinates(feature,
                              mapnik::geometry_type::types::Polygon,
                              elem_info,
                              ordinates,
                              dimensions,
                              is_single_geom,
                              is_point_type);
        }
    }
    break;
    case SDO_GTYPE_MULTIPOINT:
    {
        if (ordinates_size >= dimensions)
        {
            const bool is_single_geom = false;
            const bool is_point_type = true;
            convert_ordinates(feature,
                              mapnik::geometry_type::types::Point,
                              elem_info,
                              ordinates,
                              dimensions,
                              is_single_geom,
                              is_point_type);
        }
    }
    break;
    case SDO_GTYPE_MULTILINE:
    {
        if (ordinates_size >= dimensions)
        {
            const bool is_single_geom = false;
            const bool is_point_type = false;

            convert_ordinates(feature,
                              mapnik::geometry_type::types::LineString,
                              elem_info,
                              ordinates,
                              dimensions,
                              is_single_geom,
                              is_point_type);
        }
    }
    break;
    case SDO_GTYPE_MULTIPOLYGON:
    {
        if (ordinates_size >= dimensions)
        {
            const bool is_single_geom = false;
            const bool is_point_type = false;

            convert_ordinates(feature,
                              mapnik::geometry_type::types::Polygon,
                              elem_info,
                              ordinates,
                              dimensions,
                              is_single_geom,
                              is_point_type);
        }

    }
    break;
    case SDO_GTYPE_COLLECTION:
    {
        if (ordinates_size >= dimensions)
        {
            const bool is_single_geom = false;
            const bool is_point_type = false;

            convert_ordinates(feature,
                              mapnik::geometry_type::types::Polygon,
                              elem_info,
                              ordinates,
                              dimensions,
                              is_single_geom,
                              is_point_type);
        }
    }
    break;
    case SDO_GTYPE_UNKNOWN:
    default:
    {
        MAPNIK_LOG_WARN(occi) << "occi_featureset: Unknown oracle enum "
                              << occi_enums::resolve_gtype(geomtype)
                              << "(gtype=" << gtype << ")";
    }
    break;
    }
}

void occi_featureset::convert_ordinates(mapnik::feature_ptr feature,
                                        const mapnik::geometry_type::types& geom_type,
                                        const std::vector<Number>& elem_info,
                                        const std::vector<Number>& ordinates,
                                        const int dimensions,
                                        const bool is_single_geom,
                                        const bool is_point_geom)
{
    const int elem_size = elem_info.size();
    const int ord_size = ordinates.size();

    if (elem_size >= 0)
    {
        int offset = elem_info[0];
        int etype = elem_info[1];
        int interp = elem_info[2];

        if (! is_single_geom && elem_size > SDO_ELEM_INFO_SIZE)
        {
            geometry_type* geom = new geometry_type(geom_type);

            for (int i = SDO_ELEM_INFO_SIZE; i < elem_size; i+=3)
            {
                int next_offset = elem_info[i];
                int next_etype = elem_info[i + 1];
                int next_interp = elem_info[i + 2];
                bool is_linear_element = true;
                bool is_unknown_etype = false;
                mapnik::geometry_type::types gtype = mapnik::geometry_type::types::Point;

                switch (etype)
                {
                case SDO_ETYPE_POINT:
                    if (interp == SDO_INTERPRETATION_POINT)     {}
                    if (interp > SDO_INTERPRETATION_POINT)      {}
                    gtype = mapnik::geometry_type::types::Point;
                    break;

                case SDO_ETYPE_LINESTRING:
                    if (interp == SDO_INTERPRETATION_STRAIGHT)  {}
                    if (interp == SDO_INTERPRETATION_CIRCULAR)  {}
                    gtype = mapnik::geometry_type::types::LineString;
                    break;

                case SDO_ETYPE_POLYGON:
                case SDO_ETYPE_POLYGON_INTERIOR:
                    if (interp == SDO_INTERPRETATION_STRAIGHT)  {}
                    if (interp == SDO_INTERPRETATION_CIRCULAR)  {}
                    if (interp == SDO_INTERPRETATION_RECTANGLE) {}
                    if (interp == SDO_INTERPRETATION_CIRCLE)    {}
                    gtype = mapnik::geometry_type::types::Polygon;
                    break;

                case SDO_ETYPE_COMPOUND_LINESTRING:
                case SDO_ETYPE_COMPOUND_POLYGON:
                case SDO_ETYPE_COMPOUND_POLYGON_INTERIOR:
                    // interp = next ETYPE to consider
                    is_linear_element = false;
                    gtype = mapnik::geometry_type::types::Polygon;
                    break;

                case SDO_ETYPE_UNKNOWN:    // unknown
                default:
                    is_unknown_etype = true;
                    break;
                }

                if (is_unknown_etype)
                {
                    break;
                }

                if (is_linear_element)
                {
                    if (geom)
                    {
                        feature->add_geometry(geom);
                    }

                    geom = new geometry_type(gtype);
                    fill_geometry_type(geom,
                                       offset - 1,
                                       next_offset - 1,
                                       ordinates,
                                       dimensions,
                                       is_point_geom);
                }

                offset = next_offset;
                etype = next_etype;
                interp = next_interp;
            }

            if (geom)
            {
                feature->add_geometry(geom);
                geom = 0;
            }
        }
        else
        {
            geometry_type * geom = new geometry_type(geom_type);
            fill_geometry_type(geom,
                               offset - 1,
                               ord_size,
                               ordinates,
                               dimensions,
                               is_point_geom);

            feature->add_geometry(geom);
        }
    }
}

void occi_featureset::fill_geometry_type(geometry_type* geom,
                                         const int real_offset,
                                         const int next_offset,
                                         const std::vector<Number>& ordinates,
                                         const int dimensions,
                                         const bool is_point_geom)
{
    geom->move_to((double) ordinates[real_offset], (double) ordinates[real_offset + 1]);

    if (is_point_geom)
    {
        for (int p = real_offset + dimensions; p < next_offset; p += dimensions)
        {
            geom->move_to((double) ordinates[p], (double) ordinates[p + 1]);
        }
    }
    else
    {
        for (int p = real_offset + dimensions; p < next_offset; p += dimensions)
        {
            geom->line_to((double) ordinates[p], (double) ordinates[p + 1]);
        }
    }
}
