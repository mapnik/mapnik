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

#ifndef COMPARISON_HH
#define COMPARISON_HH


#include "filter.hh"
#include "expression.hh"
#include "feature.hh"
#include "attribute.hh"

namespace mapnik
{
      
    template<typename Feature>
    struct property_filter : public filter<Feature> {
	const std::string name_;
	explicit property_filter(const std::string& name)
	    : name_(name) {}	
    };

    template <typename Feature,typename T>
    struct property_is_equal_to : public property_filter<Feature>
    {
	using property_filter<Feature>::name_;
	T value_;

	property_is_equal_to(const std::string& name,const T& value)
	    : property_filter<Feature>(name), value_(value) {}

	int type() const
	{
	    return filter<Feature>::COMPARISON_OPS;
	}
	
	bool pass(const Feature& feature) const
	{
	    attribute attr=feature.attribute_by_name(name_);
	    bool result=false;
	    try 
	    {
		result=(value_==attribute_cast<T>(attr))?true:false;
	    }
	    catch (bad_attribute_cast<T>& ex)
	    {
		std::cerr<<ex.what()<<std::endl;
	    }	
	    return result;
	}
	
	filter<Feature>* clone() const
	{
	    return new property_is_equal_to(name_,value_);
	}

	void accept(filter_visitor<Feature>& v)
	{
	    v.visit(*this);
	}

	virtual ~property_is_equal_to() {}
    };
    
    template <typename Feature,typename T>
    struct property_is_greater_then :  public property_filter<Feature>
    {   
	using property_filter<Feature>::name_;
	T value_;
	property_is_greater_then(const std::string& name,const T& value)
	    : property_filter<Feature>(name), value_(value) {}

	int type() const
	{
	    return filter<Feature>::COMPARISON_OPS;
	}
	
	bool pass(const Feature& feature) const
	{
	    attribute attr=feature.attribute_by_name(name_);
	    bool result=false;
	    try 
	    {
		result = (value_ > attribute_cast<T>(attr))?true:false;
	    }
	    catch (bad_attribute_cast<T>& ex)
	    {
		std::cerr<<ex.what()<<std::endl;
	    }	
	    return result;
	}
	filter<Feature>* clone() const
	{
	    return new property_is_greater_then(name_,value_);
	}
	void accept(filter_visitor<Feature>& v)
	{
	    v.visit(*this);
	}
	virtual ~property_is_greater_then() {}
    };
    
    template <typename Feature,typename T>
    struct property_is_less_then :  public property_filter<Feature>
    {
	using property_filter<Feature>::name_;
	T value_;
	property_is_less_then(const std::string& name,const T& value)
	    : property_filter<Feature>(name), value_(value) {}

	int type() const
	{
	    return filter<Feature>::COMPARISON_OPS;
	}
	
	bool pass(const Feature& feature) const
	{
	    attribute attr=feature.attribute_by_name(name_);
	    bool result=false;
	    try 
	    {
	    	result=(value_ > attribute_cast<T>(attr))?true:false;
	    }
	    catch (bad_attribute_cast<T>& ex)
	    {
		std::cerr<<ex.what()<<std::endl;
	    }	
	    return result; 
	}	
	void accept(filter_visitor<Feature>& v)
	{
	    v.visit(*this);
	}
    };
           
    template <typename Feature,typename T>
    struct property_is_between :  public property_filter<Feature>
    {
	using property_filter<Feature>::name_;
	T lo_value_;
	T hi_value_;
	
	property_is_between(const std::string& name,const T& lo_value,const T& hi_value)
	    : property_filter<Feature>(name), 
	      lo_value_(lo_value),
	      hi_value_(hi_value) {}

	int type() const
	{
	    return filter<Feature>::COMPARISON_OPS;
	}
	
	bool pass(const Feature& feature) const
	{
	    attribute attr=feature.attribute_by_name(name_);
	    bool result=false;
	    try 
	    {
		T const&  a=attribute_cast<T>(attr);
		result=(lo_value_ < a && a < hi_value_) ? true : false;
	    }
	    catch (bad_attribute_cast<T>& ex)
	    {
		std::cerr<<ex.what()<<std::endl;
	    }	
	    return result;
	}
	
	filter<Feature>* clone() const
	{
	    return new property_is_between(name_,lo_value_,hi_value_);
	}
	
	void accept(filter_visitor<Feature>& v)
	{
	    v.visit(*this);
	}
	
	virtual ~property_is_between() {}
    };    
}

#endif //COMPARISON_HH
