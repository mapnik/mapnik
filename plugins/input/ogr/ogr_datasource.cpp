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

#include "ogr_datasource.hpp"
#include "ogr_featureset.hpp"
#include "ogr_index_featureset.hpp"

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/timer.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/datasource_plugin.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <fstream>
#include <mutex>
#include <sstream>
#include <stdexcept>

#include "ogr_utils.hpp"

using mapnik::datasource;
using mapnik::parameters;

using mapnik::attribute_descriptor;
using mapnik::box2d;
using mapnik::coord2d;
using mapnik::datasource_exception;
using mapnik::featureset_ptr;
using mapnik::filter_at_point;
using mapnik::filter_in_box;
using mapnik::layer_descriptor;
using mapnik::query;

DATASOURCE_PLUGIN_IMPL(ogr_datasource_plugin, ogr_datasource);
DATASOURCE_PLUGIN_EXPORT(ogr_datasource_plugin);

void ogr_datasource_plugin::after_load() const
{
    // initialize ogr formats
    // NOTE: in GDAL >= 2.0 this is the same as GDALAllRegister()
    OGRRegisterAll();
}

void ogr_datasource_plugin::before_unload() const
{
    // initialize ogr formats
    // NOTE: in GDAL >= 2.0 this is the same as GDALDestroyDriverManager()
    OGRCleanupAll();
}

ogr_datasource::ogr_datasource(parameters const& params)
    : datasource(params)
    , extent_()
    , type_(datasource::Vector)
    , desc_(ogr_datasource::name(), *params.get<std::string>("encoding", "utf-8"))
    , indexed_(false)
{
    init(params);
}

ogr_datasource::~ogr_datasource()
{
    // free layer before destroying the datasource
    layer_.free_layer();
    GDALClose((GDALDatasetH)dataset_);
}

void ogr_datasource::init(mapnik::parameters const& params)
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "ogr_datasource::init");
#endif

    const auto file = params.get<std::string>("file");
    auto string = params.get<std::string>("string");
    if (!string)
        string = params.get<std::string>("inline");
    if (!file && !string)
    {
        throw datasource_exception("missing <file> or <string> parameter");
    }

    if (string)
    {
        dataset_name_ = *string;
    }
    else
    {
        const auto base = params.get<std::string>("base");
        if (base)
        {
            dataset_name_ = *base + "/" + *file;
        }
        else
        {
            dataset_name_ = *file;
        }
    }

    std::string driver = *params.get<std::string>("driver", "");
    std::vector<ogr_utils::option_ptr> open_options_map =
      ogr_utils::split_open_options(*params.get<std::string>("open_options", ""));
    char** open_options = ogr_utils::open_options_for_ogr(open_options_map);

    if (!driver.empty())
    {
        unsigned int nOpenFlags = GDAL_OF_READONLY | GDAL_OF_VECTOR;
        const char* papszAllowedDrivers[] = {driver.c_str(), nullptr};

        dataset_ = reinterpret_cast<gdal_dataset_type>(
          GDALOpenEx(dataset_name_.c_str(), nOpenFlags, papszAllowedDrivers, open_options, nullptr));
    }
    else
    {
        if (open_options[0] != nullptr)
        {
            throw datasource_exception("<open_options> parameter provided but <driver> is missing");
        }
        // open ogr driver
        dataset_ = reinterpret_cast<gdal_dataset_type>(OGROpen(dataset_name_.c_str(), false, nullptr));
    }

    if (!dataset_)
    {
        const std::string err = CPLGetLastErrorMsg();
        if (err.size() == 0)
        {
            throw datasource_exception("OGR Plugin: connection failed: " + dataset_name_ +
                                       " was not found or is not a supported format");
        }
        else
        {
            throw datasource_exception("OGR Plugin: " + err);
        }
    }

    // initialize layer
    const auto layer_by_name = params.get<std::string>("layer");
    const auto layer_by_index = params.get<mapnik::value_integer>("layer_by_index");
    const auto layer_by_sql = params.get<std::string>("layer_by_sql");

    int passed_parameters = 0;
    passed_parameters += layer_by_name.has_value() ? 1 : 0;
    passed_parameters += layer_by_index.has_value() ? 1 : 0;
    passed_parameters += layer_by_sql.has_value() ? 1 : 0;

    if (passed_parameters > 1)
    {
        throw datasource_exception("OGR Plugin: you can only select an ogr layer by name "
                                   "('layer' parameter), by number ('layer_by_index' parameter), "
                                   "or by sql ('layer_by_sql' parameter), "
                                   "do not supply 2 or more of them at the same time");
    }

    if (layer_by_name)
    {
        layer_name_ = *layer_by_name;
        layer_.layer_by_name(dataset_, layer_name_);
    }
    else if (layer_by_index)
    {
        int num_layers = dataset_->GetLayerCount();
        if (*layer_by_index >= num_layers)
        {
            std::ostringstream s;
            s << "OGR Plugin: only " << num_layers << " layer(s) exist, cannot find layer by index '" << *layer_by_index
              << "'";
            throw datasource_exception(s.str());
        }

        layer_.layer_by_index(dataset_, *layer_by_index);
        layer_name_ = layer_.layer_name();
    }
    else if (layer_by_sql)
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats_sql__(std::clog, "ogr_datasource::init(layer_by_sql)");
#endif

        layer_.layer_by_sql(dataset_, *layer_by_sql);
        layer_name_ = layer_.layer_name();
    }
    else
    {
        std::string s(
          "OGR Plugin: missing <layer> or <layer_by_index> or <layer_by_sql>  parameter, available layers are: ");

        unsigned num_layers = dataset_->GetLayerCount();
        bool layer_found = false;
        std::vector<std::string> layer_names;
        for (unsigned i = 0; i < num_layers; ++i)
        {
            OGRLayer* ogr_layer = dataset_->GetLayer(i);
            OGRFeatureDefn* ogr_layer_def = ogr_layer->GetLayerDefn();
            if (ogr_layer_def != 0)
            {
                layer_found = true;
                layer_names.push_back(std::string("'") + ogr_layer_def->GetName() + std::string("'"));
            }
        }

        if (!layer_found)
        {
            s += "None (no layers were found in dataset)";
        }
        else
        {
            s += boost::algorithm::join(layer_names, ", ");
        }

        throw datasource_exception(s);
    }

    if (!layer_.is_valid())
    {
        std::ostringstream s;
        s << "OGR Plugin: ";

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
    const auto ext = params.get<std::string>("extent");
    if (ext.has_value() && !ext->empty())
    {
        extent_.from_string(*ext);
    }
    else
    {
        OGREnvelope envelope;
        OGRErr e = layer->GetExtent(&envelope);
        if (e == OGRERR_FAILURE)
        {
            if (layer->GetFeatureCount() == 0)
            {
                MAPNIK_LOG_ERROR(ogr) << "could not determine extent, layer '" << layer->GetLayerDefn()->GetName()
                                      << "' appears to have no features";
            }
            else
            {
                std::ostringstream s;
                s << "OGR Plugin: Cannot determine extent for layer '" << layer->GetLayerDefn()->GetName()
                  << "'. Please provide a manual extent string (minx,miny,maxx,maxy).";
                throw datasource_exception(s.str());
            }
        }
        extent_.init(envelope.MinX, envelope.MinY, envelope.MaxX, envelope.MaxY);
    }

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

#if defined(_WIN32)
    std::ifstream index_file(mapnik::utf8_to_utf16(index_name_), std::ios::in | std::ios::binary);
#else
    std::ifstream index_file(index_name_.c_str(), std::ios::in | std::ios::binary);
#endif

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
    mapnik::progress_timer __stats2__(std::clog, "ogr_datasource::init(get_column_description)");
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
                case OFTInteger64:
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
                case OFTInteger64List:
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
    mapnik::parameters& extra_params = desc_.get_extra_parameters();
    OGRSpatialReference* srs_ref = layer->GetSpatialRef();
    char* srs_output = nullptr;
    if (srs_ref && srs_ref->exportToProj4(&srs_output) == OGRERR_NONE)
    {
        extra_params["proj4"] = mapnik::util::trim_copy(srs_output);
    }
    CPLFree(srs_output);
}

const char* ogr_datasource::name()
{
    return "ogr";
}

mapnik::datasource::datasource_t ogr_datasource::type() const
{
    return type_;
}

box2d<double> ogr_datasource::envelope() const
{
    return extent_;
}

std::optional<mapnik::datasource_geometry_t> ogr_datasource::get_geometry_type() const
{
    std::optional<mapnik::datasource_geometry_t> result;
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
                result = mapnik::datasource_geometry_t::Point;
                break;
            case wkbLinearRing:
            case wkbLineString:
            case wkbMultiLineString:
                result = mapnik::datasource_geometry_t::LineString;
                break;
            case wkbPolygon:
            case wkbMultiPolygon:
                result = mapnik::datasource_geometry_t::Polygon;
                break;
            case wkbGeometryCollection:
                result = mapnik::datasource_geometry_t::Collection;
                break;
            case wkbNone:
            case wkbUnknown: {
                // fallback to inspecting first actual geometry
                // TODO - csv and shapefile inspect first 4 features
                if (dataset_ && layer_.is_valid())
                {
                    layer = layer_.layer();
                    // only new either reset of setNext
                    // layer->ResetReading();
                    layer->SetNextByIndex(0);
                    OGRFeature* poFeature;
                    while ((poFeature = layer->GetNextFeature()) != nullptr)
                    {
                        OGRGeometry* geom = poFeature->GetGeometryRef();
                        if (geom && !geom->IsEmpty())
                        {
                            switch (wkbFlatten(geom->getGeometryType()))
                            {
                                case wkbPoint:
                                case wkbMultiPoint:
                                    result = mapnik::datasource_geometry_t::Point;
                                    break;
                                case wkbLinearRing:
                                case wkbLineString:
                                case wkbMultiLineString:
                                    result = mapnik::datasource_geometry_t::LineString;
                                    break;
                                case wkbPolygon:
                                case wkbMultiPolygon:
                                    result = mapnik::datasource_geometry_t::Polygon;
                                    break;
                                case wkbGeometryCollection:
                                    result = mapnik::datasource_geometry_t::Collection;
                                    break;
                                default:
                                    break;
                            }
                        }
                        OGRFeature::DestroyFeature(poFeature);
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
    return desc_;
}

void validate_attribute_names(query const& q, std::vector<attribute_descriptor> const& names)
{
    std::set<std::string> const& attribute_names = q.property_names();
    std::set<std::string>::const_iterator pos = attribute_names.begin();
    std::set<std::string>::const_iterator end_names = attribute_names.end();

    for (; pos != end_names; ++pos)
    {
        bool found_name = false;

        for (auto const& attr_info : names)
        {
            if (attr_info.get_name() == *pos)
            {
                found_name = true;
                break;
            }
        }

        if (!found_name)
        {
            std::ostringstream s;
            s << "OGR Plugin: no attribute named '" << *pos << "'. Valid attributes are: ";
            for (auto const& attr_info2 : names)
            {
                s << attr_info2.get_name() << std::endl;
            }
            throw mapnik::datasource_exception(s.str());
        }
    }
}

featureset_ptr ogr_datasource::features(query const& q) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "ogr_datasource::features");
#endif

    if (dataset_ && layer_.is_valid())
    {
        // First we validate query fields: https://github.com/mapnik/mapnik/issues/792

        std::vector<attribute_descriptor> const& desc_ar = desc_.get_descriptors();
        // feature context (schema)
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();

        for (auto const& attr_info : desc_ar)
        {
            ctx->push(attr_info.get_name()); // TODO only push query attributes
        }

        validate_attribute_names(q, desc_ar);

        OGRLayer* layer = layer_.layer();

        if (indexed_)
        {
            filter_in_box filter(q.get_bbox());

            return featureset_ptr(
              new ogr_index_featureset<filter_in_box>(ctx, *layer, filter, index_name_, desc_.get_encoding()));
        }
        else
        {
            return featureset_ptr(new ogr_featureset(ctx, *layer, q.get_bbox(), desc_.get_encoding()));
        }
    }

    return mapnik::make_empty_featureset();
}

featureset_ptr ogr_datasource::features_at_point(coord2d const& pt, double tol) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "ogr_datasource::features_at_point");
#endif

    if (dataset_ && layer_.is_valid())
    {
        std::vector<attribute_descriptor> const& desc_ar = desc_.get_descriptors();
        // feature context (schema)
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();

        for (auto const& attr_info : desc_ar)
        {
            ctx->push(attr_info.get_name()); // TODO only push query attributes
        }

        OGRLayer* layer = layer_.layer();

        if (indexed_)
        {
            filter_at_point filter(pt, tol);

            return featureset_ptr(
              new ogr_index_featureset<filter_at_point>(ctx, *layer, filter, index_name_, desc_.get_encoding()));
        }
        else
        {
            mapnik::box2d<double> bbox(pt, pt);
            bbox.pad(tol);
            return featureset_ptr(new ogr_featureset(ctx, *layer, bbox, desc_.get_encoding()));
        }
    }

    return mapnik::make_empty_featureset();
}
