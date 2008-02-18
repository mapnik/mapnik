/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
 * Copyright (C) 2006 10East Corp.
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

#ifndef __TEXT_PATH_H__
#define __TEXT_PATH_H__

#include <boost/utility.hpp>
#include <unicode/unistr.h>

namespace mapnik
{
   struct character_info
   { 
         int character;
         double width, height;
      
         character_info() : character(0), width(0), height(0) {}
         character_info(int c_, double width_, double height_) : character(c_), width(width_), height(height_) {}
         ~character_info() {}
        
         character_info(const character_info &ci)
            : character(ci.character), width(ci.width), height(ci.height)
         {
         }
          
   };
    
   class string_info : private boost::noncopyable
   {
      protected:
         typedef boost::ptr_vector<character_info> characters_t;
         characters_t characters_;
         UnicodeString const& text_;
         double width_;
         double height_;
      public:
         string_info(UnicodeString const& text)
            : text_(text),
              width_(0),
              height_(0) {}

         void add_info(int c, double width, double height)
         {
            characters_.push_back(new character_info(c, width, height));
         }
      
         unsigned num_characters() const
         {
            return characters_.size();
         }
      
         character_info at(unsigned i) const
         {
            return characters_[i];
         }
      
         character_info operator[](unsigned i) const
         {
            return at(i);
         }
      
         void set_dimensions(double width, double height)
         {
            width_ = width;
            height_ = height;
         }
      
         std::pair<double, double> get_dimensions() const
         {
            return std::pair<double, double>(width_, height_);
         }

         UnicodeString const&  get_string() const 
         {
           return text_;
         }
   };
    
   struct text_path : boost::noncopyable
   {
         struct character_node
         {
               int c;
               double x, y, angle;
               
               character_node(int c_, double x_, double y_, double angle_) 
                  : c(c_), x(x_), y(y_), angle(angle_) {}
               ~character_node() {}
               
               void vertex(int *c_, double *x_, double *y_, double *angle_)
               {
                  *c_ = c;
                  *x_ = x;
                  *y_ = y;
                  *angle_ = angle;
               }
         };
         
         typedef std::vector<character_node> character_nodes_t;
         double starting_x;
         double starting_y;
         character_nodes_t nodes_;
         int itr_;
          
         std::pair<unsigned,unsigned> string_dimensions;
        
         text_path() 
            : starting_x(0),
              starting_y(0),
              itr_(0) {} 
         
         //text_path(text_path const& other) : 
         //  itr_(0),
         //  nodes_(other.nodes_),
         //  string_dimensions(other.string_dimensions)
         //{}
         
         ~text_path() {}
          
         void add_node(int c, double x, double y, double angle)
         {
            nodes_.push_back(character_node(c, x, y, angle));
         }
        
         void vertex(int *c, double *x, double *y, double *angle)
         {
            nodes_[itr_++].vertex(c, x, y, angle);
         }
         
         void rewind()
         {
            itr_ = 0;
         }
         
         int num_nodes() const
         {
            return nodes_.size();
         }
         
         void clear()
         {
            nodes_.clear();
         }
   };
}

#endif


