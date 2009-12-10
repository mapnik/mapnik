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

#ifndef QUERY_HPP
#define QUERY_HPP
//mapnik
#include <mapnik/filter.hpp>
#include <mapnik/envelope.hpp>
#include <mapnik/feature.hpp>
// stl
#include <set>
#include <limits>

namespace mapnik {
   class query 
   {
      private:
         Envelope<double> bbox_;
         double resolution_;
         double scale_denominator_;
         std::set<std::string> names_;
      public:
         
         explicit query(const Envelope<double>& bbox, double resolution, double scale_denominator)
            : bbox_(bbox),
              resolution_(resolution),
              scale_denominator_(scale_denominator)
         {}

         explicit query(const Envelope<double>& bbox, double resolution)
            : bbox_(bbox),
              resolution_(resolution),
              scale_denominator_(0.0)
         {}         
        
         query(const query& other)
            : bbox_(other.bbox_),
              resolution_(other.resolution_),
              scale_denominator_(other.scale_denominator_),
              names_(other.names_)
         {}
         
         query& operator=(const query& other)
         {
            if (this == &other) return *this;
            bbox_=other.bbox_;
            resolution_=other.resolution_;
            scale_denominator_=other.scale_denominator_;
            names_=other.names_;
            return *this;
         }
         
         double resolution() const
         {
            return resolution_;
         }

         double scale_denominator() const
         {
            return scale_denominator_;
         }
         
         const Envelope<double>& get_bbox() const
         {
            return bbox_;
         }
         
         void add_property_name(const std::string& name)
         {
            names_.insert(name);
         } 
         
         const std::set<std::string>& property_names() const
         {
            return names_;
         }
   };
}


#endif //QUERY_HPP
