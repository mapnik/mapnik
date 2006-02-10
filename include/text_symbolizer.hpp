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

#ifndef TEXT_SYMBOLIZER_HPP
#define TEXT_SYMBOLIZER_HPP

//#include "symbolizer.hpp"
//#include "fill.hpp"
//#include "expression.hpp"

namespace mapnik
{
    struct text_symbolizer
    {
	text_symbolizer(std::string const& name,Color const& fill)
	    : name_(name),
	      fill_(fill) {}

	text_symbolizer(text_symbolizer const& rhs)
	    : name_(rhs.name_),
	      fill_(rhs.fill_) {}
	
	~text_symbolizer()
	{
	    //
	}
	std::string const& get_name() const
	{
	    return name_;
	}
    private:
	std::string name_;
	Color fill_;		    
    };
}

#endif //TEXT_SYMBOLIZER_HPP
