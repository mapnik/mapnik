// SPDX-License-Identifier: LGPL-2.1-or-later
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

#include "pmtiles_datasource.hpp"
#include "pmtiles_featureset.hpp"
#include "vector_tile_projection.hpp"
#include <mapnik/geom_util.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/datasource_plugin.hpp>
#include <string>
#include <algorithm>

DATASOURCE_PLUGIN_IMPL(pmtiles_datasource_plugin, pmtiles_datasource);
DATASOURCE_PLUGIN_EXPORT(pmtiles_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_AFTER_LOAD(pmtiles_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_BEFORE_UNLOAD(pmtiles_datasource_plugin);

pmtiles_datasource::pmtiles_datasource(mapnik::parameters const& params)
    : datasource(params),
      desc_(pmtiles_datasource::name(), *params.get<std::string>("encoding", "utf-8"))
{
    init(params);
}

pmtiles_datasource::~pmtiles_datasource() {}

mapnik::datasource::datasource_t pmtiles_datasource::type() const
{
    return datasource::Vector;
}

const char * pmtiles_datasource::name()
{
    return "pmtiles";
}

mapnik::layer_descriptor pmtiles_datasource::get_descriptor() const
{
    return desc_;
}

std::optional<mapnik::datasource_geometry_t> pmtiles_datasource::get_geometry_type() const
{
    return mapnik::datasource_geometry_t::Collection;
}

mapnik::box2d<double> pmtiles_datasource::envelope() const
{
    return extent_;
}

void pmtiles_datasource::init(mapnik::parameters const& params)
{
    std::optional<std::string> file = params.get<std::string>("file");
    if (!file)
    {
        throw mapnik::datasource_exception("pmtiles Plugin: missing <file> parameter");
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
        throw mapnik::datasource_exception("pmtiles Plugin: " + database_path_ + " does not exist");
    }
    std::optional<std::string> layer = params.get<std::string>("layer");
    try
    {
        layer_ = layer.value();
    }
    catch (std::bad_optional_access&)
    {
        throw mapnik::datasource_exception("pmtiles Plugin: parameter 'layer' is missing.");
    }

    file_ptr_ = std::make_shared<mapnik::pmtiles_file>(database_path_);
    if (!file_ptr_->is_good())
    {
        throw mapnik::datasource_exception("Failed to create memory mapping for " + database_path_);
    }

    file_ptr_->read_header(*this);

    // overwrite envelope with user supplied
    std::optional<std::string> ext = params.get<std::string>("extent");
    if (ext && !ext->empty())
    {
        extent_.from_string(*ext);
    }
    if (!extent_.valid())
    {
        throw mapnik::datasource_exception("pmtiles Plugin: " + database_path_ + " extent is invalid.");
    }
    // Bounds are specified in EPSG:4326, therefore transformation is required.
    mapnik::lonlat2merc(extent_.minx_, extent_.miny_);
    mapnik::lonlat2merc(extent_.maxx_, extent_.maxy_);

    std::optional<mapnik::value_integer> zoom = params.get<mapnik::value_integer>("zoom");
    if (!zoom)
    {
         throw mapnik::datasource_exception("pmtiles Plugin: parameter 'zoom' missing");
    }
    zoom_ = *zoom;

    auto metadata = file_ptr_->metadata();
    auto layers = metadata.at("vector_layers");
    bool found = false;
    for (auto const& layer : layers.as_array())
    {
        std::string id = layer.at("id").as_string().c_str();
        if (id == layer_)
        {
            found = true;
            if (auto const* p = layer.as_object().if_contains("minzoom"))
            {
                minzoom_ = std::max(minzoom_, p->as_int64());
            }
            if (auto const* p = layer.as_object().if_contains("maxzoom"))
            {
                maxzoom_ = std::min(maxzoom_, p->as_int64());
            }
            std::cerr << "====== Layer ID:" << id << " min/max zoom:" << minzoom_ << ":" << maxzoom_ << std::endl;
            for (auto const& field : layer.at("fields").as_object())
            {
                std::string name{field.key_c_str()};
                if (field.value() == "String")
                {
                    desc_.add_descriptor(mapnik::attribute_descriptor(name, mapnik::String));
                }
                else if (field.value() == "Number")
                {
                    desc_.add_descriptor(mapnik::attribute_descriptor(name, mapnik::Float));
                }
                else if (field.value() == "Boolean")
                {
                    desc_.add_descriptor(mapnik::attribute_descriptor(name, mapnik::Boolean));
                }
            }
            break;
        }
    }
    if (!found)
    {
        throw mapnik::datasource_exception("Requested layer '" + layer_ + "' not found.");
    }
}

mapnik::context_ptr pmtiles_datasource::get_context_with_attributes() const
{
    mapnik::context_ptr context = std::make_shared<mapnik::context_type>();
    std::vector<mapnik::attribute_descriptor> const& desc_ar = desc_.get_descriptors();
    for (auto const& attr_info : desc_ar)
    {
        context->push(attr_info.get_name());
    }
    return context;
}

mapnik::context_ptr pmtiles_datasource::get_query_context(mapnik::query const& q) const
{
    mapnik::context_ptr context = std::make_shared<mapnik::context_type>();
    std::vector<mapnik::attribute_descriptor> const& desc_ar = desc_.get_descriptors();
    for (auto const& name : q.property_names())
    {
        for (auto const& attr_info : desc_ar)
        {
            if (name == attr_info.get_name())
            {
                context->push(name);
                break;
            }
        }
    }
    return context;
}

namespace {

    double scales[] = {279541132.014, 139770566.007, 69885283.0036, 34942641.5018, 17471320.7509,
                       8735660.37545, 4367830.18772, 2183915.09386, 1091957.54693, 545978.773466,
                       272989.386733, 136494.693366, 68247.3466832, 34123.6733416, 17061.8366708,
                       8530.9183354,  4265.4591677,  2132.72958385, 1066.36479192, 533.182395962};
    std::int64_t scale_to_zoom(double scale, std::int64_t minzoom, std::int64_t maxzoom)
    {
        for (std::int64_t zoom = 0; zoom < 19; ++zoom)
        {
            if (scale > scales[zoom]) return minzoom;
            else if (scale < scales[zoom] && scale > scales[zoom + 1])
            {
                return std::min(zoom, maxzoom);
            }
        }
        return maxzoom;
    }
}
mapnik::featureset_ptr pmtiles_datasource::features(mapnik::query const& q) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "pmtiles_datasource::features");
#endif

    mapnik::box2d<double> const& box = q.get_bbox();
    std::cerr << "scale_denominator:" << q.scale_denominator() << std::endl;
    auto zoom = scale_to_zoom(q.scale_denominator(), minzoom_, maxzoom_);
    std::cerr << "zoom:" << zoom << std::endl;
    mapnik::context_ptr context = get_query_context(q);
    auto && file_ptr = std::make_unique<mapnik::pmtiles_file>(database_path_);
    if (!file_ptr->is_good())
    {
        throw mapnik::datasource_exception("Failed to create memory mapping for " + database_path_);
    }
    return mapnik::featureset_ptr(new pmtiles_featureset(file_ptr_, context, zoom, box, layer_));
}

mapnik::featureset_ptr pmtiles_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "pmtiles_datasource::features");
#endif

    mapnik::filter_at_point filter(pt, tol);
    mapnik::context_ptr context = get_context_with_attributes();
    return mapnik::featureset_ptr(new pmtiles_featureset(file_ptr_, context, zoom_, filter.box_, layer_));
}
