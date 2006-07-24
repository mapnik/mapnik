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

#ifndef LABEL_PLACEMENT_HPP
#define LABEL_PLACEMENT_HPP

namespace mapnik
{
    struct point_
    {
        double x;
        double y;
        point_()
            : x(0),y(0) {}
        point_(double x_,double y_)
            : x(x_),y(y_) {}	
    };
    
    class label_placement
    {
    private:
        point_ anchor_;
        point_ displacement_;
        double rotation_;
    public:
        label_placement() 
            : anchor_(),
              displacement_(),
              rotation_(0.0) {}
	
    };
}
 
#endif //LABEL_PLACEMENT_HPP
