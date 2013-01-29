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
#include <mapnik/config.hpp>
#include <mapnik/well_known_srs.hpp>

// boost
#include <boost/optional.hpp>

// stl
#include <string>
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

    explicit projection(std::string const& params = MAPNIK_LONGLAT_PROJ,
                        bool defer_proj_init = false);
    projection(projection const& rhs);
    ~projection();

    projection& operator=(projection const& rhs);
    bool operator==(const projection& other) const;
    bool operator!=(const projection& other) const;
    bool is_initialized() const;
    bool is_geographic() const;
    boost::optional<well_known_srs_e> well_known() const;
    std::string const& params() const;
    void forward(double & x, double & y) const;
    void inverse(double & x,double & y) const;
    std::string expanded() const;
    void init_proj4() const;

private:
    void swap (projection& rhs);

private:
    std::string params_;
    bool defer_proj_init_;
    mutable bool is_geographic_;
    mutable void * proj_;
    mutable void * proj_ctx_;
};

}

#endif // MAPNIK_PROJECTION_HPP
