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

#ifndef WKB_HH
#define WKB_HH

#include "geometry.hh"
#include "ctrans.hh"

namespace mapnik
{
    class geometry_utils 
    {
    public:
	static geometry_ptr from_wkb(const char* wkb, unsigned size,int srid);
    private:
	geometry_utils();
	geometry_utils(const geometry_utils&);
	geometry_utils& operator=(const geometry_utils&);
    };
}
#endif                                            //WKB_HH
