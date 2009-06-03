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

#ifndef MARKERS_CONVERTER_HPP
#define MARKERS_CONVERTER_HPP

#include "agg_basics.h"
#include "agg_trans_affine.h"
#include <boost/utility.hpp>

namespace mapnik {
   template <typename Locator, typename Shape, typename Detector>
   class markers_converter : boost::noncopyable
   {
      public:
         markers_converter(Locator & locator,Shape & shape, Detector & detector);
         void rewind(unsigned path_id);
         unsigned vertex(double * x, double * y);
      private:
         enum status_e
         {
            initial,
            markers,
            polygon,
            stop
         };
         
         Locator & locator_;
         Shape & shape_;
         Detector & detector_;
         status_e status_;
         agg::trans_affine transform_;
         agg::trans_affine mtx_;
         unsigned num_markers_;
         unsigned marker_;
   };

   template <typename Locator,typename Shape,typename Detector>
   markers_converter<Locator,Shape,Detector>::markers_converter(Locator & locator,Shape & shape, 
                                                                Detector & detector)
      : locator_(locator),
        shape_(shape),
        detector_(detector),
        status_(initial),
        num_markers_(1),
        marker_(0) {}
   
   template <typename Locator, typename Shape,typename Detector>
   void markers_converter<Locator,Shape,Detector>::rewind(unsigned path_id)
   {
      status_ = initial;
      marker_ = 0;
      num_markers_ = 1;
   }
   
   template <typename Locator, typename Shape, typename Detector>
   unsigned markers_converter<Locator,Shape,Detector>::vertex( double * x, double * y)
   {
      unsigned cmd = agg::path_cmd_move_to;
      
      double x1, y1, x2, y2;
      while (!agg::is_stop(cmd))
      {
         switch (status_)
         {
            case initial:
               if (num_markers_ == 0 )
               {
                  cmd = agg::path_cmd_stop;
                  break;
               }
               locator_.rewind(marker_++);
               status_ = markers;
               num_markers_ = 0;
               break;
            case markers:
            {
               unsigned cmd1;
               while (agg::is_move_to(cmd1 = locator_.vertex(&x1,&y1)));
               if (agg::is_stop(cmd1))
               {
                  status_ = stop;
                  break;
               }
               unsigned cmd2 = locator_.vertex(&x2,&y2);
               if (agg::is_stop(cmd2))
               {
                  status_ = stop;
                  break;
               }
              

               ++num_markers_;
               double dx = x2 - x1;
               double dy = y2 - y1;
               double d = std::sqrt(dx * dx + dy * dy);
               Envelope<double> ext = shape_.extent();
               if (d > ext.width())
               {
                  mtx_ = transform_;
                  mtx_ *= agg::trans_affine_rotation(::atan2(dy,dx));
                  double marker_x = x1 + dx * 0.5;
                  double marker_y = y1 + dy * 0.5;
                     
                  mtx_ *= agg::trans_affine_translation(marker_x,marker_y);
                                   
                  double minx = ext.minx();
                  double miny = ext.miny();
                  double maxx = ext.maxx();
                  double maxy = ext.maxy();
                  mtx_.transform(&minx,&miny);
                  mtx_.transform(&maxx,&maxy);
                     
                  Envelope<double> e0(minx,miny,maxx,maxy);
                     
                  if (detector_.has_placement(e0))
                  {
                     shape_.rewind(0);
                     status_ = polygon;
                     detector_.insert(ext);
                  }
               }
               
               break;
            }
            case polygon:
               cmd = shape_.vertex(x,y);
               if (agg::is_stop(cmd))
               {
                  cmd = agg::path_cmd_move_to;
                  status_ = markers;
                  break;
               }
               mtx_.transform(x,y);
               return cmd;
            case stop:
               cmd = agg::path_cmd_stop;
               break;
         }
      }
      return cmd;
   }
}

#endif // MARKERS_CONVERTER_HPP
