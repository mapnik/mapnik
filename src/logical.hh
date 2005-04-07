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

//$Id: logical.hh 11 2005-03-16 19:46:18Z artem $


#ifndef LOGICAL_HH
#define LOGICAL_HH

#include "filter.hh"
#include "feature.hh"

namespace mapnik
{
    template <typename Feature> 
    struct logical_and : public filter<Feature>  
    {
	logical_and(const filter<Feature>& filter1,const filter<Feature>& filter2)
	    : filter1_(filter1.clone()),
	      filter2_(filter2.clone()) {}

	int type() const
	{
	    return filter<Feature>::LOGICAL_OPS;
	}

	bool pass(const Feature& feature) const
	{
	    return (filter1_->pass(feature) && 
		filter2_->pass(feature));
	}
	filter<Feature>* clone() const
	{
	    return new logical_and(*filter1_,*filter2_);
	}

	void accept(filter_visitor<Feature>& v)
	{
	    filter1_->accept(v);
	    filter2_->accept(v);
	    v.visit(*this);
	}

	virtual ~logical_and()
	{
	    delete filter1_;
	    delete filter2_;
	}
	
    private:
	filter<Feature>* filter1_;
	filter<Feature>* filter2_;
    };

    template <typename Feature> 
    struct logical_or : public filter<Feature>  
    {
	
	logical_or(const filter<Feature>& filter1,const filter<Feature>& filter2)
	    : filter1_(filter1.clone()),
	      filter2_(filter2.clone()) {}

	int type() const
	{
	    return filter<Feature>::LOGICAL_OPS;
	}

	bool pass(const Feature& feature) const
	{
	    return (filter1_->pass(feature) ||
		filter2_->pass(feature));
	}
	filter<Feature>* clone() const
	{
	    return new logical_or(*filter1_,*filter2_);
	}

	void accept(filter_visitor<Feature>& v)
	{
	    filter1_->accept(v);
	    filter2_->accept(v);
	    v.visit(*this);
	}

	virtual ~logical_or()
	{  
	    delete filter1_;
	    delete filter2_;
	}
    private:
	filter<Feature>* filter1_;
	filter<Feature>* filter2_;
    };

    template <typename Feature> 
    struct logical_not : public filter<Feature>  
    {
	logical_not(const filter<Feature>& filter)
	    : filter_(filter.clone()) {}
	int type() const
	{
	    return filter<Feature>::LOGICAL_OPS;
	}

	bool pass(const Feature& feature) const
	{
	    return !(filter_->pass(feature));
	}

	filter<Feature>* clone() const
	{
	    return new logical_not(*filter_);
	}
	
	void accept(filter_visitor<Feature>& v)
	{
	    filter_->accept(v);
	    v.visit(*this);
	}
	
	~logical_not() 
	{
	    delete filter_;
	}
    private:
	filter<Feature>* filter_;
    };
}

#endif //LOGICAL_HH
