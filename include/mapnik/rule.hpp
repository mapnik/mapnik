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

#ifndef RULE_HPP
#define RULE_HPP
// mapnik
#include <mapnik/line_symbolizer.hpp>
#include <mapnik/line_pattern_symbolizer.hpp>
#include <mapnik/polygon_symbolizer.hpp>
#include <mapnik/polygon_pattern_symbolizer.hpp>
#include <mapnik/point_symbolizer.hpp>
#include <mapnik/raster_symbolizer.hpp>
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/markers_symbolizer.hpp>
#include <mapnik/filter.hpp>
#include <mapnik/filter_visitor.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
// stl
#include <string>
#include <vector>

namespace mapnik
{
   inline bool operator==(point_symbolizer const& lhs,
                          point_symbolizer const& rhs)
   {
      return (&lhs == &rhs); 
   }
   inline bool operator==(line_symbolizer const& lhs,
                          line_symbolizer const& rhs)
   {
      return (&lhs == &rhs); 
   }
   inline bool operator==(line_pattern_symbolizer const& lhs,
                          line_pattern_symbolizer const& rhs)
   {
      return (&lhs == &rhs); 
   }

   inline bool operator==(polygon_symbolizer const& lhs,
                          polygon_symbolizer const& rhs)
   {
      return (&lhs == &rhs); 
   }
    
   inline bool operator==(polygon_pattern_symbolizer const& lhs,
                          polygon_pattern_symbolizer const& rhs)
   {
      return (&lhs == &rhs); 
   }
    
   inline bool operator==(raster_symbolizer const& lhs,
                          raster_symbolizer const& rhs)
   {
      return (&lhs == &rhs); 
   }
    
   inline bool operator==(text_symbolizer const& lhs,
                          text_symbolizer const& rhs)
   {
      return (&lhs == &rhs); 
   }
    
   inline bool operator==(shield_symbolizer const& lhs,
                          shield_symbolizer const& rhs)
   {
      return (&lhs == &rhs); 
   }
    
   inline bool operator==(building_symbolizer const& lhs,
                          building_symbolizer const& rhs)
   {
      return (&lhs == &rhs); 
   }
   
   inline bool operator==(markers_symbolizer const& lhs,
                          markers_symbolizer const& rhs)
   {
      return (&lhs == &rhs); 
   }
   typedef boost::variant<point_symbolizer,
                          line_symbolizer,
                          line_pattern_symbolizer,
                          polygon_symbolizer,
                          polygon_pattern_symbolizer,
                          raster_symbolizer,
                          shield_symbolizer,
                          text_symbolizer,
                          building_symbolizer,
                          markers_symbolizer> symbolizer;
    
        
   typedef std::vector<symbolizer> symbolizers;
    
   template <typename FeatureT> class all_filter;

   template <typename FeatureT,template <typename> class Filter>
   class rule
   {
         typedef Filter<FeatureT> filter_type;
         typedef boost::shared_ptr<filter_type> filter_ptr;
      private:

         std::string name_;
         std::string title_;
         std::string abstract_;
         double min_scale_;
         double max_scale_;
         symbolizers syms_;
         filter_ptr filter_;
         bool else_filter_;
      public:
         rule()
            : name_(),
              title_(),
              abstract_(),
              min_scale_(0),
              max_scale_(std::numeric_limits<double>::infinity()),
              syms_(),
              filter_(new all_filter<FeatureT>),
              else_filter_(false) {}
                
         rule(const std::string& name,
              const std::string& title="",
              double min_scale_denominator=0,
              double max_scale_denominator=std::numeric_limits<double>::infinity())
            : name_(name),
              title_(title),
              min_scale_(min_scale_denominator),
              max_scale_(max_scale_denominator),
              syms_(),
              filter_(new all_filter<FeatureT>),
              else_filter_(false) {}
	    
         rule(const rule& rhs)    
            : name_(rhs.name_),
              title_(rhs.title_),
              abstract_(rhs.abstract_),
              min_scale_(rhs.min_scale_),
              max_scale_(rhs.max_scale_),
              syms_(rhs.syms_),
              filter_(rhs.filter_),
              else_filter_(rhs.else_filter_) {}
	
         rule& operator=(rule const& rhs) 
         {
            rule tmp(rhs);
            swap(tmp);
            return *this;
         }
         bool operator==(rule const& other)
         {
            return  (this == &other); 
         }
	
         void set_max_scale(double scale)
         {
            max_scale_=scale;
         }
        
         double get_max_scale() const
         {
            return max_scale_;
         }
        
         void set_min_scale(double scale)
         {
            min_scale_=scale;
         }

         double get_min_scale() const
         {
            return min_scale_;
         }

         void set_name(std::string const& name)
         {
            name_=name;
         }
	
         std::string const& get_name() const
         {
            return name_;
         }
	
         std::string const& get_title() const
         {
            return  title_;
         }

         void set_title(std::string const& title)
         {
            title_=title;
         }
  
         void set_abstract(std::string const& abstract)
         {
            abstract_=abstract;
         }
	
         std::string const& get_abstract() const
         {
            return abstract_;
         }
		
         void append(const symbolizer& sym)
         {
            syms_.push_back(sym);
         }
	
         void remove_at(size_t index)
         {
            if (index < syms_.size())
            {
               syms_.erase(syms_.begin()+index);
            }
         }
	
         const symbolizers& get_symbolizers() const
         {
            return syms_;
         }
	
         symbolizers::const_iterator begin() const
         {
            return syms_.begin();
         }
	
         symbolizers::const_iterator end() const
         {
            return syms_.end();
         }
	
         symbolizers::iterator begin()
         {
            return syms_.begin();
         }
          
         symbolizers::iterator end()
         {
            return syms_.end();
         }
          
         void set_filter(const filter_ptr& filter)
         {
            filter_=filter;
         }

         filter_ptr const& get_filter() const
         {
            return filter_;
         }
	
         void set_else(bool else_filter)
         {
            else_filter_=else_filter;
         }
     
         bool has_else_filter() const
         {
            return else_filter_;
         }
        
         bool active(double scale) const
         {
            return ( scale >= min_scale_ - 1e-6 && scale < max_scale_ + 1e-6);
         }

         void accept(filter_visitor<FeatureT>& v) const
         {
            v.visit(*this);
         }
	
      private:
	
         void swap(rule& rhs) throw()
         {
            name_=rhs.name_;
            title_=rhs.title_;
            abstract_=rhs.abstract_;
            min_scale_=rhs.min_scale_;
            max_scale_=rhs.max_scale_;
            syms_=rhs.syms_;
            filter_=rhs.filter_;
            else_filter_=rhs.else_filter_;
         }
   };

   typedef rule<Feature,filter> rule_type;
}

#endif //RULE_HPP
