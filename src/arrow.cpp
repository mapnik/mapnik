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

#include <mapnik/arrow.hpp>

#include <agg_basics.h>

namespace mapnik {

   arrow::arrow() 
      : pos_(0) 
   {
      x_[0] = -7.0; y_[0] = 1.0; cmd_[0] = agg::path_cmd_move_to;
      x_[1] =  1.0; y_[1] = 1.0; cmd_[1] = agg::path_cmd_line_to;
      x_[2] =  1.0; y_[2] = 3.0; cmd_[2] = agg::path_cmd_line_to;
      x_[3] =  7.0; y_[3] = 0.0; cmd_[3] = agg::path_cmd_line_to;
      x_[4] =  1.0; y_[4] =-3.0; cmd_[4] = agg::path_cmd_line_to;
      x_[5] =  1.0; y_[5] =-1.0; cmd_[5] = agg::path_cmd_line_to;
      x_[6] = -7.0; y_[6] =-1.0; cmd_[6] = agg::path_cmd_line_to;
      cmd_[7] = agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw;
      cmd_[8] = agg::path_cmd_stop;
   }

   void arrow::rewind(unsigned )
   {
      pos_ = 0;
   }
    
   unsigned arrow::vertex(double* x, double* y)
   {
      if(pos_ < 7 )
      {
         *x = x_[pos_];
         *y = y_[pos_];
         return cmd_[pos_++];
      }
      return agg::path_cmd_stop;
   }
   
   Envelope<double> arrow::extent() const
   {
      return Envelope<double>(-7,-3,7,3);
   }
}

