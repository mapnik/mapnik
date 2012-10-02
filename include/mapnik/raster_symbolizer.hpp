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

#ifndef MAPNIK_RASTER_SYMBOLIZER_HPP
#define MAPNIK_RASTER_SYMBOLIZER_HPP

// mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/image_scaling.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

namespace mapnik
{

struct MAPNIK_DECL raster_symbolizer : public symbolizer_base
{
    raster_symbolizer();
    raster_symbolizer(raster_symbolizer const& rhs);

    std::string const& get_mode() const;
    void set_mode(std::string const& mode);
    scaling_method_e get_scaling_method() const;
    void set_scaling_method(scaling_method_e scaling);
    void set_opacity(float opacity);
    float get_opacity() const;
    raster_colorizer_ptr get_colorizer() const;
    void set_colorizer(raster_colorizer_ptr const& colorizer);
    double get_filter_factor() const;
    void set_filter_factor(double filter_factor);
    double calculate_filter_factor() const;
    unsigned get_mesh_size() const;
    void set_mesh_size(unsigned mesh_size);
    boost::optional<bool> premultiplied() const;
    void set_premultiplied(bool premultiplied);

private:
    std::string mode_;
    scaling_method_e scaling_;
    float opacity_;
    raster_colorizer_ptr colorizer_;
    double filter_factor_;
    unsigned mesh_size_;
    boost::optional<bool> premultiplied_;
};
}

#endif // MAPNIK_RASTER_SYMBOLIZER_HPP
