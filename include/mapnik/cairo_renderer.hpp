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

#ifndef CAIRO_RENDERER_HPP
#define CAIRO_RENDERER_HPP



// mapnik
#include <mapnik/config.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/map.hpp>

// cairo
#include <cairomm/context.h>
#include <cairomm/surface.h>

// boost
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

namespace mapnik {

   class cairo_face;

   typedef boost::shared_ptr<cairo_face> cairo_face_ptr;

   class cairo_face_manager : private boost::noncopyable
   {
     public:
      cairo_face_manager(boost::shared_ptr<freetype_engine> engine,
                         face_manager<freetype_engine> & manager);
      cairo_face_ptr get_face(face_ptr face);

     private:
      typedef std::map<face_ptr,cairo_face_ptr> cairo_face_cache;
      boost::shared_ptr<freetype_engine> font_engine_;
      face_manager<freetype_engine> & font_manager_;
      cairo_face_cache cache_;
   };

   class MAPNIK_DECL cairo_renderer_base : private boost::noncopyable
   {
     protected:
      cairo_renderer_base(Map const& m, Cairo::RefPtr<Cairo::Context> const& context, unsigned offset_x=0, unsigned offset_y=0);
     public:
      ~cairo_renderer_base();
      void start_map_processing(Map const& map);
      void start_layer_processing(Layer const& lay);
      void end_layer_processing(Layer const& lay);
      void process(point_symbolizer const& sym,
                   Feature const& feature,
                   proj_transform const& prj_trans);
      void process(line_symbolizer const& sym,
                   Feature const& feature,
                   proj_transform const& prj_trans);
      void process(line_pattern_symbolizer const& sym,
                   Feature const& feature,
                   proj_transform const& prj_trans);
      void process(polygon_symbolizer const& sym,
                   Feature const& feature,
                   proj_transform const& prj_trans);
      void process(polygon_pattern_symbolizer const& sym,
                   Feature const& feature,
                   proj_transform const& prj_trans);
      void process(raster_symbolizer const& sym,
                   Feature const& feature,
                   proj_transform const& prj_trans);
      void process(shield_symbolizer const& sym,
                   Feature const& feature,
                   proj_transform const& prj_trans);
      void process(text_symbolizer const& sym,
                   Feature const& feature,
                   proj_transform const& prj_trans);
      void process(building_symbolizer const& sym,
                   Feature const& feature,
                   proj_transform const& prj_trans);
      void process(markers_symbolizer const& sym,
                   Feature const& feature,
                   proj_transform const& prj_trans);
     protected:
      Map const& m_;
      Cairo::RefPtr<Cairo::Context> context_;
      CoordTransform t_;
      boost::shared_ptr<freetype_engine> font_engine_;
      face_manager<freetype_engine> font_manager_;
      cairo_face_manager face_manager_;
      label_collision_detector4 detector_;
   };

   template <typename T>
   class MAPNIK_DECL cairo_renderer : public feature_style_processor<cairo_renderer<T> >,
                                      public cairo_renderer_base
   {
     public:
      cairo_renderer(Map const& m, Cairo::RefPtr<T> const& surface, unsigned offset_x=0, unsigned offset_y=0);
      void end_map_processing(Map const& map);
   };
}

#endif

#endif //CAIRO_RENDERER_HPP
