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

#if !defined LABEL_COLLISION_DETECTOR
#define LABEL_COLLISION_DETECTOR
// stl
#include <vector>
// mapnik
#include <mapnik/quad_tree.hpp>

namespace mapnik
{
    //this needs to be tree structure 
    //as a proof of a concept _only_ we use sequential scan 

    struct label_collision_detector
    {
        typedef std::vector<Envelope<double> > label_placements;

        bool has_plasement(Envelope<double> const& box)
        {
            label_placements::const_iterator itr=labels_.begin();
            for( ; itr !=labels_.end();++itr)
            {
                if (itr->intersects(box))
                {
                    return false;
                }
            }
            labels_.push_back(box);
            return true;
        }
    private:

        label_placements labels_;
    };

    // quad_tree based label collision detector
    class label_collision_detector2 : boost::noncopyable
    {
        typedef quad_tree<Envelope<double> > tree_t;
        tree_t tree_;
    public:
	
        explicit label_collision_detector2(Envelope<double> const& extent)
            : tree_(extent) {}
	
        bool has_placement(Envelope<double> const& box)
        {
            tree_t::query_iterator itr = tree_.query_in_box(box);
            tree_t::query_iterator end = tree_.query_end();
	    
            for ( ;itr != end; ++itr)
            {
                if (itr->intersects(box))
                {
                    return false;
                }
            }
	    
            tree_.insert(box,box);
            return true;
        }	
    };
    
    // quad_tree based label collision detector with seperate check/insert
    class label_collision_detector3 : boost::noncopyable
    {
      typedef quad_tree< Envelope<double> > tree_t;
      tree_t tree_;
    public:
	
      explicit label_collision_detector3(Envelope<double> const& extent)
          : tree_(extent) {}
	
      bool has_placement(Envelope<double> const& box)
      {
          tree_t::query_iterator itr = tree_.query_in_box(box);
          tree_t::query_iterator end = tree_.query_end();
          
          for ( ;itr != end; ++itr)
          {
            if (itr->intersects(box))
            {
                return false;
            }
          }
          
          return true;
      }

      void insert(Envelope<double> const& box)
      {
        tree_.insert(box, box);
      }
    };
}

#endif 
