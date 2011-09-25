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

#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/expression_evaluator.hpp>

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_scanline_u.h"

// boost
#include <boost/make_shared.hpp>

namespace mapnik {

template <typename T>
void  agg_renderer<T>::process(shield_symbolizer const& sym,
                               Feature const& feature,
                               proj_transform const& prj_trans)
{
    typedef  coord_transform2<CoordTransform,geometry_type> path_type;


    text_placement_info_ptr placement_options = sym.get_placement_options()->get_placement_info();
    placement_options->next();
    placement_options->next_position_only();

    UnicodeString text;
    if( sym.get_no_text() )
        text = UnicodeString( " " );  // TODO: fix->use 'space' as the text to render
    else
    {
        expression_ptr name_expr = sym.get_name();
        if (!name_expr) return;
        value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature),*name_expr);
        text = result.to_unicode();
    }
    
    if ( sym.get_text_transform() == UPPERCASE)
    {
        text = text.toUpper();
    }
    else if ( sym.get_text_transform() == LOWERCASE)
    {
        text = text.toLower();
    }
    else if ( sym.get_text_transform() == CAPITALIZE)
    {
        text = text.toTitle(NULL);
    }
    
    agg::trans_affine tr;
    boost::array<double,6> const& m = sym.get_transform();
    tr.load_from(&m[0]);
    
    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);
    boost::optional<mapnik::marker_ptr> marker;
    if ( !filename.empty() )
    {
        marker = marker_cache::instance()->find(filename, true);
    }
    else
    {
        marker.reset(boost::make_shared<mapnik::marker>());
    }
    
    
    if (text.length() > 0 && marker)
    {
        int w = (*marker)->width();
        int h = (*marker)->height();
        
        double px0 = - 0.5 * w;
        double py0 = - 0.5 * h;
        double px1 = 0.5 * w;
        double py1 = 0.5 * h;
        double px2 = px1;
        double py2 = py0;
        double px3 = px0;
        double py3 = py1;
        tr.transform(&px0,&py0);
        tr.transform(&px1,&py1);
        tr.transform(&px2,&py2);
        tr.transform(&px3,&py3);
        box2d<double> label_ext (px0, py0, px1, py1);
        label_ext.expand_to_include(px2, py2);
        label_ext.expand_to_include(px3, py3);
        
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
        if (strk && faces->size() > 0)
        {
            text_renderer<T> ren(pixmap_, faces, *strk);

            ren.set_pixel_size(sym.get_text_size() * scale_factor_);
            ren.set_fill(sym.get_fill());
            ren.set_halo_fill(sym.get_halo_fill());
            ren.set_halo_radius(sym.get_halo_radius() * scale_factor_);
            ren.set_opacity(sym.get_text_opacity());

            placement_finder<label_collision_detector4> finder(detector_);

            string_info info(text);

            faces->get_string_info(info);

            metawriter_with_properties writer = sym.get_metawriter();

            for (unsigned i = 0; i < feature.num_geometries(); ++i)
            {
                geometry_type const& geom = feature.get_geometry(i);
                if (geom.num_points() > 0 )
                {
                    path_type path(t_,geom,prj_trans);

                    label_placement_enum how_placed = sym.get_label_placement();
                    if (how_placed == POINT_PLACEMENT || how_placed == VERTEX_PLACEMENT || how_placed == INTERIOR_PLACEMENT)
                    {
                        // for every vertex, try and place a shield/text
                        geom.rewind(0);
                        placement text_placement(info, sym, scale_factor_, w, h, false);
                        text_placement.avoid_edges = sym.get_avoid_edges();
                        text_placement.allow_overlap = sym.get_allow_overlap();
                        if (writer.first)
                            text_placement.collect_extents =true; // needed for inmem metawriter
                        position const& pos = sym.get_displacement();
                        position const& shield_pos = sym.get_shield_displacement();
                        for( unsigned jj = 0; jj < geom.num_points(); jj++ )
                        {
                            double label_x;
                            double label_y;
                            double z=0.0;

                            if( how_placed == VERTEX_PLACEMENT )
                                geom.vertex(&label_x,&label_y);  // by vertex
                            else if( how_placed == INTERIOR_PLACEMENT )
                                geom.label_interior_position(&label_x,&label_y);
                            else
                                geom.label_position(&label_x, &label_y);  // by middle of line or by point
                            prj_trans.backward(label_x,label_y, z);
                            t_.forward(&label_x,&label_y);

                            label_x += boost::get<0>(shield_pos);
                            label_y += boost::get<1>(shield_pos);

                            finder.find_point_placement( text_placement, placement_options,
                                                         label_x, label_y, 0.0,
                                                         sym.get_line_spacing(),
                                                         sym.get_character_spacing());

                            // check to see if image overlaps anything too, there is only ever 1 placement found for points and verticies
                            if( text_placement.placements.size() > 0)
                            {
                                double x = floor(text_placement.placements[0].starting_x);
                                double y = floor(text_placement.placements[0].starting_y);
                                int px;
                                int py;
                                
                                if( !sym.get_unlock_image() )
                                {
                                    // center image at text center position
                                    // remove displacement from image label
                                    double lx = x - boost::get<0>(pos);
                                    double ly = y - boost::get<1>(pos);
                                    px=int(floor(lx - (0.5 * w))) + 1;
                                    py=int(floor(ly - (0.5 * h))) + 1;
                                    label_ext.re_center(lx,ly);
                                }
                                else
                                {  // center image at reference location
                                    px=int(floor(label_x - 0.5 * w));
                                    py=int(floor(label_y - 0.5 * h));
                                    label_ext.re_center(label_x,label_y);
                                }
                                
                                if ( sym.get_allow_overlap() || detector_.has_placement(label_ext) )
                                {
                                    render_marker(px,py,**marker,tr,sym.get_opacity());

                                    box2d<double> dim = ren.prepare_glyphs(&text_placement.placements[0]);
                                    ren.render(x,y);
                                    detector_.insert(label_ext);
                                    finder.update_detector(text_placement);
                                    if (writer.first) {
                                        writer.first->add_box(label_ext, feature, t_, writer.second);
                                        writer.first->add_text(text_placement, faces, feature, t_, writer.second);
                                    }
                                }
                            }
                        }
                    }

                    else if (geom.num_points() > 1 && how_placed == LINE_PLACEMENT)
                    {
                        placement text_placement(info, sym, scale_factor_, label_ext.width(), label_ext.height(), true);
                        
                        text_placement.avoid_edges = sym.get_avoid_edges();
                        finder.find_point_placements<path_type>(text_placement, placement_options, path);

                        position const&  pos = sym.get_displacement();
                        for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ ii)
                        {
                            double x = floor(text_placement.placements[ii].starting_x);
                            double y = floor(text_placement.placements[ii].starting_y);

                            double lx = x - boost::get<0>(pos);
                            double ly = y - boost::get<1>(pos);
                            int px=int(floor(lx - (0.5*w))) + 1;
                            int py=int(floor(ly - (0.5*h))) + 1;

                            render_marker(px,py,**marker,tr,sym.get_opacity());

                            if (writer.first) writer.first->add_box(label_ext, feature, t_, writer.second);

                            box2d<double> dim = ren.prepare_glyphs(&text_placement.placements[ii]);
                            ren.render(x,y);
                        }
                        finder.update_detector(text_placement);
                        if (writer.first) writer.first->add_text(text_placement, faces, feature, t_, writer.second);
                    }
                }
            }
        }
    }
}


template void agg_renderer<image_32>::process(shield_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
