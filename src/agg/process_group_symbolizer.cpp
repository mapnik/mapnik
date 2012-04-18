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
#include <mapnik/attribute_collector.hpp>

// boost
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

namespace mapnik {

namespace {
/*
 * this is some nasty stuff. basically, we can't change feature 
 * or copy it by the time the code gets to here, so we have to
 * overlay the data on top and use the template system to hide
 * the fact that we're doing this.
 */
struct feature_overlay
{
   typedef mapnik::value value_type;
   typedef std::vector<value_type> cont_type;

   feature_overlay(mapnik::feature_ptr const& feature)
      : feature_(feature),
        ctx_(new context_type)
   {}

   value_type const& get(context_type::key_type const &key) const
   {
      context_type::map_type::const_iterator itr = ctx_->find(key);
      if (itr != ctx_->end())
         return get(itr->second);
      else
         return feature_->get(key);
   }

   value_type const& get(std::size_t index) const
   {
      if (index < data_.size())
         return data_[index];
      throw std::out_of_range("Index out of range");
   }
   
   template <typename T>
   void put_new(context_type::key_type const& key, T const& val)
   {
      put_new(key,value(val));
   }

   void put_new(context_type::key_type const& key, value const& val)
   {
      xcontext_type::map_type::const_iterator itr = ctx_->find(key);
      if (itr != ctx_->end()
          && itr->second < data_.size())
      {
         data_[itr->second] = val;
      }
      else
      {
         cont_type::size_type index = ctx_->push(key);
         if (index == data_.size())
            data_.push_back(val);
      }
   }   
   
   const mapnik::feature_ptr feature_;
   context_ptr ctx_;
   cont_type data_;
};
}

template <typename T>
void  agg_renderer<T>::process(group_symbolizer const& sym,
                               mapnik::feature_ptr const& feature,
                               proj_transform const& prj_trans)
{
   // find dependant columns in the rules
   std::set<std::string> columns;
   for (group_symbolizer::rules::const_iterator itr = sym.begin();
        itr != sym.end(); ++itr)
   {
      boost::apply_visitor(expression_attributes(columns), *(itr->get_filter()));
      // TODO: also go down to symbolizers too?
   }

   for (size_t col_idx = sym.get_column_index_start(); 
        col_idx != sym.get_column_index_end(); ++col_idx)
   {
      bool have_columns = true;
      
      // make a copy of feature
      feature_overlay overlay(feature);

      // copy over columns
      BOOST_FOREACH(const std::string &col_name, columns)
      {
         const std::string col_idx_name = (boost::format("%1%%2%") % col_name % col_idx).str();
         const bool have_column = feature->has_key(col_idx_name);

         if (have_column) 
         {
            overlay.put_new(col_name, feature->get(col_idx_name));
         }
         else
         {
            have_columns = false;
            break;
         }
      }

      // if all columns were present
      if (have_columns)
      {
         for (group_symbolizer::rules::const_iterator itr = sym.begin();
              itr != sym.end(); ++itr)
         {
            const group_rule &rule = *itr;
            bool match = boost::apply_visitor(evaluate<feature_overlay,value_type>(overlay), 
                                              *rule.get_filter()).to_bool();
            if (match)
            {
               // add this to the list of things to layout
               std::cerr << "Foo!\n";
            }
         }
      }
   }
}


template void agg_renderer<image_32>::process(group_symbolizer const&,
                                              mapnik::feature_ptr const&,
                                              proj_transform const&);

}
