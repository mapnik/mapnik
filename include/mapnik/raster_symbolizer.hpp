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
#include <mapnik/config.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_scaling.hpp>

// boost
#include <boost/algorithm/string.hpp>

namespace mapnik
{
struct MAPNIK_DECL raster_symbolizer : public symbolizer_base
{

    raster_symbolizer()
        : symbolizer_base(),
        mode_("normal"),
        scaling_(SCALING_NEAR),
        opacity_(1.0),
        colorizer_(),
        filter_factor_(-1),
        mesh_size_(16) {}

    raster_symbolizer(const raster_symbolizer &rhs)
        : symbolizer_base(rhs),
        mode_(rhs.mode_),
        scaling_(rhs.scaling_),
        opacity_(rhs.opacity_),
        colorizer_(rhs.colorizer_),
        filter_factor_(rhs.filter_factor_),
        mesh_size_(rhs.mesh_size_) {}

    std::string const& get_mode() const
    {
        MAPNIK_LOG_ERROR(raster_symbolizer) << "getting 'mode' is deprecated and will be removed in Mapnik 3.x, use 'comp-op' with Mapnik >= 2.1.x";
        return mode_;
    }
    void set_mode(std::string const& mode)
    {
        MAPNIK_LOG_ERROR(raster_symbolizer) << "setting 'mode' is deprecated and will be removed in Mapnik 3.x, use 'comp-op' with Mapnik >= 2.1.x";
        mode_ = mode;
        if (mode == "normal")
        {
            MAPNIK_LOG_ERROR(raster_symbolizer) << "converting 'mode=normal' to 'comp-op:src_over'";
            this->set_comp_op(src_over);
        }
        else
        {
            std::string mode2 = boost::algorithm::replace_last_copy(mode,"2","");
            boost::optional<composite_mode_e> comp_op = comp_op_from_string(mode2);
            if (comp_op)
            {
                MAPNIK_LOG_ERROR(raster_symbolizer) << "converting 'mode:" << mode << "' to 'comp-op:" + *comp_op_to_string(*comp_op) + "'";
                this->set_comp_op(*comp_op);
            }
            else
            {
                MAPNIK_LOG_ERROR(raster_symbolizer) << "could not convert mode '" << mode << "' into comp-op, defaulting to 'comp-op:src-over'";
            }
        }
    }
    scaling_method_e get_scaling_method() const
    {
        return scaling_;
    }
    void set_scaling_method(scaling_method_e scaling)
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
            double ff = 1.0;

            switch(scaling_)
            {
            case SCALING_NEAR:
                ff = 1.0;
                break;

                // TODO potentially some of these algorithms would use filter_factor >2.0.
                // Contributions welcome from someone who knows more about them.
            case SCALING_BILINEAR:
            case SCALING_BILINEAR8:
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
    scaling_method_e scaling_;
    float opacity_;
    raster_colorizer_ptr colorizer_;
    double filter_factor_;
    unsigned mesh_size_;
};
}

#endif // MAPNIK_RASTER_SYMBOLIZER_HPP
