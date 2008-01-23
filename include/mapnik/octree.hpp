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

#ifndef _OCTREE_HPP_
#define _OCTREE_HPP_

#include <boost/format.hpp>
#include <boost/utility.hpp>
#include <vector>
#include <iostream>
#include <deque>

namespace mapnik {
	
   typedef uint8_t byte ;
   struct rgb	
   {
         byte r;
         byte g;
         byte b;
         rgb(byte r_, byte b_, byte g_)
            : r(r_), g(g_), b(b_) {}
   };

   struct RGBPolicy
   {
         const static unsigned MAX_LEVELS = 6;
         inline static unsigned index_from_level(unsigned level, rgb const& c)
         {
            unsigned shift = 7 - level;
            return (((c.r >> shift) & 1) << 2) 
               | (((c.g >> shift) & 1) << 1) 
               | ((c.b >> shift) & 1);
         }
   };

   template <typename T, typename InsertPolicy = RGBPolicy >
   class octree : private boost::noncopyable
   {	
         struct node 
         {
               node ()
                  :  reds(0),
                     greens(0),
                     blues(0),
                     count(0),
                     index(0)
               {
                  memset(&children_[0],0,sizeof(children_));
               }
      
               ~node () 
               {
                  for (unsigned i = 0;i < 8; ++i) {
                     if (children_[i] != 0) delete children_[i],children_[i]=0;
                  }
               }	

               bool is_leaf() const { return count == 0; }	
               node * children_[8];	
               unsigned reds;
               unsigned greens;
               unsigned blues;
               unsigned count;	
               uint8_t  index;					
         };
         
         std::deque<node*> reducible_[InsertPolicy::MAX_LEVELS];
         unsigned max_colors_;
         unsigned colors_;
         unsigned leaf_level_; 
    
      public:
         explicit octree(unsigned max_colors=255) 
            : max_colors_(max_colors),
              colors_(0),
              leaf_level_(InsertPolicy::MAX_LEVELS),
              root_(new node())
         {}
         
         ~octree() { delete root_;}
	
         void insert(T const& data) 
         {       
            unsigned level = 0;
            node * cur_node = root_;
            while (true) {
                             
               if ( cur_node->count > 0 || level == leaf_level_) 
               {    
                  cur_node->reds   += data.r;
                  cur_node->greens += data.g;
                  cur_node->blues  += data.b;
                  cur_node->count  += 1;
                  if (cur_node->count == 1) ++colors_;
                  
                  while ( colors_ >= max_colors_ - 1)
                     reduce();   
                  
                  break;
               }
               
               unsigned idx = InsertPolicy::index_from_level(level,data);
               if (cur_node->children_[idx] == 0) {
                  cur_node->children_[idx] = new node();
                  if (level < leaf_level_-1) 
                  {
                     reducible_[level+1].push_back(cur_node->children_[idx]);
                  }
               }
               cur_node = cur_node->children_[idx];
               ++level; 
            }
         }  
         int quantize(rgb const& c) const 
         {
            unsigned level = 0;
            node * cur_node = root_;
            while (cur_node)
            {
               if (cur_node->count != 0) return cur_node->index;
               unsigned idx = InsertPolicy::index_from_level(level,c);
               cur_node = cur_node->children_[idx];
               ++level;
            } 
            return -1;
         }
	
         void create_palette(std::vector<rgb> & palette) const
         {
            palette.reserve(colors_);
            create_palette(palette, root_);
         }
      private:	    
         void reduce()
         {	
            while (leaf_level_ >0  && reducible_[leaf_level_-1].size() == 0) 
            {
               --leaf_level_;
            }

            if (leaf_level_ < 1) return;
            
            typename std::deque<node*>::iterator pos = reducible_[leaf_level_-1].begin();
            if (pos == reducible_[leaf_level_-1].end()) return; 
            
            node * cur_node = *pos;
            unsigned num_children = 0;
            for (unsigned idx=0; idx < 8; ++idx)
            {
               if (cur_node->children_[idx] != 0)
               {
                  ++num_children;
                  cur_node->reds   += cur_node->children_[idx]->reds;
                  cur_node->greens += cur_node->children_[idx]->greens;
                  cur_node->blues  += cur_node->children_[idx]->blues;
                  cur_node->count  += cur_node->children_[idx]->count;
                  delete cur_node->children_[idx], cur_node->children_[idx]=0;
               }
            }
            
            reducible_[leaf_level_-1].erase(pos);
            if (num_children > 0 ) 
            {
               colors_ -= (num_children - 1);
            }
         }
       
         void create_palette(std::vector<rgb> & palette, node * itr) const
         {
            if (itr->count != 0)
            {
               unsigned count = itr->count;
               palette.push_back(rgb(uint8_t(itr->reds/float(count)),
                                     uint8_t(itr->greens/float(count)),
                                     uint8_t(itr->blues/float(count))));
               itr->index = palette.size() - 1;			
            }
            for (unsigned i=0; i < 8 ;++i)
            {
               if (itr->children_[i] != 0) 
                  create_palette(palette, itr->children_[i]);
            }
    
         }
      private:	
         node * root_;		
   };
} // namespace mapnik

#endif /* _OCTREE_HPP_ */
