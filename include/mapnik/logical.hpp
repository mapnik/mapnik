/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/
//$Id$

#ifndef LOGICAL_HPP
#define LOGICAL_HPP

#include <mapnik/filter.hpp>

namespace mapnik
{
    template <typename FeatureT> 
    struct logical_and : public filter<FeatureT>  
    {
	logical_and(filter<FeatureT> const& filter1,
		    filter<FeatureT> const& filter2)
	    : filter<FeatureT>(),
	      filter1_(filter1.clone()),
	      filter2_(filter2.clone()) {}
	
	logical_and(logical_and const& other)
	    : filter<FeatureT>(),
	      filter1_(other.filter1_->clone()),
	      filter2_(other.filter2_->clone()) {}

	bool pass(const FeatureT& feature) const
	{
	    return (filter1_->pass(feature) && 
		filter2_->pass(feature));
	}
	std::string to_string() const
	{
	    return "("+filter1_->to_string()+" and "+filter2_->to_string()+")";
	}
	
	filter<FeatureT>* clone() const
	{
	    return new logical_and(*this);
	}

	void accept(filter_visitor<FeatureT>& v)
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
	filter<FeatureT>* filter1_;
	filter<FeatureT>* filter2_;
    };

    template <typename FeatureT> 
    struct logical_or : public filter<FeatureT>  
    {
	
	logical_or(const filter<FeatureT>& filter1,const filter<FeatureT>& filter2)
	    : filter<FeatureT>(),
	      filter1_(filter1.clone()),
	      filter2_(filter2.clone()) {}
	
	logical_or(logical_or const& other)
	    : filter<FeatureT>(),
	      filter1_(other.filter1_->clone()),
	      filter2_(other.filter2_->clone()) {}

	bool pass(const FeatureT& feature) const
	{
	    if (filter1_->pass(feature))
	    {
		return true;
	    }
	    else
	    {
		return filter2_->pass(feature);
	    }
	}
	filter<FeatureT>* clone() const
	{
	    return new logical_or(*this);
	}

	void accept(filter_visitor<FeatureT>& v)
	{
	    filter1_->accept(v);
	    filter2_->accept(v);
	    v.visit(*this);
	}
	std::string to_string() const
	{
	    return "("+filter1_->to_string()+" or "+filter2_->to_string()+")";
	}	
	virtual ~logical_or()
	{  
	    delete filter1_;
	    delete filter2_;
	}
    private:
	filter<FeatureT>* filter1_;
	filter<FeatureT>* filter2_;
    };

    template <typename FeatureT> 
    struct logical_not : public filter<FeatureT>  
    {
	logical_not(filter<FeatureT> const& _filter)
	    : filter<FeatureT>(),
	      filter_(_filter.clone()) {}
	logical_not(logical_not const& other)
	    : filter<FeatureT>(),
	      filter_(other.filter_->clone()) {}

	int type() const
	{
	    return filter<FeatureT>::LOGICAL_OPS;
	}

	bool pass(const FeatureT& feature) const
	{
	    return !(filter_->pass(feature));
	}

	filter<FeatureT>* clone() const
	{
	    return new logical_not(*this);
	}
	
	void accept(filter_visitor<FeatureT>& v)
	{
	    filter_->accept(v);
	    v.visit(*this);
	}
	std::string to_string() const
	{
	    return "not ("+filter_->to_string()+")";
	}
	 
	~logical_not() 
	{
	    delete filter_;
	}
    private:
	filter<FeatureT>* filter_;
    };
}
 
#endif //LOGICAL_HPP
