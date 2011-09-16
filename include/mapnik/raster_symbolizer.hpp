/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
//$Id$

#ifndef RASTER_SYMBOLIZER_HPP
#define RASTER_SYMBOLIZER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/image_util.hpp>

namespace mapnik
{
struct MAPNIK_DECL raster_symbolizer : public symbolizer_base 
{
    
    raster_symbolizer()
        : symbolizer_base(),
        mode_("normal"),
        scaling_("fast"),
        opacity_(1.0),
        colorizer_(),
        filter_factor_(-1),
        mesh_size_(16) {}

    raster_symbolizer(const raster_symbolizer &rhs)
        : symbolizer_base(rhs),
        mode_(rhs.get_mode()),
        scaling_(rhs.get_scaling()),
        opacity_(rhs.get_opacity()),
        colorizer_(rhs.colorizer_),
        filter_factor_(rhs.filter_factor_),
        mesh_size_(rhs.mesh_size_) {}
    
    std::string const& get_mode() const
    {
        return mode_;
    }
    void set_mode(std::string const& mode)
    {
        mode_ = mode;
    }
    std::string const& get_scaling() const
    {
        return scaling_;
    }
    void set_scaling(std::string const& scaling)
    {
        scaling_ = scaling;
    }
    void set_opacity(float opacity)
    {
        opacity_ = opacity;
    }
    float get_opacity() const
    {
        return opacity_;
    }
    raster_colorizer_ptr get_colorizer() const
    {
        return colorizer_;
    }
    void set_colorizer(raster_colorizer_ptr const& colorizer)
    {
        colorizer_ = colorizer;
    }
    double get_filter_factor() const
    {
        return filter_factor_;
    }
    void set_filter_factor(double filter_factor)
    {
        filter_factor_=filter_factor;
    }
    double calculate_filter_factor() const
    {
        if (filter_factor_ > 0)
        {
            // respect explicitly specified values
            return filter_factor_;
        } else {
            // No filter factor specified, calculate a sensible default value
            // based on the scaling algorithm being employed.
            scaling_method_e scaling = get_scaling_method_by_name (scaling_);
            
            double ff = 1.0;
            
            switch(scaling)
            {
                case SCALING_NEAR:
                    ff = 1.0;
                    break;
                
                // TODO potentially some of these algorithms would use filter_factor >2.0.
                // Contributions welcome from someone who knows more about them.
                case SCALING_BILINEAR:
                case SCALING_BICUBIC:
                case SCALING_SPLINE16:
                case SCALING_SPLINE36:
                case SCALING_HANNING:
                case SCALING_HAMMING:
                case SCALING_HERMITE:
                case SCALING_KAISER:
                case SCALING_QUADRIC:
                case SCALING_CATROM:
                case SCALING_GAUSSIAN:
                case SCALING_BESSEL:
                case SCALING_MITCHELL:
                case SCALING_SINC:
                case SCALING_LANCZOS:
                case SCALING_BLACKMAN:
                    ff = 2.0;
                    break;
                default:
                    ff = 1.0;
                    break;
            }
            return ff;
        }
    }
    unsigned get_mesh_size() const
    {
        return mesh_size_;
    }
    void set_mesh_size(unsigned mesh_size)
    {
        mesh_size_=mesh_size;
    }
    
    
private:
    std::string mode_;
    std::string scaling_;
    float opacity_;
    raster_colorizer_ptr colorizer_;
    double filter_factor_;
    unsigned mesh_size_;
};
}

#endif //RASTER_SYMBOLIZER_HPP
