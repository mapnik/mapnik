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

#ifndef POLYGON_PATTERN_SYMBOLIZER_HPP
#define POLYGON_PATTERN_SYMBOLIZER_HPP

#include <boost/shared_ptr.hpp>
#include "graphics.hpp" 

namespace mapnik
{
    struct MAPNIK_DECL polygon_pattern_symbolizer
    {
	
	polygon_pattern_symbolizer(std::string const& file,
				   std::string const& type,
				   unsigned width,unsigned height);
        
	polygon_pattern_symbolizer(polygon_pattern_symbolizer const& rhs);
        
	ImageData32 const& get_pattern() const;
    private:
	boost::shared_ptr<ImageData32> pattern_;
    };
}

#endif //POLYGON_PATTERN_SYMBOLIZER_HPP
