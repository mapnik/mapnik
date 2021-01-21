/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/util/trim.hpp>
#include <mapnik/well_known_srs.hpp>

// stl
#include <stdexcept>

#ifdef MAPNIK_USE_PROJ
// proj
#include <proj.h>
#include <cmath> // HUGE_VAL
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
#ifdef MAPNIK_USE_PROJ
        init_proj();
#else
        throw std::runtime_error(std::string("Cannot initialize projection '") + params_ + " ' without proj support (-DMAPNIK_USE_PROJ)");
#endif
    }
    if (!defer_proj_init_) init_proj();
}

projection::projection(projection const& rhs)
    : params_(rhs.params_),
      defer_proj_init_(rhs.defer_proj_init_),
      is_geographic_(rhs.is_geographic_),
      proj_(nullptr),
      proj_ctx_(nullptr)
{
    if (!defer_proj_init_) init_proj();
}

projection& projection::operator=(projection const& rhs)
{
    projection tmp(rhs);
    swap(tmp);
    proj_ctx_ = nullptr;
    proj_ = nullptr;
    if (!defer_proj_init_) init_proj();
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

void projection::init_proj() const
{
#ifdef MAPNIK_USE_PROJ
    if (!proj_)
    {
        proj_ctx_ = proj_context_create();
        proj_ = proj_create_crs_to_crs(proj_ctx_, "epsg:4326", params_.c_str(), nullptr);
        if (!proj_ || !proj_ctx_)
        {
            if (proj_ctx_) {
                proj_context_destroy(proj_ctx_);
                proj_ctx_ = nullptr;
            }
            if (proj_) {
                proj_destroy(proj_);
                proj_ = nullptr;
            }
            throw proj_init_error(params_);
        }
        // determine the type of CRS
        PJ* crs = proj_create(proj_ctx_, params_.c_str());
        if (crs)
        {
            PJ_TYPE type = proj_get_type(crs);
            is_geographic_ = (type == PJ_TYPE_GEOGRAPHIC_2D_CRS
                              ||
                              type == PJ_TYPE_GEOGRAPHIC_3D_CRS) ? true : false;
        }
        proj_destroy(crs);
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
#ifdef MAPNIK_USE_PROJ
    if (!proj_)
    {
        throw std::runtime_error("projection::forward not supported unless proj is initialized");
    }
    PJ_COORD coord;
    coord.lpzt.z = 0.0;
    coord.lpzt.t = HUGE_VAL;
    coord.lpzt.lam = x;
    coord.lpzt.phi = y;
    PJ_COORD coord_out = proj_trans(proj_, PJ_FWD, coord);
    x = coord_out.xy.x;
    y = coord_out.xy.y;
#else
    throw std::runtime_error("projection::forward not supported without proj support (-DMAPNIK_USE_PROJ)");
#endif
}

void projection::inverse(double & x,double & y) const
{
#ifdef MAPNIK_USE_PROJ
    if (!proj_)
    {
        throw std::runtime_error("projection::forward not supported unless proj is initialized");
    }
    PJ_COORD coord;
    coord.xyzt.z = 0.0;
    coord.xyzt.t = HUGE_VAL;
    coord.xyzt.x = x;
    coord.xyzt.y = y;
    PJ_COORD coord_out = proj_trans(proj_, PJ_INV, coord);
    x = coord_out.xy.x;
    y = coord_out.xy.y;
#else
    throw std::runtime_error("projection::inverse not supported without proj support (-DMAPNIK_USE_PROJ)");
#endif
}

projection::~projection()
{
#ifdef MAPNIK_USE_PROJ
    if (proj_)
    {
        proj_destroy(proj_);
        proj_ = nullptr;
    }
    if (proj_ctx_)
    {
        proj_context_destroy(proj_ctx_);
        proj_ctx_ = nullptr;
    }
#endif
}

std::string projection::expanded() const
{
#ifdef MAPNIK_USE_PROJ
    if (proj_)
    {
        PJ_PROJ_INFO info = proj_pj_info(proj_);
        return mapnik::util::trim_copy(info.definition);
    }
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
