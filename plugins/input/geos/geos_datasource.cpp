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

#include "geos_datasource.hpp"
#include "geos_featureset.hpp"

// stl
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdarg>

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/timer.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/limits.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/make_shared.hpp>

// geos
#include <geos_c.h>

using mapnik::box2d;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;
using mapnik::datasource_exception;
using mapnik::datasource;
using mapnik::parameters;
using mapnik::filter_in_box;
using mapnik::filter_at_point;

DATASOURCE_PLUGIN(geos_datasource)

void geos_notice(const char* format, ...)
{
#ifdef MAPNIK_LOG
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 512, format, args);
    va_end(args);

    MAPNIK_LOG_WARN(geos) << "geos_datasource: " << buffer;
#endif
}

void geos_error(const char* format, ...)
{
#ifdef MAPNIK_LOG
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 512, format, args);
    va_end(args);

    MAPNIK_LOG_ERROR(geos) << "geos_datasource: " << buffer;
#endif
}


geos_datasource::geos_datasource(parameters const& params, bool bind)
    : datasource(params),
      extent_(),
      extent_initialized_(false),
      type_(datasource::Vector),
      desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding", "utf-8")),
      geometry_data_(""),
      geometry_data_name_("name"),
      geometry_id_(1)
{
    boost::optional<std::string> geometry = params.get<std::string>("wkt");
    if (! geometry) throw datasource_exception("missing <wkt> parameter");
    geometry_string_ = *geometry;

    boost::optional<std::string> ext = params_.get<std::string>("extent");
    if (ext) extent_initialized_ = extent_.from_string(*ext);

    boost::optional<int> id = params_.get<int>("gid");
    if (id) geometry_id_ = *id;

    boost::optional<std::string> gdata = params_.get<std::string>("field_data");
    if (gdata) geometry_data_ = *gdata;

    boost::optional<std::string> gdata_name = params_.get<std::string>("field_name");
    if (gdata_name) geometry_data_name_ = *gdata_name;

    desc_.add_descriptor(attribute_descriptor(geometry_data_name_, mapnik::String));

    if (bind)
    {
        this->bind();
    }
}

geos_datasource::~geos_datasource()
{
    if (is_bound_)
    {
        geometry_.set_feature(0);

        finishGEOS();
    }
}

void geos_datasource::bind() const
{
    if (is_bound_) return;

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "geos_datasource::bind");
#endif

    // open geos driver
    initGEOS(geos_notice, geos_error);

    // parse the string into geometry
    geometry_.set_feature(GEOSGeomFromWKT(geometry_string_.c_str()));
    if (*geometry_ == NULL || ! GEOSisValid(*geometry_))
    {
        throw datasource_exception("GEOS Plugin: invalid <wkt> geometry specified");
    }

    // try to obtain the extent from the geometry itself
    if (! extent_initialized_)
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats2__(std::clog, "geos_datasource::bind(initialize_extent)");
#endif

        MAPNIK_LOG_DEBUG(geos) << "geos_datasource: Initializing extent from geometry";

        if (GEOSGeomTypeId(*geometry_) == GEOS_POINT)
        {
            double x, y;
            unsigned int size;

            const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq(*geometry_);

            GEOSCoordSeq_getSize(cs, &size);
            GEOSCoordSeq_getX(cs, 0, &x);
            GEOSCoordSeq_getY(cs, 0, &y);

            extent_.init(x, y, x, y);
            extent_initialized_ = true;
        }
        else
        {
            geos_feature_ptr envelope (GEOSEnvelope(*geometry_));
            if (*envelope != NULL && GEOSisValid(*envelope))
            {
#ifdef MAPNIK_LOG
                char* wkt = GEOSGeomToWKT(*envelope);
                MAPNIK_LOG_DEBUG(geos) << "geos_datasource: Getting coord sequence from=" << wkt;
                GEOSFree(wkt);
#endif

                const GEOSGeometry* exterior = GEOSGetExteriorRing(*envelope);
                if (exterior != NULL && GEOSisValid(exterior))
                {
                    const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq(exterior);
                    if (cs != NULL)
                    {
                        MAPNIK_LOG_DEBUG(geos) << "geos_datasource: Iterating boundary points";

                        double x, y;
                        double minx = std::numeric_limits<float>::max(),
                            miny = std::numeric_limits<float>::max(),
                            maxx = -std::numeric_limits<float>::max(),
                            maxy = -std::numeric_limits<float>::max();

                        unsigned int num_points;
                        GEOSCoordSeq_getSize(cs, &num_points);

                        for (unsigned int i = 0; i < num_points; ++i)
                        {
                            GEOSCoordSeq_getX(cs, i, &x);
                            GEOSCoordSeq_getY(cs, i, &y);

                            if (x < minx) minx = x;
                            if (x > maxx) maxx = x;
                            if (y < miny) miny = y;
                            if (y > maxy) maxy = y;
                        }

                        extent_.init(minx, miny, maxx, maxy);
                        extent_initialized_ = true;
                    }
                }
            }
        }
    }

    if (! extent_initialized_)
    {
        throw datasource_exception("GEOS Plugin: cannot determine extent for <wkt> geometry");
    }

    is_bound_ = true;
}

const char * geos_datasource::name()
{
    return "geos";
}

mapnik::datasource::datasource_t geos_datasource::type() const
{
    return type_;
}

box2d<double> geos_datasource::envelope() const
{
    if (! is_bound_) bind();

    return extent_;
}

boost::optional<mapnik::datasource::geometry_t> geos_datasource::get_geometry_type() const
{
    if (! is_bound_) bind();
    boost::optional<mapnik::datasource::geometry_t> result;

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "geos_datasource::get_geometry_type");
#endif

    // get geometry type
    const int type = GEOSGeomTypeId(*geometry_);
    switch (type)
    {
    case GEOS_POINT:
    case GEOS_MULTIPOINT:
        result.reset(mapnik::datasource::Point);
        break;
    case GEOS_LINESTRING:
    case GEOS_LINEARRING:
    case GEOS_MULTILINESTRING:
        result.reset(mapnik::datasource::LineString);
        break;
    case GEOS_POLYGON:
    case GEOS_MULTIPOLYGON:
        result.reset(mapnik::datasource::Polygon);
        break;
    case GEOS_GEOMETRYCOLLECTION:
        result.reset(mapnik::datasource::Collection);
        break;
    default:
        break;
    }

    return result;
}

layer_descriptor geos_datasource::get_descriptor() const
{
    if (! is_bound_) bind();

    return desc_;
}

featureset_ptr geos_datasource::features(query const& q) const
{
    if (! is_bound_) bind();

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "geos_datasource::features");
#endif

    const mapnik::box2d<double> extent = q.get_bbox();

    std::ostringstream s;
    s << "POLYGON(("
      << extent.minx() << " " << extent.miny() << ","
      << extent.maxx() << " " << extent.miny() << ","
      << extent.maxx() << " " << extent.maxy() << ","
      << extent.minx() << " " << extent.maxy() << ","
      << extent.minx() << " " << extent.miny()
      << "))";

    MAPNIK_LOG_DEBUG(geos) << "geos_datasource: Using extent=" << s.str();

    return boost::make_shared<geos_featureset>(*geometry_,
                                               GEOSGeomFromWKT(s.str().c_str()),
                                               geometry_id_,
                                               geometry_data_,
                                               geometry_data_name_,
                                               desc_.get_encoding());
}

featureset_ptr geos_datasource::features_at_point(coord2d const& pt, double tol) const
{
    if (! is_bound_) bind();

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "geos_datasource::features_at_point");
#endif

    std::ostringstream s;
    s << "POINT(" << pt.x << " " << pt.y << ")";

    MAPNIK_LOG_DEBUG(geos) << "geos_datasource: Using point=" << s.str();

    return boost::make_shared<geos_featureset>(*geometry_,
                                               GEOSGeomFromWKT(s.str().c_str()),
                                               geometry_id_,
                                               geometry_data_,
                                               geometry_data_name_,
                                               desc_.get_encoding());
}
