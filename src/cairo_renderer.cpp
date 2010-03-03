/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2008 Tom Hughes
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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/cairo_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/markers_converter.hpp>
#include <mapnik/arrow.hpp>
#include <mapnik/config_error.hpp>

// cairo
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <cairo-ft.h>

// boost
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>

// stl
#ifdef MAPNIK_DEBUG
#include <iostream>
#endif

namespace mapnik
{
   class cairo_pattern : private boost::noncopyable
   {
      public:
         cairo_pattern(ImageData32 const& data)
         {
            int pixels = data.width() * data.height();
            const unsigned int *in_ptr = data.getData();
            const unsigned int *in_end = in_ptr + pixels;
            unsigned int *out_ptr;

            out_ptr = data_ = new unsigned int[pixels];

            while (in_ptr < in_end)
            {
               unsigned int in = *in_ptr++;
               unsigned int r = (in >> 0) & 0xff;
               unsigned int g = (in >> 8) & 0xff;
               unsigned int b = (in >> 16) & 0xff;
               unsigned int a = (in >> 24) & 0xff;

               r = r * a / 255;
               g = g * a / 255;
               b = b * a / 255;

               *out_ptr++ = (a << 24) | (r << 16) | (g << 8) | b;
            }

            surface_ = Cairo::ImageSurface::create(reinterpret_cast<unsigned char *>(data_), Cairo::FORMAT_ARGB32, data.width(), data.height(), data.width() * 4);
            pattern_ = Cairo::SurfacePattern::create(surface_);
         }

         ~cairo_pattern(void)
         {
            delete [] data_;
         }
         
         void set_matrix(Cairo::Matrix const& matrix)
         {
            pattern_->set_matrix(matrix);
         }

         void set_origin(double x, double y)
         {
            Cairo::Matrix matrix;
            
            pattern_->get_matrix(matrix);
     
            matrix.x0 = -x;
            matrix.y0 = -y;
            
            pattern_->set_matrix(matrix);
         }
         
         void set_extend(Cairo::Extend extend)
         {
            pattern_->set_extend(extend);
         }

         void set_filter(Cairo::Filter filter)
         {
            pattern_->set_filter(filter);
         }

         Cairo::RefPtr<Cairo::SurfacePattern> const& pattern(void) const
         {
            return pattern_;
         }

      private:
         unsigned int *data_;
         Cairo::RefPtr<Cairo::ImageSurface> surface_;
         Cairo::RefPtr<Cairo::SurfacePattern> pattern_;
   };

   class cairo_face : private boost::noncopyable
   {
      public:
         cairo_face(boost::shared_ptr<freetype_engine> const& engine, face_ptr const& face)
            : face_(face)
         {
            static cairo_user_data_key_t key;
            cairo_font_face_t *c_face;

            c_face = cairo_ft_font_face_create_for_ft_face(face->get_face(), FT_LOAD_NO_HINTING);
            cairo_font_face_set_user_data(c_face, &key, new handle(engine, face), destroy);

            cairo_face_ = Cairo::RefPtr<Cairo::FontFace>(new Cairo::FontFace(c_face));
         }

         Cairo::RefPtr<Cairo::FontFace> const& face(void) const
         {
            return cairo_face_;
         }

      private:
         class handle
         {
            public:
               handle(boost::shared_ptr<freetype_engine> const& engine, face_ptr const& face)
                  : engine_(engine), face_(face) {}

            private:
               boost::shared_ptr<freetype_engine> engine_;
               face_ptr face_;
         };

         static void destroy(void *data)
         {
            handle *h = static_cast<handle *>(data);

            delete h;
         }

      private:
         face_ptr face_;
         Cairo::RefPtr<Cairo::FontFace> cairo_face_;
   };

   cairo_face_manager::cairo_face_manager(boost::shared_ptr<freetype_engine> engine,
                                          face_manager<freetype_engine> & manager)
      : font_engine_(engine),
        font_manager_(manager)
   {
   }

   cairo_face_ptr cairo_face_manager::get_face(face_ptr face)
   {
      cairo_face_cache::iterator itr = cache_.find(face);
      cairo_face_ptr entry;

      if (itr != cache_.end())
      {
         entry = itr->second;
      }
      else
      {
         entry = cairo_face_ptr(new cairo_face(font_engine_, face));

         cache_.insert(std::make_pair(face, entry));
      }

      return entry;
   }

   class cairo_context : private boost::noncopyable
   {
      public:
         cairo_context(Cairo::RefPtr<Cairo::Context> const& context)
            : context_(context)
         {
            context_->save();
         }

         ~cairo_context(void)
         {
            context_->restore();
         }

         void set_color(color const &color, double opacity = 1.0)
         {
            set_color(color.red(), color.green(), color.blue(), color.alpha() * opacity / 255.0);
         }

         void set_color(int r, int g, int b, double opacity = 1.0)
         {
            context_->set_source_rgba(r / 255.0, g / 255.0, b / 255.0, opacity);
         }

         void set_line_join(line_join_e join)
         {
            if (join == MITER_JOIN)
               context_->set_line_join(Cairo::LINE_JOIN_MITER);
            else if (join == MITER_REVERT_JOIN)
               context_->set_line_join(Cairo::LINE_JOIN_MITER);
            else if (join == ROUND_JOIN)
               context_->set_line_join(Cairo::LINE_JOIN_ROUND);
            else
               context_->set_line_join(Cairo::LINE_JOIN_BEVEL);
         }

         void set_line_cap(line_cap_e cap)
         {
            if (cap == BUTT_CAP)
               context_->set_line_cap(Cairo::LINE_CAP_BUTT);
            else if (cap == SQUARE_CAP)
               context_->set_line_cap(Cairo::LINE_CAP_SQUARE);
            else
               context_->set_line_cap(Cairo::LINE_CAP_ROUND);
         }

         void set_miter_limit(double limit)
         {
            context_->set_miter_limit(limit);
         }

         void set_line_width(double width)
         {
            context_->set_line_width(width);
         }

         void set_dash(dash_array const &dashes)
         {
            std::valarray<double> d(dashes.size() * 2);
            dash_array::const_iterator itr = dashes.begin();
            dash_array::const_iterator end = dashes.end();
            int index = 0;

            for (; itr != end; ++itr)
            {
               d[index++] = itr->first;
               d[index++] = itr->second;
            }

            context_->set_dash(d, 0.0);
         }

         void move_to(double x, double y)
         {
#if CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 6, 0)
            if (x < -32767.0) x = -32767.0;
            else if (x > 32767.0) x = 32767.0;
            if (y < -32767.0) y = -32767.0;
            else if (y > 32767.0) y = 32767.0;
#endif

            context_->move_to(x, y);
         }


         void line_to(double x, double y)
         {
#if CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 6, 0)
            if (x < -32767.0) x = -32767.0;
            else if (x > 32767.0) x = 32767.0;
            if (y < -32767.0) y = -32767.0;
            else if (y > 32767.0) y = 32767.0;
#endif

            context_->line_to(x, y);
         }

         template <typename T>
         void add_path(T path)
         {
            double x, y;

            path.rewind(0);

            for (unsigned cm = path.vertex(&x, &y); cm != SEG_END; cm = path.vertex(&x, &y))
            {
               if (cm == SEG_MOVETO)
               {
                  move_to(x, y);
               }
               else if (cm == SEG_LINETO)
               {
                  line_to(x, y);
               }
            }
         }

         void rectangle(double x, double y, double w, double h)
         {
            context_->rectangle(x, y, w, h);
         }

         void stroke(void)
         {
            context_->stroke();
         }

         void fill(void)
         {
            context_->fill();
         }

         void paint(void)
         {
            context_->paint();
         }

         void set_pattern(cairo_pattern const& pattern)
         {
            context_->set_source(pattern.pattern());
         }

         void add_image(double x, double y, ImageData32 & data, double opacity = 1.0)
         {
            cairo_pattern pattern(data);

            pattern.set_origin(x, y);

            context_->save();
            context_->set_source(pattern.pattern());
            context_->paint_with_alpha(opacity);
            context_->restore();
         }

         void set_font_face(cairo_face_manager & manager, face_ptr face)
         {
            context_->set_font_face(manager.get_face(face)->face());
         }

         void set_font_matrix(Cairo::Matrix const& matrix)
         {
            context_->set_font_matrix(matrix);
         }

         void show_glyph(unsigned long index, double x, double y)
         {
            Cairo::Glyph glyph;

            glyph.index = index;
            glyph.x = x;
            glyph.y = y;

            std::vector<Cairo::Glyph> glyphs;

            glyphs.push_back(glyph);

            context_->show_glyphs(glyphs);
         }

         void glyph_path(unsigned long index, double x, double y)
         {
            Cairo::Glyph glyph;

            glyph.index = index;
            glyph.x = x;
            glyph.y = y;

            std::vector<Cairo::Glyph> glyphs;

            glyphs.push_back(glyph);

            context_->glyph_path(glyphs);
         }

         void add_text(text_symbolizer const& sym, text_path & path,
                       cairo_face_manager & manager,
                       face_set_ptr const& faces)
         {
            unsigned text_size = sym.get_text_size();
            double sx = path.starting_x;
            double sy = path.starting_y;

            path.rewind();

            for (int iii = 0; iii < path.num_nodes(); iii++)
            {
               int c;
               double x, y, angle;

               path.vertex(&c, &x, &y, &angle);

               glyph_ptr glyph = faces->get_glyph(c);
 
               if (glyph)
               {
                  Cairo::Matrix matrix;

                  matrix.xx = text_size * cos(angle);
                  matrix.xy = text_size * sin(angle);
                  matrix.yx = text_size * -sin(angle);
                  matrix.yy = text_size * cos(angle);
                  matrix.x0 = 0;
                  matrix.y0 = 0;

                  set_font_matrix(matrix);

                  set_font_face(manager, glyph->get_face());

                  glyph_path(glyph->get_index(), sx + x, sy - y);
               }
            }

            set_line_width(sym.get_halo_radius());
            set_line_join(ROUND_JOIN);
            set_color(sym.get_halo_fill());
            stroke();

            set_color(sym.get_fill());

            path.rewind();

            for (int iii = 0; iii < path.num_nodes(); iii++)
            {
               int c;
               double x, y, angle;

               path.vertex(&c, &x, &y, &angle);

               glyph_ptr glyph = faces->get_glyph(c);
 
               if (glyph)
               {
                  Cairo::Matrix matrix;

                  matrix.xx = text_size * cos(angle);
                  matrix.xy = text_size * sin(angle);
                  matrix.yx = text_size * -sin(angle);
                  matrix.yy = text_size * cos(angle);
                  matrix.x0 = 0;
                  matrix.y0 = 0;

                  set_font_matrix(matrix);

                  set_font_face(manager, glyph->get_face());

                  show_glyph(glyph->get_index(), sx + x, sy - y);
               }
            }
         }


      private:
         Cairo::RefPtr<Cairo::Context> context_;
   };

   cairo_renderer_base::cairo_renderer_base(Map const& m, Cairo::RefPtr<Cairo::Context> const& context, unsigned offset_x, unsigned offset_y)
      : m_(m),
        context_(context),
        t_(m.getWidth(),m.getHeight(),m.getCurrentExtent(),offset_x,offset_y),
        font_engine_(new freetype_engine()),
        font_manager_(*font_engine_),
        face_manager_(font_engine_,font_manager_),
        detector_(Envelope<double>(-m.buffer_size() ,-m.buffer_size() , m.getWidth() + m.buffer_size() ,m.getHeight() + m.buffer_size()))
   {
#ifdef MAPNIK_DEBUG
      std::clog << "scale=" << m.scale() << "\n";
#endif
   }

   template <>
   cairo_renderer<Cairo::Context>::cairo_renderer(Map const& m, Cairo::RefPtr<Cairo::Context> const& context, unsigned offset_x, unsigned offset_y)
      : feature_style_processor<cairo_renderer>(m),
        cairo_renderer_base(m,context,offset_x,offset_y)
   {
   }

   template <>
   cairo_renderer<Cairo::Surface>::cairo_renderer(Map const& m, Cairo::RefPtr<Cairo::Surface> const& surface, unsigned offset_x, unsigned offset_y)
      : feature_style_processor<cairo_renderer>(m),
        cairo_renderer_base(m,Cairo::Context::create(surface),offset_x,offset_y)
   {
   }

   cairo_renderer_base::~cairo_renderer_base() {}

   void cairo_renderer_base::start_map_processing(Map const& map)
   {
#ifdef MAPNIK_DEBUG
      std::clog << "start map processing bbox="
                << map.getCurrentExtent() << "\n";
#endif

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 6, 0)
      Envelope<double> bounds = t_.forward(t_.extent());
      context_->rectangle(bounds.minx(), bounds.miny(), bounds.maxx(), bounds.maxy());
      context_->clip();
#endif

      boost::optional<color> bg = m_.background();
      if (bg)
      {
         cairo_context context(context_);
         context.set_color(*bg);
         context.paint();
      }
   }

   template <>
   void cairo_renderer<Cairo::Context>::end_map_processing(Map const& )
   {
#ifdef MAPNIK_DEBUG
      std::clog << "end map processing\n";
#endif
   }

   template <>
   void cairo_renderer<Cairo::Surface>::end_map_processing(Map const& )
   {
#ifdef MAPNIK_DEBUG
      std::clog << "end map processing\n";
#endif
      context_->show_page();
   }

   void cairo_renderer_base::start_layer_processing(Layer const& lay)
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

   void cairo_renderer_base::end_layer_processing(Layer const&)
   {
#ifdef MAPNIK_DEBUG
      std::clog << "end layer processing\n";
#endif
   }

   void cairo_renderer_base::process(polygon_symbolizer const& sym,
                                     Feature const& feature,
                                     proj_transform const& prj_trans)
   {
      typedef coord_transform2<CoordTransform,geometry2d> path_type;

      cairo_context context(context_);

      context.set_color(sym.get_fill(), sym.get_opacity());

      for (unsigned i = 0; i < feature.num_geometries(); ++i)
      {
         geometry2d const& geom = feature.get_geometry(i);

         if (geom.num_points() > 2)
         {
            path_type path(t_, geom, prj_trans);

            context.add_path(path);
            context.fill();
         }
      }
   }

   typedef boost::tuple<double,double,double,double> segment_t;
   bool cairo_y_order(segment_t const& first,segment_t const& second)
   {
      double miny0 = std::min(first.get<1>(), first.get<3>());
      double miny1 = std::min(second.get<1>(), second.get<3>());
      return miny0 > miny1;
   }

   void cairo_renderer_base::process(building_symbolizer const& sym,
                                     Feature const& feature,
                                     proj_transform const& prj_trans)
   {
      typedef coord_transform2<CoordTransform,geometry2d> path_type;
      typedef coord_transform3<CoordTransform,geometry2d> path_type_roof;

      cairo_context context(context_);

      color const& fill = sym.get_fill();
      double height = 0.7071 * sym.height(); // height in meters

      for (unsigned i = 0; i < feature.num_geometries(); ++i)
      {
         geometry2d const& geom = feature.get_geometry(i);

         if (geom.num_points() > 2)
         {
            boost::scoped_ptr<geometry2d> frame(new line_string_impl);
            boost::scoped_ptr<geometry2d> roof(new polygon_impl);
            std::deque<segment_t> face_segments;
            double x0(0);
            double y0(0);
            unsigned cm = geom.vertex(&x0, &y0);

            for (unsigned j = 1; j < geom.num_points(); ++j)
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

               if (j != 0)
               {
                  face_segments.push_back(segment_t(x0, y0, x, y));
               }

               x0 = x;
               y0 = y;
            }

            std::sort(face_segments.begin(), face_segments.end(), cairo_y_order);
            std::deque<segment_t>::const_iterator itr = face_segments.begin();
            for (; itr != face_segments.end(); ++itr)
            {
               boost::scoped_ptr<geometry2d> faces(new polygon_impl);

               faces->move_to(itr->get<0>(), itr->get<1>());
               faces->line_to(itr->get<2>(), itr->get<3>());
               faces->line_to(itr->get<2>(), itr->get<3>() + height);
               faces->line_to(itr->get<0>(), itr->get<1>() + height);

               path_type faces_path(t_, *faces, prj_trans);
               context.set_color(int(fill.red() * 0.8), int(fill.green() * 0.8),
                                 int(fill.blue() * 0.8), fill.alpha() * sym.get_opacity() / 255.0);
               context.add_path(faces_path);
               context.fill();

               frame->move_to(itr->get<0>(), itr->get<1>());
               frame->line_to(itr->get<0>(), itr->get<1>() + height);

            }

            geom.rewind(0);

            for (unsigned j = 0; j < geom.num_points(); ++j)
            {
               double x, y;
               unsigned cm = geom.vertex(&x, &y);

               if (cm == SEG_MOVETO)
               {
                  frame->move_to(x, y + height);
                  roof->move_to(x, y + height);
               }
               else if (cm == SEG_LINETO)
               {
                  frame->line_to(x, y + height);
                  roof->line_to(x, y + height);
               }
            }

            path_type path(t_, *frame, prj_trans);
            context.set_color(128, 128, 128, sym.get_opacity());
            context.add_path(path);
            context.stroke();

            path_type roof_path(t_, *roof, prj_trans);
            context.set_color(sym.get_fill(), sym.get_opacity());
            context.add_path(roof_path);
            context.fill();
         }
      }
    }

   void cairo_renderer_base::process(line_symbolizer const& sym,
                                     Feature const& feature,
                                     proj_transform const& prj_trans)
   {
      typedef coord_transform2<CoordTransform,geometry2d> path_type;

      cairo_context context(context_);
      mapnik::stroke const& stroke_ = sym.get_stroke();

      context.set_color(stroke_.get_color(), stroke_.get_opacity());

      for (unsigned i = 0; i < feature.num_geometries(); ++i)
      {
         geometry2d const& geom = feature.get_geometry(i);

         if (geom.num_points() > 1)
         {
            cairo_context context(context_);
            path_type path(t_, geom, prj_trans);

            if (stroke_.has_dash())
            {
               context.set_dash(stroke_.get_dash_array());
            }

            context.set_line_join(stroke_.get_line_join());
            context.set_line_cap(stroke_.get_line_cap());
            context.set_miter_limit(4.0);
            context.set_line_width(stroke_.get_width());
            context.add_path(path);
            context.stroke();
         }
      }
   }

   void cairo_renderer_base::process(point_symbolizer const& sym,
                                     Feature const& feature,
                                     proj_transform const& prj_trans)
   {
      boost::shared_ptr<ImageData32> const& data = sym.get_image();

      if ( data )
      {
         for (unsigned i = 0; i < feature.num_geometries(); ++i)
         {
            geometry2d const& geom = feature.get_geometry(i);
            double x;
            double y;
            double z = 0;

            geom.label_position(&x, &y);
            prj_trans.backward(x, y, z);
            t_.forward(&x, &y);

            int w = data->width();
            int h = data->height();

            Envelope<double> label_ext (floor(x - 0.5 * w),
                                        floor(y - 0.5 * h),
                                        ceil (x + 0.5 * w),
                                        ceil (y + 0.5 * h));

            if (sym.get_allow_overlap() ||
                detector_.has_placement(label_ext))
            {
               cairo_context context(context_);
               int px = int(floor(x - 0.5 * w));
               int py = int(floor(y - 0.5 * h));

               context.add_image(px, py, *data, sym.get_opacity());
               detector_.insert(label_ext);
            }
         }
      }
   }

   void cairo_renderer_base::process(shield_symbolizer const& sym,
                                     Feature const& feature,
                                     proj_transform const& prj_trans)
   {
      typedef coord_transform2<CoordTransform,geometry2d> path_type;

      UnicodeString text = feature[sym.get_name()].to_unicode();
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
            cairo_context context(context_);
            string_info info(text);

            faces->set_pixel_sizes(sym.get_text_size());
            faces->get_string_info(info);

            placement_finder<label_collision_detector4> finder(detector_);

            int w = data->width();
            int h = data->height();

            for (unsigned i = 0; i < feature.num_geometries(); ++i)
            {
               geometry2d const& geom = feature.get_geometry(i);

               if (geom.num_points() > 0) // don't bother with empty geometries
               {
                  path_type path(t_, geom, prj_trans);

                  if (sym.get_label_placement() == POINT_PLACEMENT) 
                  {
                     double label_x;
                     double label_y;
                     double z = 0.0;
                     placement text_placement(info, sym, false);

                     text_placement.avoid_edges = sym.get_avoid_edges();
                     geom.label_position(&label_x, &label_y);
                     prj_trans.backward(label_x, label_y, z);
                     t_.forward(&label_x, &label_y);
                     finder.find_point_placement(text_placement, label_x, label_y);

                     for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ ii)
                     { 
                        double x = text_placement.placements[ii].starting_x;
                        double y = text_placement.placements[ii].starting_y;
                        // remove displacement from image label
                        position pos = sym.get_displacement();
                        double lx = x - boost::get<0>(pos);
                        double ly = y - boost::get<1>(pos);
                        int px = int(lx - (0.5 * w)) ;
                        int py = int(ly - (0.5 * h)) ;
                        Envelope<double> label_ext (floor(lx - 0.5 * w), floor(ly - 0.5 * h), ceil (lx + 0.5 * w), ceil (ly + 0.5 * h));

                        if (detector_.has_placement(label_ext))
                        {    
                           context.add_image(px, py, *data);
                           context.add_text(sym, text_placement.placements[ii], face_manager_, faces);

                           detector_.insert(label_ext);
                        }
                     }

                     finder.update_detector(text_placement);
                  }
                  else if (geom.num_points() > 1 && sym.get_label_placement() == LINE_PLACEMENT) 
                  {
                     placement text_placement(info, sym, true);

                     text_placement.avoid_edges = sym.get_avoid_edges();
                     finder.find_point_placements<path_type>(text_placement, path);

                     for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ ii)
                     {
                        double x = text_placement.placements[ii].starting_x;
                        double y = text_placement.placements[ii].starting_y;
                        int px = int(x - (w/2));
                        int py = int(y - (h/2));

                        context.add_image(px, py, *data);
                        context.add_text(sym, text_placement.placements[ii], face_manager_, faces);
                     }

                     finder.update_detector(text_placement);
                  }
               }
            }
         }
      }
   }

   void cairo_renderer_base::process(line_pattern_symbolizer const& sym,
                                     Feature const& feature,
                                     proj_transform const& prj_trans)
   {
      typedef coord_transform2<CoordTransform,geometry2d> path_type;

      boost::shared_ptr<ImageData32> const& image = sym.get_image();
      unsigned width(image->width());
      unsigned height(image->height());

      cairo_context context(context_);
      cairo_pattern pattern(*image);

      pattern.set_extend(Cairo::EXTEND_REPEAT);
      pattern.set_filter(Cairo::FILTER_BILINEAR);
      context.set_line_width(height);

      for (unsigned i = 0; i < feature.num_geometries(); ++i)
      {
         geometry2d const& geom = feature.get_geometry(i);

         if (geom.num_points() > 1)
         {
            path_type path(t_, geom, prj_trans);
            double length(0);
            double x0(0), y0(0);
            double x, y;

            for (unsigned cm = path.vertex(&x, &y); cm != SEG_END; cm = path.vertex(&x, &y))
            {
               if (cm == SEG_MOVETO)
               {
                  length = 0.0;
               }
               else if (cm == SEG_LINETO)
               {
                  double dx = x - x0;
                  double dy = y - y0;
                  double angle = atan2(dy, dx);
                  double offset = fmod(length, width);
                  
                  Cairo::Matrix matrix;
                  cairo_matrix_init_identity(&matrix);
                  cairo_matrix_translate(&matrix,x0,y0);
                  cairo_matrix_rotate(&matrix,angle);
                  cairo_matrix_translate(&matrix,-offset,0.5*height);
                  cairo_matrix_invert(&matrix);

                  pattern.set_matrix(matrix);
     
                  context.set_pattern(pattern);
                  
                  context.move_to(x0, y0);
                  context.line_to(x, y);
                  context.stroke();

                  length = length + hypot(x - x0, y - y0);
               }

               x0 = x;
               y0 = y;
            }
         }
      }
   }

   void cairo_renderer_base::process(polygon_pattern_symbolizer const& sym,
                                     Feature const& feature,
                                     proj_transform const& prj_trans)
   {
      typedef coord_transform2<CoordTransform,geometry2d> path_type;

      cairo_context context(context_);
      cairo_pattern pattern(*sym.get_image());
      pattern.set_extend(Cairo::EXTEND_REPEAT);

      context.set_pattern(pattern);

      for (unsigned i = 0; i < feature.num_geometries(); ++i)
      {
         geometry2d const& geom = feature.get_geometry(i);

         if (geom.num_points() > 2)
         {
            path_type path(t_, geom, prj_trans);

            context.add_path(path);
            context.fill();
         }
      }
   }

   void cairo_renderer_base::process(raster_symbolizer const& sym,
                                     Feature const& feature,
                                     proj_transform const& prj_trans)
   {
      // TODO -- at the moment raster_symbolizer is an empty class
      // used for type dispatching, but we can have some fancy raster
      // processing in a future (filters??). Just copy raster into pixmap for now.
      raster_ptr const& raster = feature.get_raster();
      if (raster)
      {
         Envelope<double> ext = t_.forward(raster->ext_);
         int start_x = int(round(ext.minx()));
         int start_y = int(round(ext.miny()));
         int raster_width = int(round(ext.width()));
         int raster_height = int(round(ext.height()));
         int end_x = start_x + raster_width;
         int end_y = start_y + raster_height;
         double err_offs_x = (ext.minx()-start_x + ext.maxx()-end_x)/2;
         double err_offs_y = (ext.miny()-start_y + ext.maxy()-end_y)/2;

         if (raster_width > 0 && raster_height > 0)
         {
            ImageData32 target(raster_width, raster_height);
            //TODO -- use cairo matrix transformation for scaling
            if (sym.get_scaling() == "fast"){
            scale_image<ImageData32>(target, raster->data_);
            } else if (sym.get_scaling() == "bilinear"){
               scale_image_bilinear<ImageData32>(target,raster->data_, err_offs_x, err_offs_y);
            } else if (sym.get_scaling() == "bilinear8"){
               scale_image_bilinear8<ImageData32>(target,raster->data_, err_offs_x, err_offs_y);
            } else {
               scale_image<ImageData32>(target,raster->data_);
            }

            cairo_context context(context_);

            //TODO -- support for advanced image merging
            context.add_image(start_x, start_y, target, sym.get_opacity());
         }
      }
   }

   void cairo_renderer_base::process(markers_symbolizer const& sym,
                                     Feature const& feature,
                                     proj_transform const& prj_trans)
   {
   }

   void cairo_renderer_base::process(text_symbolizer const& sym,
                                     Feature const& feature,
                                     proj_transform const& prj_trans)
   {
      typedef coord_transform2<CoordTransform,geometry2d> path_type;

      UnicodeString text = feature[sym.get_name()].to_unicode();
      if ( sym.get_text_convert() == TOUPPER)
      {
         text = text.toUpper();
      }
      else if ( sym.get_text_convert() == TOLOWER)
      {
         text = text.toLower();
      }

      if (text.length() > 0)
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
            cairo_context context(context_);
            string_info info(text);

            faces->set_pixel_sizes(sym.get_text_size());
            faces->get_string_info(info);

            placement_finder<label_collision_detector4> finder(detector_);

            for (unsigned i = 0; i < feature.num_geometries(); ++i)
            {
               geometry2d const& geom = feature.get_geometry(i);

               if (geom.num_points() > 0) // don't bother with empty geometries
               {
                  path_type path(t_, geom, prj_trans);
                  placement text_placement(info, sym);

                  if (sym.get_label_placement() == POINT_PLACEMENT)
                  {
                     double label_x, label_y, z = 0.0;

                     geom.label_position(&label_x, &label_y);
                     prj_trans.backward(label_x, label_y, z);
                     t_.forward(&label_x, &label_y);
                     finder.find_point_placement(text_placement, label_x, label_y);
                     finder.update_detector(text_placement);
                  }
                  else //LINE_PLACEMENT
                  {
                     finder.find_line_placements<path_type>(text_placement, path);
                  }

                  for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ii)
                  {
                     context.add_text(sym, text_placement.placements[ii], face_manager_, faces);
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

   template class cairo_renderer<Cairo::Surface>;
   template class cairo_renderer<Cairo::Context>;
}

#endif
