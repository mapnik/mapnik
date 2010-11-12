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


void notice(const char* fmt, ...)
{
    va_list ap;
    fprintf( stdout, "NOTICE: ");
    
    va_start (ap, fmt);
    vfprintf( stdout, fmt, ap);
    va_end(ap);
    fprintf( stdout, "\n" );
}

void log_and_exit(const char* fmt, ...)
{
    va_list ap;
    fprintf( stdout, "ERROR: ");

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
     desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8"))
{
    boost::optional<std::string> geometry = params.get<std::string>("wkt");
    if (!geometry) throw datasource_exception("missing <wkt> parameter");
    geometry_string_ = *geometry;

    multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries",false);

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

#ifdef MAPNIK_DEBUG
        clog << "finalizing GEOS...";
#endif
    
        finishGEOS();

#ifdef MAPNIK_DEBUG
        clog << " finalized !" << endl;
#endif
    }
}

void geos_datasource::bind() const
{
    if (is_bound_) return;   

#ifdef MAPNIK_DEBUG
    clog << "initializing GEOS...";
#endif

    // open ogr driver
    initGEOS(notice, log_and_exit);

#ifdef MAPNIK_DEBUG
    clog << " initialized !" << endl;
#endif

    // parse the string into geometry
    geometry_.set_feature(GEOSGeomFromWKT(geometry_string_.c_str()));
    
    if (*geometry_ == NULL || ! GEOSisValid(*geometry_))
    {
      //char* reason = GEOSisValidReason(*geometry_);
      //std::string reason_string (reason);
      //GEOSFree(reason);

      throw datasource_exception("invalid <wkt> geometry specified");
    }

#ifdef MAPNIK_DEBUG
    clog << "geometry correctly parsed" << endl;
#endif

    // initialize envelope
    boost::optional<std::string> ext  = params_.get<std::string>("extent");
    if (ext)
    {
        boost::char_separator<char> sep(",");
        boost::tokenizer<boost::char_separator<char> > tok(*ext,sep);
        unsigned i = 0;
        bool success = false;
        double d[4];
        for (boost::tokenizer<boost::char_separator<char> >::iterator beg=tok.begin(); 
             beg!=tok.end();++beg)
        {
            try 
            {
                d[i] = boost::lexical_cast<double>(*beg);
            }
            catch (boost::bad_lexical_cast & ex)
            {
                std::clog << ex.what() << "\n";
                break;
            }
            if (i==3) 
            {
                success = true;
                break;
            }
            ++i;
        }

        if (success)
        {
            extent_.init(d[0],d[1],d[2],d[3]);
            extent_initialized_ = true;
        }
    }
    
    if (!extent_initialized_)
    {
        geos_feature_ptr envelope (GEOSEnvelope(*geometry_));
        
        if (*envelope != NULL)
        {
            const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq(*envelope);
            if (cs != NULL)
            {
                double x, y;
                unsigned int num_points;

                GEOSCoordSeq_getSize(cs, &num_points);

                for (unsigned int i = 0; i < num_points; ++i)
                {
                    GEOSCoordSeq_getX(cs, i, &x);
                    GEOSCoordSeq_getY(cs, i, &y);
                    
                    clog << x << " " << y << endl; 
                }

                extent_initialized_ = true;
            }
        }
    }
   
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
    s << "POLYGON("
      << extent.minx() << " " << extent.miny() << ","
      << extent.maxx() << " " << extent.miny() << ","
      << extent.maxx() << " " << extent.maxy() << ","
      << extent.minx() << " " << extent.maxy() << ","
      << extent.minx() << " " << extent.miny()
      << ")";

#ifdef MAPNIK_DEBUG
    clog << "using extent: " << s.str() << endl;
#endif

    return featureset_ptr(new geos_featureset (*geometry_,
                                               GEOSGeomFromWKT(s.str().c_str()),
                                               desc_.get_encoding(),
                                               multiple_geometries_));
}

featureset_ptr geos_datasource::features_at_point(coord2d const& pt) const
{
    if (!is_bound_) bind();

    std::ostringstream s;
    s << "POINT(" << pt.x << " " << pt.y << ")";

#ifdef MAPNIK_DEBUG
    clog << "using point: " << s.str() << endl;
#endif

    return featureset_ptr(new geos_featureset (*geometry_,
                                               GEOSGeomFromWKT(s.str().c_str()),
                                               desc_.get_encoding(),
                                               multiple_geometries_));
}

