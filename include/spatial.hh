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
#include "filter_visitor.hh"

namespace mapnik
{    

    template <typename FeatureT>
    struct equals_ : public filter<FeatureT>
    {

	bool pass(const FeatureT& feature) const
	{
	    return false;
	}
	
	void accept(const filter_visitor<FeatureT>& v)
	{
	    v.visit(*this);
	}
    };
    
    template <typename FeatureT>
    struct disjoint : public filter<FeatureT>
    {
	  
	
	bool pass(const FeatureT& feature) const
	{
	    return false;
	}
	
	void accept(const filter_visitor<FeatureT>& v)
	{
	    v.visit(*this);
	}
    };
  
    template <typename FeatureT>
    struct touches : public filter<FeatureT>
    {

	
	bool pass(const FeatureT& feature) const
	{
	    return false;
	}
	
	void accept(const filter_visitor<FeatureT>& v)
	{
	    v.visit(*this);
	}
    };

    template <typename FeatureT>
    struct within : public filter<FeatureT>
    {

	bool pass(const FeatureT& feature) const
	{
	    return false;
	}

	void accept(const filter_visitor<FeatureT>& v)
	{
	    v.visit(*this);
	}
    };

    template <typename FeatureT>
    struct overlaps : public filter<FeatureT>
    {

	bool pass(const FeatureT& feature) const
	{
	    return false;
	}
	
	void accept(const filter_visitor<FeatureT>& v)
	{
	    v.visit(*this);
	}
    };

    template <typename FeatureT>
    struct crosses : public filter<FeatureT>
    {

	
	bool pass(const FeatureT& feature) const
	{
	    return false;
	}
	
	void accept(const filter_visitor<FeatureT>& v)
	{
	    v.visit(*this);
	}
    };
    
    template <typename FeatureT>
    struct bbox  : public filter<FeatureT> 
    {
    private:
	Envelope<double> box_;
    public:
	bbox(const Envelope<double>& box)
	    : box_(box) {}

	
	bool pass(const FeatureT& feature) const
	{
	    return box_.contains(feature.get_geometry()->bbox());
	}
	

	filter<FeatureT>* clone() const
	{
	    return new bbox<FeatureT>(box_);
	}
	void accept(const filter_visitor<FeatureT>& v)
	{
	    v.visit(*this);
	}

	virtual ~bbox() {}
    };
}

#endif //SPATIAL_HH
