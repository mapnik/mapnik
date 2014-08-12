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
#include <mapnik/projection.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/well_known_srs.hpp>

// stl
#include <stdexcept>

#ifdef MAPNIK_USE_PROJ4
// proj4
#include <proj_api.h>
 #if defined(MAPNIK_THREADSAFE) && PJ_VERSION < 480
    #include <mutex>
    static std::mutex mutex_;
    #ifdef _MSC_VER
     #pragma NOTE(mapnik is building against < proj 4.8, reprojection will be faster if you use >= 4.8)
    #else
     #warning mapnik is building against < proj 4.8, reprojection will be faster if you use >= 4.8
    #endif
 #endif
#endif

namespace mapnik {


projection::projection(std::string const& params, bool defer_proj_init)
    : params_(params),
      defer_proj_init_(defer_proj_init),
      is_geographic_(false),
      proj_(nullptr),
      proj_ctx_(nullptr)
{
    boost::optional<bool> is_known = is_known_geographic(params_);
    if (is_known){
        is_geographic_ = *is_known;
    }
    else
    {
#ifdef MAPNIK_USE_PROJ4
        init_proj4();
#else
        throw std::runtime_error(std::string("Cannot initialize projection '") + params_ + " ' without proj4 support (-DMAPNIK_USE_PROJ4)");
#endif
    }
    if (!defer_proj_init_) init_proj4();
}

projection::projection(projection const& rhs)
    : params_(rhs.params_),
      defer_proj_init_(rhs.defer_proj_init_),
      is_geographic_(rhs.is_geographic_),
      proj_(nullptr),
      proj_ctx_(nullptr)
{
    if (!defer_proj_init_) init_proj4();
}

projection& projection::operator=(projection const& rhs)
{
    projection tmp(rhs);
    swap(tmp);
    proj_ctx_ = 0;
    proj_ = 0;
    if (!defer_proj_init_) init_proj4();
    return *this;
}

bool projection::operator==(const projection& other) const
{
    return (params_ == other.params_);
}

bool projection::operator!=(const projection& other) const
{
    return !(*this == other);
}

void projection::init_proj4() const
{
#ifdef MAPNIK_USE_PROJ4
    if (!proj_)
    {
#if PJ_VERSION >= 480
        proj_ctx_ = pj_ctx_alloc();
        proj_ = pj_init_plus_ctx(proj_ctx_, params_.c_str());
        if (!proj_)
        {
            if (proj_ctx_) {
                pj_ctx_free(proj_ctx_);
                proj_ctx_ = 0;
            }
            throw proj_init_error(params_);
        }
#else
        #if defined(MAPNIK_THREADSAFE)
        mapnik::scoped_lock lock(mutex_);
        #endif
        proj_ = pj_init_plus(params_.c_str());
        if (!proj_) throw proj_init_error(params_);
#endif
        is_geographic_ = pj_is_latlong(proj_) ? true : false;
    }
#endif
}

bool projection::is_initialized() const
{
    return proj_ ? true : false;
}

bool projection::is_geographic() const
{
    return is_geographic_;
}

boost::optional<well_known_srs_e> projection::well_known() const
{
    return is_well_known_srs(params_);
}

std::string const& projection::params() const
{
    return params_;
}

void projection::forward(double & x, double &y ) const
{
#ifdef MAPNIK_USE_PROJ4
    if (!proj_)
    {
        throw std::runtime_error("projection::forward not supported unless proj4 is initialized");
    }
    #if defined(MAPNIK_THREADSAFE) && PJ_VERSION < 480
    mapnik::scoped_lock lock(mutex_);
    #endif
    projUV p;
    p.u = x * DEG_TO_RAD;
    p.v = y * DEG_TO_RAD;
    p = pj_fwd(p,proj_);
    x = p.u;
    y = p.v;
    if (is_geographic_)
    {
        x *=RAD_TO_DEG;
        y *=RAD_TO_DEG;
    }
#else
    throw std::runtime_error("projection::forward not supported without proj4 support (-DMAPNIK_USE_PROJ4)");
#endif
}

void projection::inverse(double & x,double & y) const
{
#ifdef MAPNIK_USE_PROJ4
    if (!proj_)
    {
        throw std::runtime_error("projection::inverse not supported unless proj4 is initialized");
    }

    #if defined(MAPNIK_THREADSAFE) && PJ_VERSION < 480
    mapnik::scoped_lock lock(mutex_);
    #endif
    if (is_geographic_)
    {
        x *=DEG_TO_RAD;
        y *=DEG_TO_RAD;
    }
    projUV p;
    p.u = x;
    p.v = y;
    p = pj_inv(p,proj_);
    x = RAD_TO_DEG * p.u;
    y = RAD_TO_DEG * p.v;
#else
    throw std::runtime_error("projection::inverse not supported without proj4 support (-DMAPNIK_USE_PROJ4)");
#endif
}

projection::~projection()
{
#ifdef MAPNIK_USE_PROJ4
    #if defined(MAPNIK_THREADSAFE) && PJ_VERSION < 480
        mapnik::scoped_lock lock(mutex_);
    #endif
        if (proj_) pj_free(proj_);
    #if PJ_VERSION >= 480
        if (proj_ctx_) pj_ctx_free(proj_ctx_);
    #endif
#endif
}

std::string projection::expanded() const
{
#ifdef MAPNIK_USE_PROJ4
    if (proj_) return mapnik::util::trim_copy(pj_get_def( proj_, 0 ));
#endif
    return params_;
}

void projection::swap(projection& rhs)
{
    std::swap(params_,rhs.params_);
    std::swap(defer_proj_init_,rhs.defer_proj_init_);
    std::swap(is_geographic_,rhs.is_geographic_);
}

}
