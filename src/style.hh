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

#ifndef STYLE_HH
#define STYLE_HH

#include "color.hh"
#include "ptr.hh"
#include "symbolizer.hh"

#include <vector>
#include <algorithm>
#include <functional>

namespace mapnik
{
    
    class IsActive : public std::unary_function<ref_ptr<Symbolizer>, bool>
    {
    private:
	double scale_;
    public:	
	IsActive(double scale)
	    : scale_(scale) {}
	bool operator()(const ref_ptr<Symbolizer>& symbol)
	{
	    return symbol->active(scale_);
	}
    };

    class Style 
    {
    private:
	std::vector<ref_ptr<Symbolizer> > symbols_;
	static ref_ptr<Symbolizer> zero_symbol_;
    public:
	typedef std::vector<ref_ptr<Symbolizer> >::const_iterator Iterator; 

	Style() {}

	Style(const ref_ptr<Symbolizer>& symbol) 
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
	
	void add(const ref_ptr<Symbolizer>& symbol) 
	{
	    symbols_.push_back(symbol);
	}
        
	Iterator find(double scale) const 
	{
	    return std::find_if(symbols_.begin(),symbols_.end(),IsActive(scale));
	} 

	Iterator end() const 
	{
	    return symbols_.end();
	}
    };   
}

#endif                                            //STYLE_HH
