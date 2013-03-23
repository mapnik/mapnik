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
//TODO: Find a better place for halo_rasterizer_e!
//TODO: Halo rasterizer selection should go to text_properties because it might make sense to use a different rasterizer for different fonts

//boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

struct FT_Bitmap_;

namespace mapnik
{

struct glyph_t;

class text_renderer : private boost::noncopyable
{
public:
    text_renderer (halo_rasterizer_e rasterizer, composite_mode_e comp_op = src_over, double scale_factor=1.0, stroker_ptr stroker=stroker_ptr());
protected:
    typedef boost::ptr_vector<glyph_t> glyph_vector;
    void prepare_glyphs(glyph_positions_ptr pos);

    halo_rasterizer_e rasterizer_;
    composite_mode_e comp_op_;
    double scale_factor_;
    boost::shared_ptr<glyph_vector> glyphs_;
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
    void render(glyph_positions_ptr pos);
private:
    pixmap_type & pixmap_;
    void render_halo(FT_Bitmap_ *bitmap, unsigned rgba, int x, int y,
                     int halo_radius, double opacity,
                     composite_mode_e comp_op);
};

template <typename T>
class grid_text_renderer : public text_renderer
{
public:
    typedef T pixmap_type;
    grid_text_renderer (pixmap_type & pixmap, composite_mode_e comp_op = src_over,
                        double scale_factor=1.0);
    void render(glyph_positions_ptr pos, value_integer feature_id);
private:
    pixmap_type & pixmap_;
    void render_halo_id(FT_Bitmap_ *bitmap, mapnik::value_integer feature_id, int x, int y, int halo_radius);
};

}
#endif // RENDERER_HPP
