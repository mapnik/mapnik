/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

// mapnik
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/symbolizer_helpers.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/attribute_collector.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/group_layout_manager.hpp>

// boost
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

// stl
#include <queue>

namespace mapnik {

namespace {
// needed for replacing check to box placement in the
// bit of code where we're pre-generating the text 
// boxes.
struct true_functor
{
   bool operator()(box2d<double> const&) const
   {
      return true;
   }
};

// a visitor to extract the bboxes that a symbolizer would use,
// were it to be rendered at 0,0 and merge them together.
struct place_bboxes : public boost::static_visitor<>
{
   // boxes vector that we will push_back to.
   box2d<double> &box_;

   // the feature we've evaluating
   Feature const& feature_;

   // ???
   processed_text &text_;

   double scale_factor_;

   place_bboxes(box2d<double> &box, 
                Feature const &feature,
                processed_text &text,
                double scale_factor)
      : box_(box), feature_(feature), text_(text), scale_factor_(scale_factor)
   {}

   void operator()(point_symbolizer const& sym) const
   {
      symbolizer_with_image_helper helper(sym, feature_);

      box_.expand_to_include(helper.get_label_ext());
   }

   void operator()(text_symbolizer const& sym) const
   {
      text_placement_info_ptr placement_ = sym.get_placement_options()->get_placement_info(scale_factor_);
      placement_->properties.process(text_, feature_);
      string_info *info_ = &(text_.get_string_info());

      text_place_boxes_at_point box_placer(*placement_, *info_);
      true_functor check;
      std::auto_ptr<text_path> current_placement(new text_path(0, 0));

      boost::optional<std::queue< box2d<double> > > maybe_boxes = 
         box_placer.check_point_placement(check, current_placement.get(), 0, 0, 0);

      if (maybe_boxes)
      {
         std::queue< box2d<double> > boxes = *maybe_boxes;
         while (!boxes.empty())
         {
            box_.expand_to_include(boxes.front());
            boxes.pop();
         }
      }
      else
      {
         // some sort of error? weren't expecting to get here.
      }
   }

   void operator()(shield_symbolizer const &sym) const
   {
   }

   template <typename T>
   void operator()(T const &) const
   {
      // some warning here about unimplemented
   }
};

/**
 * visitor to render symbolizers - this is the other
 * "half" of the process, where the first half is the
 * placement of the boxes.
 */
template <class RendererT>
struct render_visitor : public boost::static_visitor<>
{
   // the actual renderer object - here, an agg_renderer
   RendererT &renderer_;

   // position to render at
   // TODO: extend this for text rendering too
   pixel_position pixel_;

   // the feature we've evaluating
   Feature const& feature_;

   // some text settings needed for text rendering
   processed_text &text_;

   double scale_factor_;

   face_manager<freetype_engine> &font_manager_;

   image_32 &pixmap_;

   render_visitor(RendererT &renderer,
                  pixel_position const &pixel,
                  Feature const &feature,
                  processed_text &text,
                  double scale_factor,
                  face_manager<freetype_engine> &font_manager,
                  image_32 &pixmap)
      : renderer_(renderer), pixel_(pixel), feature_(feature), text_(text), 
        scale_factor_(scale_factor), font_manager_(font_manager),
        pixmap_(pixmap)
   {}

   void operator()(point_symbolizer const &sym) const
   {
      symbolizer_with_image_helper helper(sym, feature_);
      const marker &m = **helper.get_marker();
      pixel_position pos(pixel_.x - 0.5 * m.width(),
                         pixel_.y - 0.5 * m.height());

      renderer_.render_marker(pos, m, helper.get_transform(), sym.get_opacity());
   }

   void operator()(text_symbolizer const &sym) const
   {
      text_placement_info_ptr placement_ = sym.get_placement_options()->get_placement_info(scale_factor_);
      placement_->properties.process(text_, feature_);
      string_info *info_ = &(text_.get_string_info());

      text_place_boxes_at_point box_placer(*placement_, *info_);
      true_functor check;
      std::auto_ptr<text_path> current_placement(new text_path(pixel_.x, pixel_.y));

      box_placer.check_point_placement(check, current_placement.get(), 0, 0, 0);

      text_renderer<image_32> ren(pixmap_, font_manager_, *(font_manager_.get_stroker()));
      
      ren.prepare_glyphs(current_placement.get());
      ren.render(current_placement->center);
   }
   
   template <typename T>
   void operator()(T const &) const
   {
   }
};

}

template <typename T>
void  agg_renderer<T>::process(group_symbolizer const& sym,
                               mapnik::feature_ptr const& feature,
                               proj_transform const& prj_trans)
{
   // find all column names referenced in the group rules and symbolizers
   std::set<std::string> columns;
   attribute_collector collector(columns);
   
   for (group_symbolizer::rules::const_iterator itr = sym.begin();
        itr != sym.end(); ++itr)
   {
      // note that this recurses down on to the symbolizer
      // internals too, so we get all free variables.
      collector(*itr);
   }
   
   // create a new context for the sub features of this group
   mapnik::context_ptr sub_feature_ctx = boost::make_shared<mapnik::context_type>();
   
   // populate new context with column names referenced in the group rules and symbolizers
   // and determine if any indexed columns are present
   bool has_idx_cols = false;
   BOOST_FOREACH(const std::string &col_name, columns)
   {
      sub_feature_ctx->push(col_name);
      if (col_name.find('%') != std::string::npos)
      {
         has_idx_cols = true;
      }
   }

   // keep track of the sub features that we'll want to symbolize
   // along with the group rules that they matched
   std::vector< std::pair<const group_rule *, feature_ptr> > matches;

   // layout manager to store and arrange bboxes of matched features
   group_layout_manager layout_manager(sym.get_layout());
   processed_text text(font_manager_, scale_factor_);

   // run feature or sub feature through the group rules & symbolizers
   // for each index value in the range
   for (size_t col_idx = sym.get_column_index_start(); 
        col_idx != sym.get_column_index_end(); ++col_idx)
   {
      feature_ptr sub_feature;
      
      if (has_idx_cols)
      {
          // create sub feature with indexed column values
          sub_feature = feature_factory::create(sub_feature_ctx, col_idx);
          
          // copy the necessary columns to sub feature
          BOOST_FOREACH(const std::string &col_name, columns)
          {
             if (col_name.find('%') != std::string::npos)
             {
                // indexed column
                std::string col_idx_name = col_name;
                boost::replace_all(col_idx_name, "%", boost::lexical_cast<std::string>(col_idx));
                sub_feature->put(col_name, feature->get(col_idx_name));
             }
             else
             {
                // non-indexed column
                sub_feature->put(col_name, feature->get(col_name));
             }
          }
      }
      else
      {
          // no indexed columns, so use the existing feature instead of copying
          sub_feature = feature;
      }

      for (group_symbolizer::rules::const_iterator itr = sym.begin();
           itr != sym.end(); ++itr)
      {
         const group_rule &rule = *itr;
         
         if (boost::apply_visitor(evaluate<Feature,value_type>(*sub_feature), 
                                           *rule.get_filter()).to_bool())
         {
            // add matched rule and feature to the list of things to draw
            matches.push_back(std::make_pair(&rule, sub_feature));

            // construct a bounding box around all symbolizers for the matched rule
            box2d<double> box;
            for (group_rule::symbolizers::const_iterator itr = rule.begin();
                 itr != rule.end(); ++itr)
            {
               boost::apply_visitor(place_bboxes(box, *sub_feature, text, scale_factor_), *itr);
            }

            // add the bounding box to the layout manager
            layout_manager.add_member_bound_box(box);
            break;
         }
      }
   }

   // find placements which can accomodate the offset bboxes from the layout manager.
   string_info empty_info;
   text_placement_info_ptr placement = sym.get_placement_options()->get_placement_info(scale_factor_);
   placement_finder<label_collision_detector4> finder(*feature, *placement, empty_info, *detector_, query_extent_);
   for (size_t i = 0; i < matches.size(); ++i)
   {
      finder.additional_boxes.push_back(layout_manager.offset_box_at(i));
   }

   // for each placement:
   //    render the symbolizer at that point.
   unsigned int num_geoms = feature->num_geometries();
   for (unsigned int i = 0; i < num_geoms; ++i)
   {
      typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
      typedef coord_transform2<CoordTransform,clipped_geometry_type> path_type;

      geometry_type const &geom = feature->get_geometry(i);
      clipped_geometry_type clipped(const_cast<geometry_type &>(geom));
      clipped.clip_box(query_extent_.minx(),query_extent_.miny(),
                       query_extent_.maxx(),query_extent_.maxy());
      path_type path(t_, clipped, prj_trans);

      finder.clear_placements();
      finder.find_point_placements(path);
      finder.update_detector();

      BOOST_FOREACH(text_path const &p, finder.get_results())
      {
         for (size_t j = 0; j < matches.size(); ++j)
         {
            const group_rule *match_rule = matches[j].first;
            feature_ptr match_feature = matches[j].second;
            pixel_position pos = p.center;
            const layout_offset &offset = layout_manager.offset_at(j);
            pos.x += offset.x;
            pos.y += offset.y;
            
            // finally, do the actual rendering.
            render_visitor<agg_renderer> symbolize(*this, pos, *match_feature, text, 
                                                    scale_factor_, font_manager_,
                                                    pixmap_);

            for (group_rule::symbolizers::const_iterator itr = match_rule->begin();
                itr != match_rule->end(); ++itr)
            {
               boost::apply_visitor(symbolize, *itr);
            }
         }
      }
   }
}


template void agg_renderer<image_32>::process(group_symbolizer const&,
                                              mapnik::feature_ptr const&,
                                              proj_transform const&);

}
