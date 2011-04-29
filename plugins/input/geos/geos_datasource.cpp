/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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
// $Id$

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdarg>

#include "geos_datasource.hpp"
#include "geos_featureset.hpp"

// mapnik
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/geom_util.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/limits.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem/operations.hpp>

// geos
#include <geos_c.h>

using std::clog;
using std::endl;

using boost::lexical_cast;
using boost::bad_lexical_cast;

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(geos_datasource)

using mapnik::box2d;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;
using mapnik::datasource_exception;
using mapnik::filter_in_box;
using mapnik::filter_at_point;


void geos_notice(const char* fmt, ...)
{
    va_list ap;
    fprintf( stdout, "GEOS Plugin: (GEOS NOTICE) ");
    
    va_start (ap, fmt);
    vfprintf( stdout, fmt, ap);
    va_end(ap);
    fprintf( stdout, "\n" );
}

void geos_error(const char* fmt, ...)
{
    va_list ap;
    fprintf( stdout, "GEOS Plugin: (GEOS ERROR) ");

    va_start (ap, fmt);
    vfprintf( stdout, fmt, ap);
    va_end(ap);
    fprintf( stdout, "\n" );
}


geos_datasource::geos_datasource(parameters const& params, bool bind)
   : datasource(params),
     extent_(),
     extent_initialized_(false),
     type_(datasource::Vector),
     desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8")),
     geometry_data_(""),
     geometry_data_name_("name"),
     geometry_id_(1)
{
    boost::optional<std::string> geometry = params.get<std::string>("wkt");
    if (!geometry) throw datasource_exception("missing <wkt> parameter");
    geometry_string_ = *geometry;

    multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries",false);

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
#ifdef MAPNIK_DEBUG
        clog << "GEOS Plugin: initializing extent from geometry" << endl;
#endif

        if (GEOSGeomTypeId(*geometry_) == GEOS_POINT)
        {
            double x, y;
            unsigned int size;

            const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq(*geometry_);

            GEOSCoordSeq_getSize(cs, &size);
            GEOSCoordSeq_getX(cs, 0, &x);
            GEOSCoordSeq_getY(cs, 0, &y);
        
            extent_.init(x,y,x,y);
            extent_initialized_ = true;
        }
        else
        {
            geos_feature_ptr envelope (GEOSEnvelope(*geometry_));
            if (*envelope != NULL && GEOSisValid(*envelope))
            {
#ifdef MAPNIK_DEBUG
                char* wkt = GEOSGeomToWKT(*envelope);
                clog << "GEOS Plugin: getting coord sequence from: " << wkt << endl;
                GEOSFree(wkt);
#endif

                const GEOSGeometry* exterior = GEOSGetExteriorRing(*envelope);
                if (exterior != NULL && GEOSisValid(exterior))
                {
                    const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq(exterior);
                    if (cs != NULL)
                    {
#ifdef MAPNIK_DEBUG
                        clog << "GEOS Plugin: iterating boundary points" << endl;
#endif

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

                        extent_.init(minx,miny,maxx,maxy);
                        extent_initialized_ = true;
                    }
                }
            }
        }
    }

    if (! extent_initialized_)
        throw datasource_exception("GEOS Plugin: cannot determine extent for <wkt> geometry");
   
    is_bound_ = true;
}

std::string geos_datasource::name()
{
    return "geos";
}

int geos_datasource::type() const
{
    return type_;
}

box2d<double> geos_datasource::envelope() const
{
    if (!is_bound_) bind();
    
    return extent_;
}

layer_descriptor geos_datasource::get_descriptor() const
{
    if (!is_bound_) bind();
    
    return desc_;
}

featureset_ptr geos_datasource::features(query const& q) const
{
    if (!is_bound_) bind();

    const mapnik::box2d<double> extent = q.get_bbox();

    std::ostringstream s;
    s << "POLYGON(("
      << extent.minx() << " " << extent.miny() << ","
      << extent.maxx() << " " << extent.miny() << ","
      << extent.maxx() << " " << extent.maxy() << ","
      << extent.minx() << " " << extent.maxy() << ","
      << extent.minx() << " " << extent.miny()
      << "))";

#ifdef MAPNIK_DEBUG
    clog << "GEOS Plugin: using extent: " << s.str() << endl;
#endif

    return featureset_ptr(new geos_featureset (*geometry_,
                                               GEOSGeomFromWKT(s.str().c_str()),
                                               geometry_id_,
                                               geometry_data_,
                                               geometry_data_name_,
                                               desc_.get_encoding(),
                                               multiple_geometries_));
}

featureset_ptr geos_datasource::features_at_point(coord2d const& pt) const
{
    if (!is_bound_) bind();

    std::ostringstream s;
    s << "POINT(" << pt.x << " " << pt.y << ")";

#ifdef MAPNIK_DEBUG
    clog << "GEOS Plugin: using point: " << s.str() << endl;
#endif

    return featureset_ptr(new geos_featureset (*geometry_,
                                               GEOSGeomFromWKT(s.str().c_str()),
                                               geometry_id_,
                                               geometry_data_,
                                               geometry_data_name_,
                                               desc_.get_encoding(),
                                               multiple_geometries_));
}

