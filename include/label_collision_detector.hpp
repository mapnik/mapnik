/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2006 Artem Pavlenko
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


#if !defined LABEL_COLLISION_DETECTOR
#define LABEL_COLLISION_DETECTOR

#include "envelope.hpp"
#include <vector>

namespace mapnik
{
    //this needs to be tree structure 
    //as a proof of a concept _only_ we use sequential scan 

    struct label_collision_detector
    {
	typedef std::vector<Envelope<double> > label_placements;

	bool allowed_to_render(Envelope<double> const& box)
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
}

#endif 
