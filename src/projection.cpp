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

// boost
#include <boost/algorithm/string.hpp>

// proj4
#include <proj_api.h>

namespace mapnik {

#if defined(MAPNIK_THREADSAFE) && PJ_VERSION < 480
#warning mapnik is building against < proj 4.8, reprojection will be faster if you use >= 4.8
boost::mutex projection::mutex_;
#endif

projection::projection(std::string const& params)
    : params_(params)
{
    init();
}

projection::projection(projection const& rhs)
    : params_(rhs.params_)
{
    init();
}

projection& projection::operator=(projection const& rhs)
{
    projection tmp(rhs);
    swap(tmp);
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

bool projection::is_initialized() const
{
    return proj_ ? true : false;
}

bool projection::is_geographic() const
{
    return is_geographic_;
}

std::string const& projection::params() const
{
    return params_;
}

void projection::forward(double & x, double &y ) const
{
#if defined(MAPNIK_THREADSAFE) && PJ_VERSION < 480
    mutex::scoped_lock lock(mutex_);
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
}

void projection::inverse(double & x,double & y) const
{
#if defined(MAPNIK_THREADSAFE) && PJ_VERSION < 480
    mutex::scoped_lock lock(mutex_);
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
}

projection::~projection()
{
#if defined(MAPNIK_THREADSAFE) && PJ_VERSION < 480
    mutex::scoped_lock lock(mutex_);
#endif
    if (proj_) pj_free(proj_);
#if PJ_VERSION >= 480
    if (proj_ctx_) pj_ctx_free(proj_ctx_);
#endif
}

void projection::init()
{
#if defined(MAPNIK_THREADSAFE) && PJ_VERSION < 480
    mutex::scoped_lock lock(mutex_);
#endif
#if PJ_VERSION >= 480
    proj_ctx_ = pj_ctx_alloc();
    proj_ = pj_init_plus_ctx(proj_ctx_, params_.c_str());
    if (!proj_)
    {
        if (proj_ctx_) pj_ctx_free(proj_ctx_);
        throw proj_init_error(params_);
    }
#else
    proj_ = pj_init_plus(params_.c_str());
    if (!proj_) throw proj_init_error(params_);
#endif
    is_geographic_ = pj_is_latlong(proj_) ? true : false;
}

std::string projection::expanded() const
{
    if (proj_) {
        std::string def(pj_get_def( proj_, 0 ));
        //boost::algorithm::ireplace_first(def,params_,"");
        boost::trim(def);
        return def;
    }
    return std::string("");
}

void projection::swap(projection& rhs)
{
    std::swap(params_,rhs.params_);
    init();
}
}
