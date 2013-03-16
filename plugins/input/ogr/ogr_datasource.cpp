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

#include "ogr_datasource.hpp"
#include "ogr_featureset.hpp"
#include "ogr_index_featureset.hpp"

#include <gdal_version.h>

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/timer.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

// stl
#include <iostream>
#include <fstream>
#include <stdexcept>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(ogr_datasource)

using mapnik::box2d;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;
using mapnik::datasource_exception;
using mapnik::filter_in_box;
using mapnik::filter_at_point;


ogr_datasource::ogr_datasource(parameters const& params, bool bind)
    : datasource(params),
      extent_(),
      type_(datasource::Vector),
      desc_(*params_.get<std::string>("type"), *params_.get<std::string>("encoding", "utf-8")),
      indexed_(false)
{
    boost::optional<std::string> file = params.get<std::string>("file");
    boost::optional<std::string> string = params.get<std::string>("string");
    if (! file && ! string)
    {
        throw datasource_exception("missing <file> or <string> parameter");
    }

    if (string)
    {
        dataset_name_ = *string;
    }
    else
    {
        boost::optional<std::string> base = params.get<std::string>("base");
        if (base)
        {
            dataset_name_ = *base + "/" + *file;
        }
        else
        {
            dataset_name_ = *file;
        }
    }

    if (bind)
    {
        this->bind();
    }
}

ogr_datasource::~ogr_datasource()
{
    if (is_bound_)
    {
        // free layer before destroying the datasource
        layer_.free_layer();

        OGRDataSource::DestroyDataSource (dataset_);
    }
}

void ogr_datasource::bind() const
{
    if (is_bound_) return;

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "ogr_datasource::bind");
#endif

    // initialize ogr formats
    OGRRegisterAll();

    std::string driver = *params_.get<std::string>("driver","");

    if (! driver.empty())
    {
        OGRSFDriver * ogr_driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(driver.c_str());
        if (ogr_driver && ogr_driver != NULL)
        {
            dataset_ = ogr_driver->Open((dataset_name_).c_str(), FALSE);
        }

    }
    else
    {
        // open ogr driver
        dataset_ = OGRSFDriverRegistrar::Open((dataset_name_).c_str(), FALSE);
    }

    if (! dataset_)
    {
        const std::string err = CPLGetLastErrorMsg();
        if (err.size() == 0)
        {
            throw datasource_exception("OGR Plugin: connection failed: " + dataset_name_ + " was not found or is not a supported format");
        }
        else
        {
            throw datasource_exception("OGR Plugin: " + err);
        }
    }

    // initialize layer
    boost::optional<std::string> layer_by_name = params_.get<std::string>("layer");
    boost::optional<unsigned> layer_by_index = params_.get<unsigned>("layer_by_index");
    boost::optional<std::string> layer_by_sql = params_.get<std::string>("layer_by_sql");

    int passed_parameters = 0;
    passed_parameters += layer_by_name ? 1 : 0;
    passed_parameters += layer_by_index ? 1 : 0;
    passed_parameters += layer_by_sql ? 1 : 0;

    if (passed_parameters > 1)
    {
        throw datasource_exception("OGR Plugin: you can only select an ogr layer by name "
                                   "('layer' parameter), by number ('layer_by_index' parameter), "
                                   "or by sql ('layer_by_sql' parameter), "
                                   "do not supply 2 or more of them at the same time" );
    }

    if (layer_by_name)
    {
        layer_name_ = *layer_by_name;
        layer_.layer_by_name(dataset_, layer_name_);
    }
    else if (layer_by_index)
    {
        const unsigned num_layers = dataset_->GetLayerCount();
        if (*layer_by_index >= num_layers)
        {
            std::ostringstream s;
            s << "OGR Plugin: only ";
            s << num_layers;
            s << " layer(s) exist, cannot find layer by index '" << *layer_by_index << "'";

            throw datasource_exception(s.str());
        }

        layer_.layer_by_index(dataset_, *layer_by_index);
        layer_name_ = layer_.layer_name();
    }
    else if (layer_by_sql)
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats_sql__(std::clog, "ogr_datasource::bind(layer_by_sql)");
#endif

        layer_.layer_by_sql(dataset_, *layer_by_sql);
        layer_name_ = layer_.layer_name();
    }
    else
    {
        std::ostringstream s;
        s << "OGR Plugin: missing <layer> or <layer_by_index> or <layer_by_sql> "
          << "parameter, available layers are: ";

        unsigned num_layers = dataset_->GetLayerCount();
        bool layer_found = false;
        std::vector<std::string> layer_names;
        for (unsigned i = 0; i < num_layers; ++i )
        {
            OGRLayer* ogr_layer = dataset_->GetLayer(i);
            OGRFeatureDefn* ogr_layer_def = ogr_layer->GetLayerDefn();
            if (ogr_layer_def != 0)
            {
                layer_found = true;
                layer_names.push_back(std::string("'") + ogr_layer_def->GetName() + std::string("'"));
            }
        }

        if (! layer_found)
        {
            s << "None (no layers were found in dataset)";
        }
        else
        {
            s << boost::algorithm::join(layer_names,", ");
        }

        throw datasource_exception(s.str());
    }

    if (! layer_.is_valid())
    {
        std::ostringstream s("OGR Plugin: ");

        if (layer_by_name)
        {
            s << "cannot find layer by name '" << *layer_by_name;
        }
        else if (layer_by_index)
        {
            s << "cannot find layer by index number '" << *layer_by_index;
        }
        else if (layer_by_sql)
        {
            s << "cannot find layer by sql query '" << *layer_by_sql;
        }

        s << "' in dataset '" << dataset_name_ << "'";

        throw datasource_exception(s.str());
    }

    // work with real OGR layer
    OGRLayer* layer = layer_.layer();

    // initialize envelope
    OGREnvelope envelope;
    layer->GetExtent(&envelope);
    extent_.init(envelope.MinX, envelope.MinY, envelope.MaxX, envelope.MaxY);

    // scan for index file
    // TODO - layer names don't match dataset name, so this will break for
    // any layer types of ogr than shapefiles, etc
    // fix here and in ogrindex
    size_t breakpoint = dataset_name_.find_last_of(".");
    if (breakpoint == std::string::npos)
    {
        breakpoint = dataset_name_.length();
    }
    index_name_ = dataset_name_.substr(0, breakpoint) + ".ogrindex";

    std::ifstream index_file(index_name_.c_str(), std::ios::in | std::ios::binary);
    if (index_file)
    {
        indexed_ = true;
        index_file.close();
    }
#if 0
    // TODO - enable this warning once the ogrindex tool is a bit more stable/mature
    else
    {
        MAPNIK_LOG_DEBUG(ogr) << "ogr_datasource: no ogrindex file found for " << dataset_name_
                              << ", use the 'ogrindex' program to build an index for faster rendering";
    }
#endif

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats2__(std::clog, "ogr_datasource::bind(get_column_description)");
#endif

    // deal with attributes descriptions
    OGRFeatureDefn* def = layer->GetLayerDefn();
    if (def != 0)
    {
        const int fld_count = def->GetFieldCount();
        for (int i = 0; i < fld_count; i++)
        {
            OGRFieldDefn* fld = def->GetFieldDefn(i);

            const std::string fld_name = fld->GetNameRef();
            const OGRFieldType type_oid = fld->GetType();

            switch (type_oid)
            {
            case OFTInteger:
                desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Integer));
                break;

            case OFTReal:
                desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Double));
                break;

            case OFTString:
            case OFTWideString: // deprecated
                desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::String));
                break;

            case OFTBinary:
                desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Object));
                break;

            case OFTIntegerList:
            case OFTRealList:
            case OFTStringList:
            case OFTWideStringList: // deprecated !
                MAPNIK_LOG_WARN(ogr) << "ogr_datasource: Unhandled type_oid=" << type_oid;
                break;

            case OFTDate:
            case OFTTime:
            case OFTDateTime: // unhandled !
                desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Object));
                MAPNIK_LOG_WARN(ogr) << "ogr_datasource: Unhandled type_oid=" << type_oid;
                break;
            }
        }
    }

    is_bound_ = true;
}

const char * ogr_datasource::name()
{
    return "ogr";
}

mapnik::datasource::datasource_t ogr_datasource::type() const
{
    return type_;
}

box2d<double> ogr_datasource::envelope() const
{
    if (! is_bound_) bind();
    return extent_;
}

boost::optional<mapnik::datasource::geometry_t> ogr_datasource::get_geometry_type() const
{
    boost::optional<mapnik::datasource::geometry_t> result;
    if (dataset_ && layer_.is_valid())
    {
        OGRLayer* layer = layer_.layer();
        // NOTE: wkbFlatten macro in ogr flattens 2.5d types into base 2d type
#if GDAL_VERSION_NUM < 1800
        switch (wkbFlatten(layer->GetLayerDefn()->GetGeomType()))
#else
        switch (wkbFlatten(layer->GetGeomType()))
#endif
            {
            case wkbPoint:
            case wkbMultiPoint:
                result.reset(mapnik::datasource::Point);
                break;
            case wkbLinearRing:
            case wkbLineString:
            case wkbMultiLineString:
                result.reset(mapnik::datasource::LineString);
                break;
            case wkbPolygon:
            case wkbMultiPolygon:
                result.reset(mapnik::datasource::Polygon);
                break;
            case wkbGeometryCollection:
                result.reset(mapnik::datasource::Collection);
                break;
            case wkbNone:
            case wkbUnknown:
            {
                // fallback to inspecting first actual geometry
                // TODO - csv and shapefile inspect first 4 features
                if (dataset_ && layer_.is_valid())
                {
                    OGRLayer* layer = layer_.layer();
                    // only new either reset of setNext
                    //layer->ResetReading();
                    layer->SetNextByIndex(0);
                    OGRFeature *poFeature;
                    while ((poFeature = layer->GetNextFeature()) != NULL)
                    {
                        OGRGeometry* geom = poFeature->GetGeometryRef();
                        if (geom && ! geom->IsEmpty())
                        {
                            switch (wkbFlatten(geom->getGeometryType()))
                            {
                            case wkbPoint:
                            case wkbMultiPoint:
                                result.reset(mapnik::datasource::Point);
                                break;
                            case wkbLinearRing:
                            case wkbLineString:
                            case wkbMultiLineString:
                                result.reset(mapnik::datasource::LineString);
                                break;
                            case wkbPolygon:
                            case wkbMultiPolygon:
                                result.reset(mapnik::datasource::Polygon);
                                break;
                            case wkbGeometryCollection:
                                result.reset(mapnik::datasource::Collection);
                                break;
                            default:
                                break;
                            }
                        }
                        OGRFeature::DestroyFeature( poFeature );
                        break;
                    }
                }
                break;
            }
            default:
                break;
            }
    }

    return result;
}

layer_descriptor ogr_datasource::get_descriptor() const
{
    if (! is_bound_) bind();
    return desc_;
}

void validate_attribute_names(query const& q, std::vector<attribute_descriptor> const& names )
{
    std::set<std::string> const& attribute_names = q.property_names();
    std::set<std::string>::const_iterator pos = attribute_names.begin();
    std::set<std::string>::const_iterator end_names = attribute_names.end();

    for ( ;pos != end_names; ++pos)
    {
        bool found_name = false;

        std::vector<attribute_descriptor>::const_iterator itr = names.begin();
        std::vector<attribute_descriptor>::const_iterator end = names.end();

        for (; itr!=end; ++itr)
        {
            if (itr->get_name() == *pos)
            {
                found_name = true;
                break;
            }
        }

        if (! found_name)
        {
            std::ostringstream s;
            std::vector<attribute_descriptor>::const_iterator itr = names.begin();
            std::vector<attribute_descriptor>::const_iterator end = names.end();
            s << "OGR Plugin: no attribute '" << *pos << "'. Valid attributes are: ";
            for ( ;itr!=end;++itr)
            {
                s << itr->get_name() << std::endl;
            }
            throw mapnik::datasource_exception(s.str());
        }
    }
}

featureset_ptr ogr_datasource::features(query const& q) const
{
    if (! is_bound_) bind();

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "ogr_datasource::features");
#endif

    if (dataset_ && layer_.is_valid())
    {
        // First we validate query fields: https://github.com/mapnik/mapnik/issues/792

        std::vector<attribute_descriptor> const& desc_ar = desc_.get_descriptors();
        // feature context (schema)
        mapnik::context_ptr ctx = boost::make_shared<mapnik::context_type>();

        std::vector<attribute_descriptor>::const_iterator itr = desc_ar.begin();
        std::vector<attribute_descriptor>::const_iterator end = desc_ar.end();

        for (; itr!=end; ++itr) ctx->push(itr->get_name()); // TODO only push query attributes

        validate_attribute_names(q, desc_ar);

        OGRLayer* layer = layer_.layer();

        if (indexed_)
        {
            filter_in_box filter(q.get_bbox());

            return featureset_ptr(new ogr_index_featureset<filter_in_box>(ctx,
                                                                          *layer,
                                                                          filter,
                                                                          index_name_,
                                                                          desc_.get_encoding()));
        }
        else
        {
            return featureset_ptr(new ogr_featureset(ctx,
                                                      *layer,
                                                      q.get_bbox(),
                                                      desc_.get_encoding()));
        }
    }

    return featureset_ptr();
}

featureset_ptr ogr_datasource::features_at_point(coord2d const& pt, double tol) const
{
    if (!is_bound_) bind();

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "ogr_datasource::features_at_point");
#endif

    if (dataset_ && layer_.is_valid())
    {
        std::vector<attribute_descriptor> const& desc_ar = desc_.get_descriptors();
        // feature context (schema)
        mapnik::context_ptr ctx = boost::make_shared<mapnik::context_type>();

        std::vector<attribute_descriptor>::const_iterator itr = desc_ar.begin();
        std::vector<attribute_descriptor>::const_iterator end = desc_ar.end();
        for (; itr!=end; ++itr) ctx->push(itr->get_name());

        OGRLayer* layer = layer_.layer();

        if (indexed_)
        {
            filter_at_point filter(pt);

            return featureset_ptr(new ogr_index_featureset<filter_at_point> (ctx,
                                                                             *layer,
                                                                             filter,
                                                                             index_name_,
                                                                             desc_.get_encoding()));
        }
        else
        {
            OGRPoint point;
            point.setX (pt.x);
            point.setY (pt.y);

            return featureset_ptr(new ogr_featureset (ctx,
                                                      *layer,
                                                      point,
                                                      desc_.get_encoding()));
        }
    }

    return featureset_ptr();
}
