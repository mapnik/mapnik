/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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

// mapnik
#include <mapnik/agg_renderer.hpp>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(glyph_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    face_set_ptr faces = font_manager_.get_face_set(sym.get_face_name());
    stroker_ptr strk = font_manager_.get_stroker();
    if (faces->size() > 0 && strk)
    {
        // Get x and y from geometry and translate to pixmap coords.
        double x, y, z=0.0;
        feature.get_geometry(0).label_position(&x, &y);
        prj_trans.backward(x,y,z);
        t_.forward(&x, &y);

        text_renderer<T> ren(pixmap_, faces, *strk);

        // set fill and halo colors
        color fill = sym.eval_color(feature);
        ren.set_fill(fill);
        if (fill != color("transparent")) {
            ren.set_halo_fill(sym.get_halo_fill());
            ren.set_halo_radius(sym.get_halo_radius() * scale_factor_);
        }

        // set font size
        unsigned size = sym.eval_size(feature);
        ren.set_pixel_size(size * scale_factor_);
        faces->set_pixel_sizes(size * scale_factor_);

        // Get and render text path
        //
        text_path_ptr path = sym.get_text_path(faces, feature);
        // apply displacement
        position pos = sym.get_displacement();
        double dx = boost::get<0>(pos);
        double dy = boost::get<1>(pos);
        path->starting_x = x = x+dx;
        path->starting_y = y = y+dy;

        // Prepare glyphs to set internal state and calculate the marker's
        // final box so we can check for a valid placement
        box2d<double> dim = ren.prepare_glyphs(path.get());
        box2d<double> ext(x-dim.width()/2, y-dim.height()/2, x+dim.width()/2, y+dim.height()/2);
        if ((sym.get_allow_overlap() || detector_.has_placement(ext)) &&
            (!sym.get_avoid_edges() || detector_.extent().contains(ext)))
        {    
            // Placement is valid, render glyph and update detector.
            ren.render(x, y);
            detector_.insert(ext);
            metawriter_with_properties writer = sym.get_metawriter();
            if (writer.first) writer.first->add_box(ext, feature, t_, writer.second);
        }
    }
    else
    {
        throw config_error(
            "Unable to find specified font face in GlyphSymbolizer"
            );
    }
}
template void agg_renderer<image_32>::process(glyph_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);
}
