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

#ifndef SPATIAL_HH
#define SPATIAL_HH

#include "filter.hh"

namespace mapnik
{    
    template <typename Feature>
    struct equals : public filter<Feature>
    {
	int type() const 
	{
	    return filter<Feature>::SPATIAL_OPS;
	}
	
	bool pass(const Feature& feature) const
	{
	    return false;
	}
	
	void accept(const filter_visitor<Feature>& v)
	{
	    v.visit(*this);
	}
    };
    
    template <typename Feature>
    struct disjoint : public filter<Feature>
    {
	int type() const 
	{
	    return filter<Feature>::SPATIAL_OPS;
	}
	
	bool pass(const Feature& feature) const
	{
	    return false;
	}
	
	void accept(const filter_visitor<Feature>& v)
	{
	    v.visit(*this);
	}
    };
  
    template <typename Feature>
    struct touches : public filter<Feature>
    {
	int type() const 
	{
	    return filter<Feature>::SPATIAL_OPS;
	}
	
	bool pass(const Feature& feature) const
	{
	    return false;
	}
	
	void accept(const filter_visitor<Feature>& v)
	{
	    v.visit(*this);
	}
    };

    template <typename Feature>
    struct within : public filter<Feature>
    {
	int type() const 
	{
	    return filter<Feature>::SPATIAL_OPS;
	}
	
	bool pass(const Feature& feature) const
	{
	    return false;
	}

	void accept(const filter_visitor<Feature>& v)
	{
	    v.visit(*this);
	}
    };

    template <typename Feature>
    struct overlaps : public filter<Feature>
    {
	int type() const 
	{
	    return filter<Feature>::SPATIAL_OPS;
	}
	
	bool pass(const Feature& feature) const
	{
	    return false;
	}
	
	void accept(const filter_visitor<Feature>& v)
	{
	    v.visit(*this);
	}
    };

    template <typename Feature>
    struct crosses : public filter<Feature>
    {
	int type() const 
	{
	    return filter<Feature>::SPATIAL_OPS;
	}
	
	bool pass(const Feature& feature) const
	{
	    return false;
	}
	
	void accept(const filter_visitor<Feature>& v)
	{
	    v.visit(*this);
	}
    };
    
    template <typename Feature>
    struct bbox  : public filter<Feature> 
    {
    private:
	Envelope<double> box_;
    public:
	bbox(const Envelope<double>& box)
	    : box_(box) {}
	
	int type() const 
	{
	    return filter<Feature>::SPATIAL_OPS;
	}
	
	bool pass(const Feature& feature) const
	{
	    return box_.contains(feature.get_geometry()->bbox());
	}
	
	std::string to_string() 
	{
	    return "BBOX filter";
	}
	filter<Feature>* clone() const
	{
	    return new bbox<Feature>(box_);
	}
	void accept(const filter_visitor<Feature>& v)
	{
	    v.visit(*this);
	}

	virtual ~bbox() {}
    };
}

#endif //SPATIAL_HH
