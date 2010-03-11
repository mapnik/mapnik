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

// mapnik
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/markers_converter.hpp>
#include <mapnik/arrow.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/font_set.hpp>

// agg
#define AGG_RENDERING_BUFFER row_ptr_cache<int8u>
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_basics.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_path_storage.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_image_accessors.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_conv_contour.h"
#include "agg_conv_clip_polyline.h"
#include "agg_vcgen_stroke.h"
#include "agg_conv_adaptor_vcgen.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_conv_marker.h"
#include "agg_vcgen_markers_term.h"
#include "agg_renderer_outline_aa.h"
#include "agg_rasterizer_outline_aa.h"
#include "agg_rasterizer_outline.h"
#include "agg_renderer_outline_image.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_renderer_scanline.h"
#include "agg_pattern_filters_rgba.h"
#include "agg_renderer_outline_image.h"
#include "agg_vpgen_clip_polyline.h"
#include "agg_arrowhead.h"

// boost
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>

// stl
#ifdef MAPNIK_DEBUG
#include <iostream>
#endif

#include <cmath>

namespace mapnik
{
   class pattern_source : private boost::noncopyable
   {
      public:
         pattern_source(ImageData32 const& pattern)
            : pattern_(pattern) {}

         unsigned int width() const
         {
            return pattern_.width();
         }
         unsigned int height() const
         {
            return pattern_.height();
         }
         agg::rgba8 pixel(int x, int y) const
         {
            unsigned c = pattern_(x,y);
            return agg::rgba8(c & 0xff,
                              (c >> 8) & 0xff,
                              (c >> 16) & 0xff,
                              (c >> 24) & 0xff);
         }
      private:
         ImageData32 const& pattern_;
   };

   struct rasterizer :  agg::rasterizer_scanline_aa<>, boost::noncopyable {};

   template <typename T>
   agg_renderer<T>::agg_renderer(Map const& m, T & pixmap, unsigned offset_x, unsigned offset_y)
      : feature_style_processor<agg_renderer>(m),
        pixmap_(pixmap),
        width_(pixmap_.width()),
        height_(pixmap_.height()),
        t_(m.getWidth(),m.getHeight(),m.getCurrentExtent(),offset_x,offset_y),
        font_engine_(),
        font_manager_(font_engine_),
        detector_(Envelope<double>(-m.buffer_size(), -m.buffer_size(), m.getWidth() + m.buffer_size() ,m.getHeight() + m.buffer_size())),
        ras_ptr(new rasterizer)
   {
      boost::optional<color> bg = m.background();
      if (bg) pixmap_.setBackground(*bg);
#ifdef MAPNIK_DEBUG
      std::clog << "scale=" << m.scale() << "\n";
#endif
   }

   template <typename T>
   agg_renderer<T>::~agg_renderer() {}

   template <typename T>
   void agg_renderer<T>::start_map_processing(Map const& map)
   {
#ifdef MAPNIK_DEBUG
      std::clog << "start map processing bbox="
                << map.getCurrentExtent() << "\n";
#endif
      ras_ptr->clip_box(0,0,width_,height_);
   }

   template <typename T>
   void agg_renderer<T>::end_map_processing(Map const& )
   {
#ifdef MAPNIK_DEBUG
      std::clog << "end map processing\n";
#endif
   }

   template <typename T>
   void agg_renderer<T>::start_layer_processing(Layer const& lay)
   {
#ifdef MAPNIK_DEBUG
      std::clog << "start layer processing : " << lay.name()  << "\n";
      std::clog << "datasource = " << lay.datasource().get() << "\n";
#endif
      if (lay.clear_label_cache())
      {
         detector_.clear();
      }
   }

   template <typename T>
   void agg_renderer<T>::end_layer_processing(Layer const&)
   {
#ifdef MAPNIK_DEBUG
      std::clog << "end layer processing\n";
#endif
   }

   template <typename T>
   void agg_renderer<T>::process(polygon_symbolizer const& sym,
                                 Feature const& feature,
                                 proj_transform const& prj_trans)
   {
      typedef  coord_transform2<CoordTransform,geometry2d> path_type;
      typedef agg::renderer_base<agg::pixfmt_rgba32_plain> ren_base;
      typedef agg::renderer_scanline_aa_solid<ren_base> renderer;

      color const& fill_ = sym.get_fill();
      agg::scanline_u8 sl;

      agg::rendering_buffer buf(pixmap_.raw_data(),width_,height_, width_ * 4);
      agg::pixfmt_rgba32_plain pixf(buf);

      ren_base renb(pixf);
      unsigned r=fill_.red();
      unsigned g=fill_.green();
      unsigned b=fill_.blue();
      unsigned a=fill_.alpha();
      renderer ren(renb);

      ras_ptr->reset();
      ras_ptr->gamma(agg::gamma_linear(0.0, sym.get_gamma()));

      for (unsigned i=0;i<feature.num_geometries();++i)
      {
         geometry2d const& geom=feature.get_geometry(i);
         if (geom.num_points() > 2)
         {
            path_type path(t_,geom,prj_trans);
            ras_ptr->add_path(path);
         }
      }
      ren.color(agg::rgba8(r, g, b, int(a * sym.get_opacity())));
      agg::render_scanlines(*ras_ptr, sl, ren);
   }

   typedef boost::tuple<double,double,double,double> segment_t;
   bool y_order(segment_t const& first,segment_t const& second)
   {
      double miny0 = std::min(first.get<1>(),first.get<3>());
      double miny1 = std::min(second.get<1>(),second.get<3>());
      return  miny0 > miny1;
   }

   template <typename T>
   void agg_renderer<T>::process(building_symbolizer const& sym,
                                 Feature const& feature,
                                 proj_transform const& prj_trans)
   {
      typedef  coord_transform2<CoordTransform,geometry2d> path_type;
      typedef  coord_transform3<CoordTransform,geometry2d> path_type_roof;
      typedef agg::renderer_base<agg::pixfmt_rgba32_plain> ren_base;
      typedef agg::renderer_scanline_aa_solid<ren_base> renderer;

      agg::rendering_buffer buf(pixmap_.raw_data(),width_,height_, width_ * 4);
      agg::pixfmt_rgba32_plain pixf(buf);
      ren_base renb(pixf);

      color const& fill_  = sym.get_fill();
      unsigned r=fill_.red();
      unsigned g=fill_.green();
      unsigned b=fill_.blue();
      unsigned a=fill_.alpha();
      renderer ren(renb);
      agg::scanline_u8 sl;

      ras_ptr->reset();      
      ras_ptr->gamma(agg::gamma_linear());
      
      double height = 0.7071 * sym.height(); // height in meters

      for (unsigned i=0;i<feature.num_geometries();++i)
      {
         geometry2d const& geom = feature.get_geometry(i);
         if (geom.num_points() > 2)
         {
            boost::scoped_ptr<geometry2d> frame(new line_string_impl);
            boost::scoped_ptr<geometry2d> roof(new polygon_impl);
            std::deque<segment_t> face_segments;
            double x0(0);
            double y0(0);
            unsigned cm = geom.vertex(&x0,&y0);
            for (unsigned j=1;j<geom.num_points();++j)
            {
               double x,y;
               cm = geom.vertex(&x,&y);
               if (cm == SEG_MOVETO)
               {
                  frame->move_to(x,y);
               }
               else if (cm == SEG_LINETO)
               {
                  frame->line_to(x,y);
               }
               if (j!=0)
               {
                  face_segments.push_back(segment_t(x0,y0,x,y));
               }
               x0 = x;
               y0 = y;
            }
            std::sort(face_segments.begin(),face_segments.end(), y_order);
            std::deque<segment_t>::const_iterator itr=face_segments.begin();
            for (;itr!=face_segments.end();++itr)
            {
               boost::scoped_ptr<geometry2d> faces(new polygon_impl);
               faces->move_to(itr->get<0>(),itr->get<1>());
               faces->line_to(itr->get<2>(),itr->get<3>());
               faces->line_to(itr->get<2>(),itr->get<3>() + height);
               faces->line_to(itr->get<0>(),itr->get<1>() + height);

               path_type faces_path (t_,*faces,prj_trans);
               ras_ptr->add_path(faces_path);
               ren.color(agg::rgba8(int(r*0.8), int(g*0.8), int(b*0.8), int(a * sym.get_opacity())));
               agg::render_scanlines(*ras_ptr, sl, ren);
               ras_ptr->reset();

               frame->move_to(itr->get<0>(),itr->get<1>());
               frame->line_to(itr->get<0>(),itr->get<1>()+height);
            }

            geom.rewind(0);
            for (unsigned j=0;j<geom.num_points();++j)
            {
               double x,y;
               unsigned cm = geom.vertex(&x,&y);
               if (cm == SEG_MOVETO)
               {
                  frame->move_to(x,y+height);
                  roof->move_to(x,y+height);
               }
               else if (cm == SEG_LINETO)
               {
                  frame->line_to(x,y+height);
                  roof->line_to(x,y+height);
               }
            }
            path_type path(t_,*frame,prj_trans);
            agg::conv_stroke<path_type>  stroke(path);
            ras_ptr->add_path(stroke);
            ren.color(agg::rgba8(128, 128, 128, int(255 * sym.get_opacity())));
            agg::render_scanlines(*ras_ptr, sl, ren);
            ras_ptr->reset();

            path_type roof_path (t_,*roof,prj_trans);
            ras_ptr->add_path(roof_path);
            ren.color(agg::rgba8(r, g, b, int(a * sym.get_opacity())));
            agg::render_scanlines(*ras_ptr, sl, ren);
         }
      }
   }

   template <typename T>
   void agg_renderer<T>::process(line_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
   {
      typedef agg::renderer_base<agg::pixfmt_rgba32_plain> ren_base;
      typedef coord_transform2<CoordTransform,geometry2d> path_type;
      typedef agg::renderer_outline_aa<ren_base> renderer_oaa;
      typedef agg::rasterizer_outline_aa<renderer_oaa> rasterizer_outline_aa;
      typedef agg::renderer_scanline_aa_solid<ren_base> renderer;

      agg::rendering_buffer buf(pixmap_.raw_data(),width_,height_, width_ * 4);
      agg::pixfmt_rgba32_plain pixf(buf);

      ren_base renb(pixf);
      mapnik::stroke const&  stroke_ = sym.get_stroke();
      color const& col = stroke_.get_color();
      unsigned r=col.red();
      unsigned g=col.green();
      unsigned b=col.blue();
      unsigned a=col.alpha();
      renderer ren(renb);
      ras_ptr->reset();
      ras_ptr->gamma(agg::gamma_linear());
      agg::scanline_p8 sl;

      for (unsigned i=0;i<feature.num_geometries();++i)
      {
         geometry2d const& geom = feature.get_geometry(i);
         if (geom.num_points() > 1)
         {
            path_type path(t_,geom,prj_trans);

            if (stroke_.has_dash())
            {
               agg::conv_dash<path_type> dash(path);
               dash_array const& d = stroke_.get_dash_array();
               dash_array::const_iterator itr = d.begin();
               dash_array::const_iterator end = d.end();
               for (;itr != end;++itr)
               {
                  dash.add_dash(itr->first, itr->second);
               }

               agg::conv_stroke<agg::conv_dash<path_type > > stroke(dash);

               line_join_e join=stroke_.get_line_join();
               if ( join == MITER_JOIN)
                  stroke.generator().line_join(agg::miter_join);
               else if( join == MITER_REVERT_JOIN)
                  stroke.generator().line_join(agg::miter_join);
               else if( join == ROUND_JOIN)
                  stroke.generator().line_join(agg::round_join);
               else
                  stroke.generator().line_join(agg::bevel_join);

               line_cap_e cap=stroke_.get_line_cap();
               if (cap == BUTT_CAP)
                  stroke.generator().line_cap(agg::butt_cap);
               else if (cap == SQUARE_CAP)
                  stroke.generator().line_cap(agg::square_cap);
               else
                  stroke.generator().line_cap(agg::round_cap);

               stroke.generator().miter_limit(4.0);
               stroke.generator().width(stroke_.get_width());

               ras_ptr->add_path(stroke);

            }
            else
            {
               agg::conv_stroke<path_type>  stroke(path);
               line_join_e join=stroke_.get_line_join();
               if ( join == MITER_JOIN)
                  stroke.generator().line_join(agg::miter_join);
               else if( join == MITER_REVERT_JOIN)
                  stroke.generator().line_join(agg::miter_join);
               else if( join == ROUND_JOIN)
                  stroke.generator().line_join(agg::round_join);
               else
                  stroke.generator().line_join(agg::bevel_join);

               line_cap_e cap=stroke_.get_line_cap();
               if (cap == BUTT_CAP)
                  stroke.generator().line_cap(agg::butt_cap);
               else if (cap == SQUARE_CAP)
                  stroke.generator().line_cap(agg::square_cap);
               else
                  stroke.generator().line_cap(agg::round_cap);

               stroke.generator().miter_limit(4.0);
               stroke.generator().width(stroke_.get_width());
               ras_ptr->add_path(stroke);
            }
         }
      }
      ren.color(agg::rgba8(r, g, b, int(a*stroke_.get_opacity())));
      agg::render_scanlines(*ras_ptr, sl, ren);
   }

   template <typename T>
   void agg_renderer<T>::process(point_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
   {
      double x;
      double y;
      double z=0;
      boost::shared_ptr<ImageData32> const& data = sym.get_image();
      if ( data )
      {
         for (unsigned i=0;i<feature.num_geometries();++i)
         {
            geometry2d const& geom = feature.get_geometry(i);

            geom.label_position(&x,&y);
            prj_trans.backward(x,y,z);
            t_.forward(&x,&y);
            int w = data->width();
            int h = data->height();
            int px=int(floor(x - 0.5 * w));
            int py=int(floor(y - 0.5 * h));
            Envelope<double> label_ext (floor(x - 0.5 * w),
                                        floor(y - 0.5 * h),
                                        ceil (x + 0.5 * w),
                                        ceil (y + 0.5 * h));
            if (sym.get_allow_overlap() ||
                detector_.has_placement(label_ext))
            {
               pixmap_.set_rectangle_alpha2(*data,px,py,sym.get_opacity());
               detector_.insert(label_ext);
            }
         }
      }
   }

   template <typename T>
   void  agg_renderer<T>::process(shield_symbolizer const& sym,
                                  Feature const& feature,
                                  proj_transform const& prj_trans)
   {
      typedef  coord_transform2<CoordTransform,geometry2d> path_type;

      UnicodeString text;
      if( sym.get_no_text() )
            text = UnicodeString( " " );  // use 'space' as the text to render
      else
            text = feature[sym.get_name()].to_unicode();  // use text from feature to render

      if ( sym.get_text_convert() == TOUPPER)
      {
         text = text.toUpper();
      }
      else if ( sym.get_text_convert() == TOLOWER)
      {
         text = text.toLower();
      }
      boost::shared_ptr<ImageData32> const& data = sym.get_image();
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

         if (faces->size() > 0)
         {
            text_renderer<T> ren(pixmap_, faces);

            ren.set_pixel_size(sym.get_text_size());
            ren.set_fill(sym.get_fill());
            ren.set_halo_fill(sym.get_halo_fill());
            ren.set_halo_radius(sym.get_halo_radius());

            placement_finder<label_collision_detector4> finder(detector_);

            string_info info(text);

            faces->get_string_info(info);


            int w = data->width();
            int h = data->height();

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
                        placement text_placement(info, sym, false);
                        text_placement.avoid_edges = sym.get_avoid_edges();
                        text_placement.allow_overlap = sym.get_allow_overlap();
                        if( how_placed == VERTEX_PLACEMENT )
                            geom.vertex(&label_x,&label_y);  // by vertex
                        else
                            geom.label_position(&label_x, &label_y);  // by middle of line or by point
                        prj_trans.backward(label_x,label_y, z);
                        t_.forward(&label_x,&label_y);

                        finder.find_point_placement( text_placement,label_x,label_y,sym.get_vertical_alignment(),sym.get_line_spacing(),
                                                    sym.get_character_spacing(),sym.get_horizontal_alignment(),sym.get_justify_alignment() );

                        // check to see if image overlaps anything too, there is only ever 1 placement found for points and verticies
                        if( text_placement.placements.size() > 0)
                        {
                            double x = text_placement.placements[0].starting_x;
                            double y = text_placement.placements[0].starting_y;
                            int px;
                            int py;
                            Envelope<double> label_ext;

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
                                pixmap_.set_rectangle_alpha2(*data,px,py,float(sym.get_opacity()));
                                Envelope<double> dim = ren.prepare_glyphs(&text_placement.placements[0]);
                                ren.render(x,y);
                                detector_.insert(label_ext);
                                finder.update_detector(text_placement);
                            }
                        }
                     }
                  }

                  else if (geom.num_points() > 1 && sym.get_label_placement() == LINE_PLACEMENT)
                  {
                     placement text_placement(info, sym, true);
                     text_placement.avoid_edges = sym.get_avoid_edges();
                     finder.find_point_placements<path_type>(text_placement,path);

                     for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ ii)
                     {
                        int w = data->width();
                        int h = data->height();
                        double x = text_placement.placements[ii].starting_x;
                        double y = text_placement.placements[ii].starting_y;

                        int px=int(x - (w/2));
                        int py=int(y - (h/2));

                        pixmap_.set_rectangle_alpha(px,py,*data);

                        Envelope<double> dim = ren.prepare_glyphs(&text_placement.placements[ii]);
                        ren.render(x,y);
                     }
                     finder.update_detector(text_placement);
                  }
               }
            }
         }
      }
   }

   template <typename T>
   void  agg_renderer<T>::process(line_pattern_symbolizer const& sym,
                               Feature const& feature,
                               proj_transform const& prj_trans)
   {
      typedef  coord_transform2<CoordTransform,geometry2d> path_type;
      typedef agg::line_image_pattern<agg::pattern_filter_bilinear_rgba8> pattern_type;
      typedef agg::renderer_base<agg::pixfmt_rgba32_plain> renderer_base;
      typedef agg::renderer_outline_image<renderer_base, pattern_type> renderer_type;
      typedef agg::rasterizer_outline_aa<renderer_type> rasterizer_type;

      agg::rendering_buffer buf(pixmap_.raw_data(),width_,height_, width_ * 4);
      agg::pixfmt_rgba32_plain pixf(buf);

      ImageData32 pat =  * sym.get_image();
      renderer_base ren_base(pixf);
      agg::pattern_filter_bilinear_rgba8 filter;
      pattern_source source(pat);
      pattern_type pattern (filter,source);
      renderer_type ren(ren_base, pattern);
      ren.clip_box(0,0,width_,height_);
      rasterizer_type ras(ren);

      for (unsigned i=0;i<feature.num_geometries();++i)
      {
         geometry2d const& geom = feature.get_geometry(i);
         if (geom.num_points() > 1)
         {
            path_type path(t_,geom,prj_trans);
            ras.add_path(path);
         }
      }
   }

   template <typename T>
   void agg_renderer<T>::process(polygon_pattern_symbolizer const& sym,
                                 Feature const& feature,
                                 proj_transform const& prj_trans)
   {
      typedef coord_transform2<CoordTransform,geometry2d> path_type;
      typedef agg::renderer_base<agg::pixfmt_rgba32_plain> ren_base;
      typedef agg::wrap_mode_repeat wrap_x_type;
      typedef agg::wrap_mode_repeat wrap_y_type;
      typedef agg::pixfmt_alpha_blend_rgba<agg::blender_rgba32,
         agg::row_accessor<agg::int8u>, agg::pixel32_type> rendering_buffer;
      typedef agg::image_accessor_wrap<rendering_buffer,
         wrap_x_type,
         wrap_y_type> img_source_type;

      typedef agg::span_pattern_rgba<img_source_type> span_gen_type;

      typedef agg::renderer_scanline_aa<ren_base,
         agg::span_allocator<agg::rgba8>,
         span_gen_type> renderer_type;


      agg::rendering_buffer buf(pixmap_.raw_data(),width_,height_, width_ * 4);
      agg::pixfmt_rgba32_plain pixf(buf);
      ren_base renb(pixf);

      agg::scanline_u8 sl;
      ras_ptr->reset();
      ras_ptr->gamma(agg::gamma_linear());

      ImageData32 const& pattern =  * sym.get_image();
      unsigned w=pattern.width();
      unsigned h=pattern.height();
      agg::row_accessor<agg::int8u> pattern_rbuf((agg::int8u*)pattern.getBytes(),w,h,w*4);
      agg::span_allocator<agg::rgba8> sa;
      agg::pixfmt_alpha_blend_rgba<agg::blender_rgba32,
         agg::row_accessor<agg::int8u>, agg::pixel32_type> pixf_pattern(pattern_rbuf);
      img_source_type img_src(pixf_pattern);

      double x0=0,y0=0;
      unsigned num_geometries = feature.num_geometries();
      if (num_geometries>0)
      {
         path_type path(t_,feature.get_geometry(0),prj_trans);
         path.vertex(&x0,&y0);
      }
      unsigned offset_x = unsigned(width_-x0);
      unsigned offset_y = unsigned(height_-y0);
      span_gen_type sg(img_src, offset_x, offset_y);
      renderer_type rp(renb,sa, sg);
      for (unsigned i=0;i<num_geometries;++i)
      {
         geometry2d const& geom = feature.get_geometry(i);
         if (geom.num_points() > 2)
         {
            path_type path(t_,geom,prj_trans);
            ras_ptr->add_path(path);
         }
      }
      agg::render_scanlines(*ras_ptr, sl, rp);
   }

   template <typename T>
   void agg_renderer<T>::process(raster_symbolizer const& sym,
                                 Feature const& feature,
                                 proj_transform const& prj_trans)
   {
      raster_ptr const& raster=feature.get_raster();
      if (raster)
      {
         Envelope<double> ext=t_.forward(raster->ext_);
         int start_x = rint(ext.minx());
         int start_y = rint(ext.miny());
         int raster_width = rint(ext.width());
         int raster_height = rint(ext.height());
         int end_x = start_x + raster_width;
         int end_y = start_y + raster_height;
         double err_offs_x = (ext.minx()-start_x + ext.maxx()-end_x)/2;
         double err_offs_y = (ext.miny()-start_y + ext.maxy()-end_y)/2;
         
         if (raster_width > 0 && raster_height > 0)
         {
            ImageData32 target(raster_width,raster_height);

            if (sym.get_scaling() == "fast") {
               scale_image<ImageData32>(target,raster->data_);
            } else if (sym.get_scaling() == "bilinear"){
               scale_image_bilinear<ImageData32>(target,raster->data_, err_offs_x, err_offs_y);
            } else if (sym.get_scaling() == "bilinear8"){
               scale_image_bilinear8<ImageData32>(target,raster->data_, err_offs_x, err_offs_y);
            } else {
               scale_image<ImageData32>(target,raster->data_);
            }

            if (sym.get_mode() == "normal"){
                if (sym.get_opacity() == 1.0) {
                   pixmap_.set_rectangle(start_x,start_y,target);
                } else {
                   pixmap_.set_rectangle_alpha2(target,start_x,start_y, sym.get_opacity());
                }
            } else if (sym.get_mode() == "grain_merge"){
               pixmap_.template merge_rectangle<MergeGrain> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "grain_merge2"){
               pixmap_.template merge_rectangle<MergeGrain2> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "multiply"){
               pixmap_.template merge_rectangle<Multiply> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "multiply2"){
               pixmap_.template merge_rectangle<Multiply2> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "divide"){
               pixmap_.template merge_rectangle<Divide> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "divide2"){
               pixmap_.template merge_rectangle<Divide2> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "screen"){
               pixmap_.template merge_rectangle<Screen> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "hard_light"){
               pixmap_.template merge_rectangle<HardLight> (target,start_x,start_y, sym.get_opacity());
            } else {
                if (sym.get_opacity() == 1.0){
                    pixmap_.set_rectangle(start_x,start_y,target);
                } else {
                   pixmap_.set_rectangle_alpha2(target,start_x,start_y, sym.get_opacity());
                }
            }
            // TODO: other modes? (add,diff,sub,...)
         }
      }
   }

   template <typename T>
   void agg_renderer<T>::process(markers_symbolizer const& sym,
                                 Feature const& feature,
                                 proj_transform const& prj_trans)
   {
      typedef  coord_transform2<CoordTransform,geometry2d> path_type;
      typedef agg::renderer_base<agg::pixfmt_rgba32_plain> ren_base;
      typedef agg::renderer_scanline_aa_solid<ren_base> renderer;
      arrow arrow_;
      ras_ptr->reset();
      ras_ptr->gamma(agg::gamma_linear());

      agg::scanline_u8 sl;
      agg::rendering_buffer buf(pixmap_.raw_data(),width_,height_, width_ * 4);
      agg::pixfmt_rgba32_plain pixf(buf);
      ren_base renb(pixf);

      unsigned r = 0;// fill_.red();
      unsigned g = 0; //fill_.green();
      unsigned b = 255; //fill_.blue();
      unsigned a = 255; //fill_.alpha();
      renderer ren(renb);
      for (unsigned i=0;i<feature.num_geometries();++i)
      {
         geometry2d const& geom=feature.get_geometry(i);
         if (geom.num_points() > 1)
         {
            path_type path(t_,geom,prj_trans);

            agg::conv_dash <path_type> dash(path);
            dash.add_dash(20.0,200.0);
            markers_converter<agg::conv_dash<path_type>,
               arrow,
               label_collision_detector4>
               marker(dash, arrow_, detector_);
            ras_ptr->add_path(marker);
         }
      }
      ren.color(agg::rgba8(r, g, b, a));
      agg::render_scanlines(*ras_ptr, sl, ren);
   }

   template <typename T>
   void agg_renderer<T>::process(text_symbolizer const& sym,
                                 Feature const& feature,
                                 proj_transform const& prj_trans)
   {
      typedef  coord_transform2<CoordTransform,geometry2d> path_type;

      UnicodeString text = feature[sym.get_name()].to_unicode();
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

         if (faces->size() > 0)
         {
            text_renderer<T> ren(pixmap_, faces);
            ren.set_pixel_size(sym.get_text_size());
            ren.set_fill(fill);
            ren.set_halo_fill(sym.get_halo_fill());
            ren.set_halo_radius(sym.get_halo_radius());
            ren.set_opacity(sym.get_opacity());

            placement_finder<label_collision_detector4> finder(detector_);

            string_info info(text);

            faces->get_string_info(info);
            unsigned num_geom = feature.num_geometries();
            for (unsigned i=0;i<num_geom;++i)
            {
               geometry2d const& geom = feature.get_geometry(i);
               if (geom.num_points() > 0) // don't bother with empty geometries
               {
                  path_type path(t_,geom,prj_trans);
                  placement text_placement(info,sym);
                  text_placement.avoid_edges = sym.get_avoid_edges();
                  if (sym.get_label_placement() == POINT_PLACEMENT)
                  {
                     double label_x, label_y, z=0.0;
                     geom.label_position(&label_x, &label_y);
                     prj_trans.backward(label_x,label_y, z);
                     t_.forward(&label_x,&label_y);
                     finder.find_point_placement(text_placement,label_x,label_y,sym.get_vertical_alignment(),sym.get_line_spacing(),
                                                 sym.get_character_spacing(),sym.get_horizontal_alignment(),sym.get_justify_alignment());
                     finder.update_detector(text_placement);
                  }
                  else if ( geom.num_points() > 1 && sym.get_label_placement() == LINE_PLACEMENT)
                  {
                     finder.find_line_placements<path_type>(text_placement,path);
                  }

                  for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ii)
                  {
                     double x = text_placement.placements[ii].starting_x;
                     double y = text_placement.placements[ii].starting_y;
                     Envelope<double> dim = ren.prepare_glyphs(&text_placement.placements[ii]);
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
   template class agg_renderer<Image32>;
}
