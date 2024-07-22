/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_MEMORY_DATASOURCE_HPP
#define MAPNIK_MEMORY_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/datasource_plugin.hpp>

// stl
#include <deque>

namespace mapnik {

DATASOURCE_PLUGIN_DEF(memory_datasource_plugin, memory)
class MAPNIK_DECL memory_datasource : public datasource
{
    friend class memory_featureset;

  public:
    memory_datasource(parameters const& params);
    static const char* name();
    virtual ~memory_datasource();
    virtual datasource::datasource_t type() const;
    virtual featureset_ptr features(query const& q) const;
    virtual featureset_ptr features_at_point(coord2d const& pt, double tol = 0) const;
    virtual box2d<double> envelope() const;
    virtual std::optional<datasource_geometry_t> get_geometry_type() const;
    virtual layer_descriptor get_descriptor() const;
    //
    void push(feature_ptr feature);
    void set_envelope(box2d<double> const& box);
    size_t size() const;
    void clear();

  private:
    std::deque<feature_ptr> features_;
    mapnik::layer_descriptor desc_;
    datasource::datasource_t type_;
    bool bbox_check_;
    bool type_set_;
    mutable box2d<double> extent_;
    mutable bool dirty_extent_ = true;
};

} // namespace mapnik

#endif // MAPNIK_MEMORY_DATASOURCE_HPP
