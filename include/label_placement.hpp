/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

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
