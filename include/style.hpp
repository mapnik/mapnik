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

//$Id: style.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef STYLE_HPP
#define STYLE_HPP

//#include "feature.hpp"
#include "color.hpp"
#include "ptr.hpp"
#include "symbolizer.hpp"
//#include "rule.hpp"

#include <vector>
#include <algorithm>
#include <functional>

namespace mapnik
{
    class Style 
    {
    private:
	std::vector<ref_ptr<symbolizer> > symbols_;
	static ref_ptr<symbolizer> zero_symbol_;
    public:
	typedef std::vector<ref_ptr<symbolizer> >::const_iterator Iterator; 

	Style() {}

	Style(const ref_ptr<symbolizer>& symbol) 
	{
	    symbols_.push_back(symbol);
	}

	~Style() {}

	Style(const Style& rhs) 
	    : symbols_(rhs.symbols_) {}
	
	Style& operator=(const Style& rhs)
	{
	    if (this==&rhs) return *this;
	    symbols_=rhs.symbols_;
	    return *this;
	}
	
	void add(const ref_ptr<symbolizer>& symbol) 
	{
	    symbols_.push_back(symbol);
	}
        
	Iterator begin() const 
	{
	    return symbols_.begin();
	} 

	Iterator end() const 
	{
	    return symbols_.end();
	}
    };    
}

#endif //STYLE_HPP
