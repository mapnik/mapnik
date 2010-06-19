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
#include <mapnik/agg_rasterizer.hpp>
/*
#include <mapnik/unicode.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/text_path.hpp>
*/

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(text_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    typedef  coord_transform2<CoordTransform,geometry2d> path_type;

    expression_ptr name_expr = sym.get_name();
    if (!name_expr) return;
    value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature),*name_expr);
    UnicodeString text = result.to_unicode();
    
    if ( sym.get_text_convert() == TOUPPER)
    {
        text = text.toUpper();
    }
    else if ( sym.get_text_convert() == TOLOWER)
    {
        text = text.toLower();
    }

    if ( text.length() > 0 )
    {
        color const& fill = sym.get_fill();

        face_set_ptr faces;

        if (sym.get_fontset().size() > 0)
        {
            faces = font_manager_.get_face_set(sym.get_fontset());
        }
        else
        {
            faces = font_manager_.get_face_set(sym.get_face_name());
        }

        stroker_ptr strk = font_manager_.get_stroker();
        if (faces->size() > 0 && strk)
        {
            text_renderer<T> ren(pixmap_, faces, *strk);
            ren.set_pixel_size(sym.get_text_size() * scale_factor_);
            ren.set_fill(fill);
            ren.set_halo_fill(sym.get_halo_fill());
            ren.set_halo_radius(sym.get_halo_radius() * scale_factor_);
            ren.set_opacity(sym.get_text_opacity());

            placement_finder<label_collision_detector4> finder(detector_);

            string_info info(text);

            faces->get_string_info(info);
            unsigned num_geom = feature.num_geometries();
            for (unsigned i=0;i<num_geom;++i)
            {
                geometry2d const& geom = feature.get_geometry(i);
                if (geom.num_points() > 0) // don't bother with empty geometries
                {           
                    placement text_placement(info,sym);
                    text_placement.avoid_edges = sym.get_avoid_edges();
                    if (sym.get_label_placement() == POINT_PLACEMENT)
                    {
                        double label_x, label_y, z=0.0;
                        geom.label_position(&label_x, &label_y);
                        prj_trans.backward(label_x,label_y, z);
                        t_.forward(&label_x,&label_y);

                        double angle = 0.0;
                        expression_ptr angle_expr = sym.get_orientation();
                        if (angle_expr)
                        {
                            // apply rotation
                            value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature),*angle_expr);
                            angle = result.to_double();
                        }
                        
                        finder.find_point_placement(text_placement,label_x,label_y, angle, sym.get_vertical_alignment(),sym.get_line_spacing(),
                                                    sym.get_character_spacing(),sym.get_horizontal_alignment(),sym.get_justify_alignment());
                                
                        finder.update_detector(text_placement);
                    }
                    else if ( geom.num_points() > 1 && sym.get_label_placement() == LINE_PLACEMENT)
                    {
                        path_type path(t_,geom,prj_trans);
                        finder.find_line_placements<path_type>(text_placement,path);
                    }
                    
                    for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ii)
                    {
                        double x = text_placement.placements[ii].starting_x;
                        double y = text_placement.placements[ii].starting_y;
                        box2d<double> dim = ren.prepare_glyphs(&text_placement.placements[ii]);
                        ren.render(x,y);
                    }
                }
            }
        }
        else
        {
            throw config_error("Unable to find specified font face '" + sym.get_face_name() + "'");
        }
    }
}

template void agg_renderer<image_32>::process(text_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
 
