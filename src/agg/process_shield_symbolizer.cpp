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
#include <mapnik/image_util.hpp>
#include <mapnik/image_cache.hpp>
#include <mapnik/svg/marker_cache.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"

namespace mapnik {

template <typename T>
void  agg_renderer<T>::process(shield_symbolizer const& sym,
                               Feature const& feature,
                               proj_transform const& prj_trans)
{
    typedef  coord_transform2<CoordTransform,geometry2d> path_type;
    typedef agg::pixfmt_rgba32 pixfmt;
    typedef agg::renderer_base<pixfmt> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;
    

    metawriter_with_properties writer = sym.get_metawriter();

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
    
    if ( sym.get_text_convert() == TOUPPER)
    {
        text = text.toUpper();
    }
    else if ( sym.get_text_convert() == TOLOWER)
    {
        text = text.toLower();
    }
    
    agg::trans_affine tr;
    boost::array<double,6> const& m = sym.get_transform();
    tr.load_from(&m[0]);
    tr = agg::trans_affine_scaling(scale_factor_) * tr;
    
    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);
    
    if (is_svg(filename)) //SVG Label
    {
        using namespace mapnik::svg;
        boost::optional<path_ptr> marker = marker_cache::instance()->find(filename, true);
        ras_ptr->reset();
        ras_ptr->gamma(agg::gamma_linear());
        agg::scanline_u8 sl;
        agg::rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_ * 4);
        pixfmt pixf(buf);
        renderer_base renb(pixf);
        renderer_solid ren(renb);
        
        if (text.length() > 0 && marker && *marker)
        {
            face_set_ptr faces;
            
            box2d<double> const& bbox = (*marker)->bounding_box();
            double x1 = bbox.minx();
            double y1 = bbox.miny();
            double x2 = bbox.maxx();
            double y2 = bbox.maxy();
            tr.transform(&x1,&y1);
            tr.transform(&x2,&y2);
            
            vertex_stl_adapter<svg_path_storage> stl_storage((*marker)->source());
            svg_path_adapter svg_path(stl_storage);
            svg_renderer<svg_path_adapter,
                         agg::pod_bvector<path_attributes> > svg_renderer(svg_path,
                                                                          (*marker)->attributes());
            
            int w = int(x2 - x1);
            int h = int(y2 - y1);
            
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
                text_renderer<T> text_ren(pixmap_, faces, *strk);
                
                text_ren.set_pixel_size(sym.get_text_size() * scale_factor_);
                text_ren.set_fill(sym.get_fill());
                text_ren.set_halo_fill(sym.get_halo_fill());
                text_ren.set_halo_radius(sym.get_halo_radius() * scale_factor_);
                text_ren.set_opacity(sym.get_text_opacity());
                
                placement_finder<label_collision_detector4> finder(detector_);

                string_info info(text);

                faces->get_string_info(info);
                
                unsigned num_geom = feature.num_geometries();
                
                for (unsigned i=0;i<num_geom;++i)
                {
                    geometry2d const& geom = feature.get_geometry(i);
                    if (geom.num_points() > 0 )
                    {
                        path_type path(t_,geom,prj_trans);
                        
                        label_placement_enum how_placed = sym.get_label_placement();
                        if (how_placed == POINT_PLACEMENT || how_placed == VERTEX_PLACEMENT)
                        {
                            // for every vertex, try and place a shield/text
                            geom.rewind(0);
                            for( unsigned jj = 0; jj < geom.num_points(); jj++ )
                            {
                                double label_x;
                                double label_y;
                                double z=0.0;
                                placement text_placement(info, sym, w, h, false);
                                text_placement.avoid_edges = sym.get_avoid_edges();
                                text_placement.allow_overlap = sym.get_allow_overlap();
                                if( how_placed == VERTEX_PLACEMENT )
                                    geom.vertex(&label_x,&label_y);  // by vertex
                                else
                                    geom.middle_point(&label_x, &label_y);  // by middle of line
                                
                                prj_trans.backward(label_x,label_y, z);
                                t_.forward(&label_x,&label_y);
                                
                                position const& shield_pos = sym.get_shield_displacement();
                                label_x += boost::get<0>(shield_pos);
                                label_y += boost::get<1>(shield_pos);
                                
                                finder.find_point_placement( text_placement,label_x,label_y,0.0,
                                                             sym.get_vertical_alignment(),
                                                             sym.get_line_spacing() * scale_factor_,
                                                             sym.get_character_spacing() * scale_factor_,
                                                             sym.get_horizontal_alignment(),
                                                             sym.get_justify_alignment() );
                                
                                // check to see if image overlaps anything too, there is only ever 1 placement found for points and verticies
                                if( text_placement.placements.size() > 0)
                                {
                                    double x = text_placement.placements[0].starting_x;
                                    double y = text_placement.placements[0].starting_y;
                                    int px;
                                    int py;
                                    box2d<double> label_ext;

                                    if( !sym.get_unlock_image() )
                                    {  // center image at text center position
                                        // remove displacement from image label
                                        position const& pos = sym.get_displacement();
                                        double lx = x - boost::get<0>(pos);
                                        double ly = y - boost::get<1>(pos);
                                        px=int(floor(lx - (0.5 * w))) ;
                                        py=int(floor(ly - (0.5 * h))) ;
                                        label_ext.init( floor(lx - 0.5 * w), floor(ly - 0.5 * h), 
                                                        ceil (lx + 0.5 * w), ceil (ly + 0.5 * h) );
                                    }
                                    else
                                    {  // center image at reference location
                                        px=int(floor(label_x - 0.5 * w));
                                        py=int(floor(label_y - 0.5 * h));
                                        label_ext.init( floor(label_x - 0.5 * w), floor(label_y - 0.5 * h), 
                                                        ceil (label_x + 0.5 * w), ceil (label_y + 0.5 * h));
                                    }
                                    
                                    if ( sym.get_allow_overlap() || detector_.has_placement(label_ext) )
                                    {
                                        agg::trans_affine matrix = tr * agg::trans_affine_translation(px, py);
                                        svg_renderer.render(*ras_ptr, sl, ren, matrix, renb.clip_box(), sym.get_opacity());
                                        box2d<double> dim = text_ren.prepare_glyphs(&text_placement.placements[0]);
                                        text_ren.render(x,y);
                                        detector_.insert(label_ext);
                                        finder.update_detector(text_placement);
                                        if (writer.first) {
                                            writer.first->add_box(box2d<double>(px,py,px+w,py+h), feature, t_, writer.second);
                                            writer.first->add_text(text_placement, faces, feature, t_, writer.second);
                                        }
                                    }
                                }
                            }
                        }

                        else if (geom.num_points() > 1 && sym.get_label_placement() == LINE_PLACEMENT)
                        {
                            placement text_placement(info, sym, w, h, true);
                            text_placement.avoid_edges = sym.get_avoid_edges();
                            finder.find_point_placements<path_type>(text_placement,path);
                        
                            for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ ii)
                            {
                                
                                double x = text_placement.placements[ii].starting_x;
                                double y = text_placement.placements[ii].starting_y;
                                
                                int px=int(x + 0.5);
                                int py=int(y + 0.5);
                                agg::trans_affine matrix = tr * agg::trans_affine_translation(px, py);
                                
                                svg_renderer.render(*ras_ptr, sl, ren, matrix, renb.clip_box(), sym.get_opacity());
                                if (writer.first) writer.first->add_box(box2d<double>(px,py,px+w,py+h), feature, t_, writer.second);
                                box2d<double> dim = text_ren.prepare_glyphs(&text_placement.placements[ii]);
                                text_ren.render(x,y);
                            }
                            finder.update_detector(text_placement);
                            if (writer.first) writer.first->add_text(text_placement, faces, feature, t_, writer.second);
                        }
                    }
                }
            }
        }    
    }
    else
    {
        boost::optional<image_ptr> data = image_cache::instance()->find(filename,true);
    
        if (text.length() > 0 && data)
        {
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


                int w = (*data)->width();
                int h = (*data)->height();

                unsigned num_geom = feature.num_geometries();
                for (unsigned i=0;i<num_geom;++i)
                {
                    geometry2d const& geom = feature.get_geometry(i);
                    if (geom.num_points() > 0 )
                    {
                        path_type path(t_,geom,prj_trans);

                        label_placement_enum how_placed = sym.get_label_placement();
                        if (how_placed == POINT_PLACEMENT || how_placed == VERTEX_PLACEMENT)
                        {
                            // for every vertex, try and place a shield/text
                            geom.rewind(0);
                            for( unsigned jj = 0; jj < geom.num_points(); jj++ )
                            {
                                double label_x;
                                double label_y;
                                double z=0.0;
                                placement text_placement(info, sym, w, h, false);
                                text_placement.avoid_edges = sym.get_avoid_edges();
                                text_placement.allow_overlap = sym.get_allow_overlap();
                                if( how_placed == VERTEX_PLACEMENT )
                                    geom.vertex(&label_x,&label_y);  // by vertex
                                else
                                    geom.label_position(&label_x, &label_y);  // by middle of line or by point
                                prj_trans.backward(label_x,label_y, z);
                                t_.forward(&label_x,&label_y);
                                position const& shield_pos = sym.get_shield_displacement();
                                label_x += boost::get<0>(shield_pos);
                                label_y += boost::get<1>(shield_pos);
                                
                                finder.find_point_placement( text_placement,label_x,label_y,0.0,
                                                             sym.get_vertical_alignment(),
                                                             sym.get_line_spacing(),
                                                             sym.get_character_spacing(),
                                                             sym.get_horizontal_alignment(),
                                                             sym.get_justify_alignment() );

                                // check to see if image overlaps anything too, there is only ever 1 placement found for points and verticies
                                if( text_placement.placements.size() > 0)
                                {
                                    double x = text_placement.placements[0].starting_x;
                                    double y = text_placement.placements[0].starting_y;
                                    int px;
                                    int py;
                                    box2d<double> label_ext;

                                    if( !sym.get_unlock_image() )
                                    {  // center image at text center position
                                        // remove displacement from image label
                                        position pos = sym.get_displacement();
                                        double lx = x - boost::get<0>(pos);
                                        double ly = y - boost::get<1>(pos);
                                        px=int(floor(lx - (0.5 * w))) ;
                                        py=int(floor(ly - (0.5 * h))) ;
                                        label_ext.init( floor(lx - 0.5 * w), floor(ly - 0.5 * h), ceil (lx + 0.5 * w), ceil (ly + 0.5 * h) );
                                    }
                                    else
                                    {  // center image at reference location
                                        px=int(floor(label_x - 0.5 * w));
                                        py=int(floor(label_y - 0.5 * h));
                                        label_ext.init( floor(label_x - 0.5 * w), floor(label_y - 0.5 * h), ceil (label_x + 0.5 * w), ceil (label_y + 0.5 * h));
                                    }

                                    if ( sym.get_allow_overlap() || detector_.has_placement(label_ext) )
                                    {
                                        //pixmap_.set_rectangle_alpha(px,py,*data);
                                        pixmap_.set_rectangle_alpha2(*(*data),px,py,sym.get_opacity());
                                        box2d<double> dim = ren.prepare_glyphs(&text_placement.placements[0]);
                                        ren.render(x,y);
                                        detector_.insert(label_ext);
                                        finder.update_detector(text_placement);
                                        if (writer.first) {
                                            writer.first->add_box(box2d<double>(px,py,px+w,py+h), feature, t_, writer.second);
                                            writer.first->add_text(text_placement, faces, feature, t_, writer.second);
                                        }
                                    }
                                }
                            }
                        }

                        else if (geom.num_points() > 1 && sym.get_label_placement() == LINE_PLACEMENT)
                        {
                            placement text_placement(info, sym, w, h, true);
                            text_placement.avoid_edges = sym.get_avoid_edges();
                            finder.find_point_placements<path_type>(text_placement,path);
                        
                            for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ ii)
                            {
                            
                                double x = text_placement.placements[ii].starting_x;
                                double y = text_placement.placements[ii].starting_y;
                        
                                int px=int(x - (w/2));
                                int py=int(y - (h/2));
                         
                                pixmap_.set_rectangle_alpha(px,py,*(*data));
                                if (writer.first) writer.first->add_box(box2d<double>(px,py,px+w,py+h), feature, t_, writer.second);
                         
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
}


template void agg_renderer<image_32>::process(shield_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
