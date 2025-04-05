// SPDX-License-Identifier: LGPL-2.1-or-later
/*****************************************************************************
 *
 * This file is part of Mapnik Vector Tile Plugin
 *
 * Copyright (C) 2023 Geofabrik GmbH
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

#include "mbtiles_vector_datasource.hpp"
#include "mbtiles_vector_featureset.hpp"
#include "vector_tile_projection.hpp"
#include <mapnik/geom_util.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/datasource_plugin.hpp>
#include <rapidjson/error/en.h>
#include <string>


DATASOURCE_PLUGIN_IMPL(mbtiles_vector_datasource_plugin, mbtiles_vector_datasource);
DATASOURCE_PLUGIN_EXPORT(mbtiles_vector_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_AFTER_LOAD(mbtiles_vector_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_BEFORE_UNLOAD(mbtiles_vector_datasource_plugin);

mbtiles_vector_datasource::mbtiles_vector_datasource(mapnik::parameters const& params)
    : datasource(params),
      desc_(mbtiles_vector_datasource::name(), *params.get<std::string>("encoding", "utf-8"))
{
    init(params);
}

mbtiles_vector_datasource::~mbtiles_vector_datasource() {}

mapnik::datasource::datasource_t mbtiles_vector_datasource::type() const
{
    return datasource::Vector;
}

const char * mbtiles_vector_datasource::name()
{
    return "mbtiles_vector";
}

mapnik::layer_descriptor mbtiles_vector_datasource::get_descriptor() const
{
    return desc_;
}

std::optional<mapnik::datasource_geometry_t> mbtiles_vector_datasource::get_geometry_type() const
{
    return mapnik::datasource_geometry_t::Collection;
}

mapnik::box2d<double> mbtiles_vector_datasource::envelope() const
{
    return extent_;
}

int mbtiles_vector_datasource::zoom_from_string(const std::string& z)
{
    return std::stoi(z);
}

int mbtiles_vector_datasource::zoom_from_string(const char* z)
{
    char *end_ptr;
    long int zoom = strtol(z, &end_ptr, 10);
    if (*end_ptr == '\0') {
        return static_cast<int>(zoom);
    }
    throw mapnik::datasource_exception("MBTiles Plugin: " + database_path_ + ": invalid minzoom/maxzoom in table 'metadata'");
}

void mbtiles_vector_datasource::init(mapnik::parameters const& params)
{
    std::optional<std::string> file = params.get<std::string>("file");
    if (!file)
    {
        throw mapnik::datasource_exception("mbtiles_vector Plugin: missing <file> parameter");
    }

    std::optional<std::string> base = params.get<std::string>("base");
    if (base)
    {
        database_path_ = *base + "/" + *file;
    }
    else
    {
        database_path_ = *file;
    }
    if (!mapnik::util::exists(database_path_))
    {
        throw mapnik::datasource_exception("MBTiles Plugin: " + database_path_ + " does not exist");
    }
    std::optional<std::string> layer = params.get<std::string>("layer");
    try {
        layer_ = layer.value();
    } catch (std::bad_optional_access&) {
        throw mapnik::datasource_exception("MBTiles Plugin: parameter 'layer' is missing.");
    }
    int sqlite_mode = SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE;
    dataset_ = std::make_shared<sqlite_connection>(database_path_, sqlite_mode);
    // Ensure that the tileset contains vector tiles
    std::shared_ptr<sqlite_resultset> result (dataset_->execute_query("SELECT value FROM metadata WHERE name = 'format';"));
    if (!result->is_valid() || !result->step_next() || result->column_type(0) != SQLITE_TEXT || strcmp(result->column_text(0), "pbf"))
    {
        throw mapnik::datasource_exception("MBTiles Plugin: " + database_path_ + " has unsupported vector tile format, expected 'pbf'.");
    }
    // initialize envelope
    std::optional<std::string> ext = params.get<std::string>("extent");
    if (ext && !ext->empty())
    {
        extent_.from_string(*ext);
    }
    else
    {
        result = dataset_->execute_query("SELECT value FROM metadata WHERE name = 'bounds';");
        if (result->is_valid() && result->step_next() && result->column_type(0) == SQLITE_TEXT)
        {
            const std::string extent_str = result->column_text(0);
            if (extent_str.empty()) {
                throw mapnik::datasource_exception("MBTiles Plugin: " + database_path_ + " has invalid extent.");
            }
            extent_.from_string(extent_str);
        }
    }
    if (!extent_.valid())
    {
        throw mapnik::datasource_exception("MBTiles Plugin: " + database_path_ + " extent is invalid.");
    }
    // Bounds are specified in EPSG:4326, therefore transformation is required.
    mapnik::lonlat2merc(extent_.minx_, extent_.miny_);
    mapnik::lonlat2merc(extent_.maxx_, extent_.maxy_);
    // initialize minzoom
    result = dataset_->execute_query("SELECT value FROM metadata WHERE name = 'minzoom';");
    if (result->is_valid() && result->step_next() && result->column_type(0) == SQLITE_TEXT)
    {
        minzoom_ = zoom_from_string(result->column_text(0));
    }
    // initialize maxzoom
    result = dataset_->execute_query("SELECT value FROM metadata WHERE name = 'maxzoom';");
    if (result->is_valid() && result->step_next() && result->column_type(0) == SQLITE_TEXT)
    {
        maxzoom_ = zoom_from_string(result->column_text(0));
    }
    //TODO make zoom level variable
    std::optional<std::string> zoom = params.get<std::string>("zoom");
    if (!zoom)
    {
        throw mapnik::datasource_exception("MBTiles Plugin: parameter 'zoom' missing");
    }
    zoom_ = zoom_from_string(zoom.value());
    // Get 'json' field
    result = dataset_->execute_query("SELECT value FROM metadata WHERE name = 'json';");
    if (result->is_valid() && result->step_next() && result->column_type(0) == SQLITE_TEXT)
    {
        json_ = result->column_text(0);
    }
    if (json_.empty())
    {
        throw mapnik::datasource_exception("MBTiles Plugin: " + database_path_ + " has no 'json' entry in metadata table.");
    }
    parse_json();
}

void mbtiles_vector_datasource::raise_json_error(std::string message)
{
    throw mapnik::datasource_exception(
            "MBTiles Plugin: " + database_path_
            + " has invalid 'json' entry in metadata table: " + message);
}

void mbtiles_vector_datasource::raise_json_parse_error(size_t pos, rapidjson::ParseErrorCode code)
{
    throw mapnik::datasource_exception(
            "MBTiles Plugin: " + database_path_
            + " has invalid 'json' entry in metadata table: JSON error at offset " +
           std::to_string(pos) +
           ": " +
           rapidjson::GetParseError_En(code));
}

void mbtiles_vector_datasource::parse_json()
{
    rapidjson::Document doc;
    doc.Parse(json_.c_str());
    if (doc.HasParseError())
    {
        raise_json_parse_error(doc.GetErrorOffset(), doc.GetParseError());
    }
    if (!doc.IsObject())
    {
        raise_json_error("Root is not an object.");
    }
    if (!doc.HasMember("vector_layers"))
    {
        raise_json_error("vector_layers is missing.");
    }
    const auto vector_layers = doc.FindMember("vector_layers");
    if (vector_layers == doc.MemberEnd())
    {
        raise_json_error("vector_layers is missing.");
    }
    if (!vector_layers->value.IsArray())
    {
        raise_json_error("vector_layers is not an array.");
    }
    bool found = false;
    for (const auto& l : vector_layers->value.GetArray())
    {
        const auto id = l.FindMember("id");
        if (id == doc.MemberEnd() || !id->value.IsString())
        {
            raise_json_error("vector_layers contains a layer without or with invalid 'id' property.");
        }
        if (id->value.GetString() == layer_)
        {
            found = true;
            const auto minzoom = l.FindMember("minzoom");
            const auto maxzoom = l.FindMember("maxzoom");
            if (minzoom != l.MemberEnd() && minzoom->value.IsInt())
            {
                minzoom_ = std::max(minzoom_, minzoom->value.GetInt());
            }
            if (maxzoom != l.MemberEnd() && maxzoom->value.IsInt())
            {
                maxzoom_ = std::min(maxzoom_, maxzoom->value.GetInt());
            }
            const auto fields = l.FindMember("fields");
            if (fields == l.MemberEnd())
            {
                raise_json_error("No member 'fields' for layer " + layer_);
            }
            for (const auto& field : fields->value.GetObject())
            {
                std::string name = field.name.GetString();
                if (!field.value.IsString())
                {
                    raise_json_error("Layer '" + layer_ + "' has an invalid field definition: value of field '" + name + "' is not a of JSON type String.");
                }
                std::string value_type = field.value.GetString();
                if (value_type == "String")
                {
                    desc_.add_descriptor(mapnik::attribute_descriptor(name, mapnik::String));
                }
                else if (value_type == "Number")
                {
                    desc_.add_descriptor(mapnik::attribute_descriptor(name, mapnik::Float));
                }
                else if (value_type == "Boolean")
                {
                    desc_.add_descriptor(mapnik::attribute_descriptor(name, mapnik::Boolean));
                }
                else
                {
                    raise_json_error("Layer '" + layer_ + "' field '" +  name + "': invalid field type '" + value_type + "'");
                }
            }
            break;
        }
    }
    if (!found)
    {
        raise_json_error("Requested layer '" + layer_ + "' not found.");
    }
    //TODO init bounds
}

mapnik::context_ptr mbtiles_vector_datasource::get_context_with_attributes() const
{
    mapnik::context_ptr context = std::make_shared<mapnik::context_type>();
    const std::vector<mapnik::attribute_descriptor>& desc_ar = desc_.get_descriptors();
    for (auto const& attr_info : desc_ar)
    {
        context->push(attr_info.get_name()); // TODO only push query attributes
    }
    return context;
}

mapnik::featureset_ptr mbtiles_vector_datasource::features(mapnik::query const& q) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "mbtiles_vector_datasource::features");
#endif

    mapnik::box2d<double> const& box = q.get_bbox();
    mapnik::context_ptr context = get_context_with_attributes();
    return mapnik::featureset_ptr(new mbtiles_vector_featureset(dataset_, context, zoom_, box, layer_));
}

mapnik::featureset_ptr mbtiles_vector_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "mbtiles_vector_datasource::features");
#endif

    mapnik::filter_at_point filter(pt, tol);
    mapnik::context_ptr context = get_context_with_attributes();
    return mapnik::featureset_ptr(new mbtiles_vector_featureset(dataset_, context, zoom_, filter.box_, layer_));
}
