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

#ifndef FILTER_HPP
#define FILTER_HPP

#include <mapnik/config.hpp>
#include <mapnik/feature.hpp>

namespace mapnik
{
    template <typename FeatureT> class filter_visitor;
    template <typename FeatureT>
    class MAPNIK_DECL filter
    {
      public:
       virtual bool pass(const FeatureT& feature) const=0; 
       virtual filter<FeatureT>* clone() const=0;
       virtual void accept(filter_visitor<FeatureT>& v) = 0;
       virtual std::string to_string() const=0;
       virtual ~filter() {}
    };
    
    typedef boost::shared_ptr<filter<Feature> > filter_ptr;
    
    template <typename FeatureT>
    class all_filter : public filter<FeatureT>
    {
       public:
          bool pass (const FeatureT&) const
          {
             return true;
          }
          
          filter<FeatureT>* clone() const
          {
             return new all_filter<FeatureT>;
          }
          std::string to_string() const
          {
             return "true";
          }  
          void accept(filter_visitor<FeatureT>&) {}
          virtual ~all_filter() {}
    };
   
   template <typename FeatureT>
   class none_filter : public filter<FeatureT>
   {
      public:
         bool pass (const FeatureT&) const
         {
            return false;
         }
         
         filter<FeatureT>* clone() const
         {
            return new none_filter<FeatureT>;
         }
         std::string to_string() const
         {
            return "false";
         }  
         void accept(filter_visitor<FeatureT>&) {}
         virtual ~none_filter() {}
	};
}

#endif //FILTER_HPP
