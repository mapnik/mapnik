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

// $Id$

#ifndef FILTER_HH
#define FILTER_HH

#include "envelope.hh"
#include "feature.hh"
#include "filter_visitor.hh"

namespace mapnik
{
    template <typename Feature>	
    struct filter
    {
	enum {
	    NULL_OPS,
	    SPATIAL_OPS,
	    COMPARISON_OPS,
	    LOGICAL_OPS,
	    FEATUREID_OPS
	};
        
	virtual int  type() const=0;
	virtual bool pass (const Feature& feature) const = 0;
	virtual filter<Feature>* clone() const = 0;
	virtual void accept(filter_visitor<Feature>& v) = 0;
	virtual ~filter() {}
    };
    
    template <typename Feature>
    struct null_filter : public filter<Feature>
    {
	typedef filter<Feature> _Base_;
	int  type() const 
	{ 
	    return _Base_::NULL_OPS;
	}

	bool pass (const Feature&) const
	{
	    return true;
	}

	_Base_* clone() const 
	{ 
	    return new null_filter<Feature>;
	}

	void accept(filter_visitor<Feature>& v)
	{
	    v.visit(*this);
	}

	~null_filter() {}
    };
}

#endif //FILTER_HH
