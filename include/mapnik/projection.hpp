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

#ifndef MAPNIK_PROJECTION_HPP
#define MAPNIK_PROJECTION_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/well_known_srs.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <string>
#include <stdexcept>

// fwd decl
#if MAPNIK_PROJ_VERSION >= 80000
struct pj_ctx;
using PJ_CONTEXT = struct pj_ctx;
#else
struct projCtx_t;
using PJ_CONTEXT = struct projCtx_t;
#endif
struct PJconsts;
using PJ = struct PJconsts;

namespace mapnik {

class proj_init_error : public std::runtime_error
{
  public:
    proj_init_error(std::string const& params)
        : std::runtime_error("failed to initialize projection with: '" + params + "'")
    {}
};

class MAPNIK_DECL projection
{
    friend class proj_transform;

  public:

    projection(std::string const& params, bool defer_proj_init = false);
    projection(projection const& rhs);
    ~projection();

    projection& operator=(projection const& rhs);
    bool operator==(const projection& other) const;
    bool operator!=(const projection& other) const;
    bool is_initialized() const;
    bool is_geographic() const;
    boost::optional<well_known_srs_e> well_known() const;
    std::string const& params() const;
    void forward(double& x, double& y) const;
    void inverse(double& x, double& y) const;
    std::string expanded() const;
    void init_proj() const;

  private:
    void swap(projection& rhs);

  private:
    std::string params_;
    bool defer_proj_init_;
    mutable bool is_geographic_;
    mutable PJ* proj_;
    mutable PJ_CONTEXT* proj_ctx_;
};

template<typename charT, typename traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& s, mapnik::projection const& p)
{
    s << "projection(\"" << p.params() << "\")";
    return s;
}

} // namespace mapnik

#endif // MAPNIK_PROJECTION_HPP
