/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_TEXT_RENDERER_HPP
#define MAPNIK_TEXT_RENDERER_HPP

// mapnik
#include <mapnik/text/placement_finder.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/color_font_renderer.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_pixfmt_rgba.h"
#include <agg_trans_affine.h>
#pragma GCC diagnostic pop

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
}

#pragma GCC diagnostic pop

namespace mapnik
{

struct glyph_t
{
    FT_Glyph image;
    detail::evaluated_format_properties const& properties;
    pixel_position pos;
    rotation rot;
    double size;
    box2d<double> bbox;
    glyph_t(FT_Glyph image_,
            detail::evaluated_format_properties const& properties_,
            pixel_position const& pos_,
            rotation const& rot_,
            double size_,
            box2d<double> const& bbox_)
        : image(image_),
          properties(properties_),
          pos(pos_),
          rot(rot_),
          size(size_),
          bbox(bbox_) {}
};

class text_renderer : private util::noncopyable
{
public:
    text_renderer (halo_rasterizer_e rasterizer,
                   composite_mode_e comp_op = src_over,
                   composite_mode_e halo_comp_op = src_over,
                   double scale_factor = 1.0,
                   stroker_ptr stroker = stroker_ptr());

    void set_comp_op(composite_mode_e comp_op)
    {
        comp_op_ = comp_op;
    }

    void set_halo_comp_op(composite_mode_e halo_comp_op)
    {
        halo_comp_op_ = halo_comp_op;
    }

    void set_halo_rasterizer(halo_rasterizer_e rasterizer)
    {
        rasterizer_ = rasterizer;
    }

    void set_scale_factor(double scale_factor)
    {
        scale_factor_ = scale_factor;
    }

    void set_stroker(stroker_ptr stroker)
    {
        stroker_ = stroker;
    }

    void set_transform(agg::trans_affine const& transform);
    void set_halo_transform(agg::trans_affine const& halo_transform);

protected:
    using glyph_vector = std::vector<glyph_t>;
    void prepare_glyphs(glyph_positions const& positions);
    halo_rasterizer_e rasterizer_;
    composite_mode_e comp_op_;
    composite_mode_e halo_comp_op_;
    double scale_factor_;
    glyph_vector glyphs_;
    stroker_ptr stroker_;
    agg::trans_affine transform_;
    agg::trans_affine halo_transform_;
};

template <typename T>
class agg_text_renderer : public text_renderer
{
public:
    using pixmap_type = T;
    agg_text_renderer (pixmap_type & pixmap, halo_rasterizer_e rasterizer,
                       composite_mode_e comp_op = src_over,
                       composite_mode_e halo_comp_op = src_over,
                       double scale_factor = 1.0,
                       stroker_ptr stroker = stroker_ptr());
    void render(glyph_positions const& positions);
private:
    pixmap_type & pixmap_;

    template <std::size_t PixelWidth>
    void render_halo(unsigned char *buffer,
                     unsigned width,
                     unsigned height,
                     unsigned rgba, int x, int y,
                     double halo_radius, double opacity,
                     composite_mode_e comp_op);
};

template <typename T>
class grid_text_renderer : public text_renderer
{
public:
    using pixmap_type = T;
    grid_text_renderer (pixmap_type & pixmap,
                        composite_mode_e comp_op = src_over,
                        double scale_factor = 1.0);
    void render(glyph_positions const& positions, value_integer feature_id);
private:
    pixmap_type & pixmap_;

    template <std::size_t PixelWidth>
    void render_halo_id(unsigned char *buffer,
                        unsigned width,
                        unsigned height,
                        mapnik::value_integer feature_id,
                        int x, int y,
                        int halo_radius);
};

}
#endif // RENDERER_HPP
