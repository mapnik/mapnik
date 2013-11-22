/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

//mapnik
#include <mapnik/text/placement_finder.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/noncopyable.hpp>

//TODO: Find a better place for halo_rasterizer_e!
//TODO: Halo rasterizer selection should go to text_properties because it might make sense to use a different rasterizer for different fonts

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
}

namespace mapnik
{

struct glyph_t
{
    FT_Glyph image;
    char_properties_ptr properties;

    glyph_t(FT_Glyph image_, char_properties_ptr properties_)
        : image(image_), properties(properties_) {}

    glyph_t( glyph_t && other) noexcept
        : image(other.image),
          properties(std::move(other.properties))
    {
        other.image = nullptr;
    }

    glyph_t(glyph_t const& ) = delete;
    glyph_t & operator=(glyph_t const&) = delete;

    ~glyph_t () { FT_Done_Glyph(image);}

};

class text_renderer : private mapnik::noncopyable
{
public:
    text_renderer (halo_rasterizer_e rasterizer,
                   composite_mode_e comp_op = src_over,
                   double scale_factor=1.0,
                   stroker_ptr stroker=stroker_ptr());
protected:
    typedef std::vector<glyph_t> glyph_vector;
    void prepare_glyphs(glyph_positions const& positions);
    halo_rasterizer_e rasterizer_;
    composite_mode_e comp_op_;
    double scale_factor_;
    glyph_vector glyphs_;
    stroker_ptr stroker_;
};

template <typename T>
class agg_text_renderer : public text_renderer
{
public:
    typedef T pixmap_type;
    agg_text_renderer (pixmap_type & pixmap, halo_rasterizer_e rasterizer,
                       composite_mode_e comp_op = src_over,
                       double scale_factor=1.0,
                       stroker_ptr stroker=stroker_ptr());
    void render(glyph_positions const& positions);
private:
    pixmap_type & pixmap_;
    void render_halo(FT_Bitmap_ *bitmap, unsigned rgba, int x, int y,
                     double halo_radius, double opacity,
                     composite_mode_e comp_op);
};

template <typename T>
class grid_text_renderer : public text_renderer
{
public:
    typedef T pixmap_type;
    grid_text_renderer (pixmap_type & pixmap, composite_mode_e comp_op = src_over,
                        double scale_factor=1.0);
    void render(glyph_positions const& positions, value_integer feature_id);
private:
    pixmap_type & pixmap_;
    void render_halo_id(FT_Bitmap_ *bitmap, mapnik::value_integer feature_id, int x, int y, int halo_radius);
};

}
#endif // RENDERER_HPP
