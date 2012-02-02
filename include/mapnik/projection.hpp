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

#ifndef MAPNIK_PROJECTION_HPP
#define MAPNIK_PROJECTION_HPP

// mapnik
#include <mapnik/box2d.hpp>

// proj4
#include <proj_api.h>

// boost
#if defined(MAPNIK_THREADSAFE) && PJ_VERSION < 480
#include <boost/thread/mutex.hpp>
#endif
#include <boost/utility.hpp>

// stl
#include <string>
#include <iostream>
#include <stdexcept>

namespace mapnik {

class proj_init_error : public std::runtime_error
{
public:
    proj_init_error(std::string const& params)
        : std::runtime_error("failed to initialize projection with: '" + params + "'") {}
};

class MAPNIK_DECL projection
{
    friend class proj_transform;
public:
    explicit projection(std::string const& params = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
    projection(projection const& rhs);
    ~projection();

    projection& operator=(projection const& rhs);
    bool operator==(const projection& other) const;
    bool operator!=(const projection& other) const;
    bool is_initialized() const;
    bool is_geographic() const;
    std::string const& params() const;

    void forward(double & x, double &y ) const;
    void inverse(double & x,double & y) const;

    std::string expanded() const;

private:
    void init();
    void swap (projection& rhs);

private:
    std::string params_;
    projPJ proj_;
    bool is_geographic_;
#if PJ_VERSION >= 480
    projCtx proj_ctx_;
#elif defined(MAPNIK_THREADSAFE)
    static boost::mutex mutex_;
#endif
};

}

#endif // MAPNIK_PROJECTION_HPP
