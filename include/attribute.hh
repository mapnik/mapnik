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

#ifndef ATTRIBUTE_HH
#define ATTRIBUTE_HH

#include <sstream>
#include <typeinfo>
#include <map>

namespace mapnik
{
    template <typename T>
    struct attribute_traits
    {
	static std::string to_string(const T& value)
	{
	    std::stringstream ss;
	    ss << value;
	    return ss.str();
	}
    };

    template <>
    struct attribute_traits<std::string>
    {
	static std::string to_string(const std::string& value)
	{
	    return value;
	}
    };
    
    class attribute
    {	
    public:
	attribute()
	    : base_(0) {}

	template <typename T>
	attribute(const T& value)
	    : base_(new attribute_impl<T>(value)) 
	{}

	attribute(const attribute& rhs)
	    : base_(rhs.base_ ? rhs.base_->clone() : 0)
	{}

	~attribute() 
	{
	    delete base_;
	}

	template<typename T>
	attribute& operator=(const T& rhs)
	{
	    attribute(rhs).swap(*this);
	    return *this;
	}

	attribute& operator=(const attribute& rhs)
	{
	    attribute(rhs).swap(*this);
	    return *this;
	}

	bool empty() const
	{
	    return !base_;
	}

	const std::type_info & type() const
        {
            return base_ ? base_->type() : typeid(void);
        }

	const std::string to_string() const
	{
	    return base_ ? base_->to_string() : "";
	}
    private:
	attribute& swap(attribute& rhs)
	{
	    std::swap(base_,rhs.base_);
	    return *this;
	}
	
	struct attribute_base
	{
	    virtual ~attribute_base() {}
	    virtual attribute_base* clone() const=0;
	    virtual std::string to_string() const=0;
	    virtual const std::type_info& type()  const=0;
	};

	template <typename T,typename ATraits=attribute_traits<T> >
	class attribute_impl : public attribute_base
	{	    
	public:
	    typedef T value_type;
	    attribute_impl(const value_type& value)
		: value_(value) {}
	    
	    std::string to_string() const
	    {
		return  ATraits::to_string(value_);
	    }
	    
	    attribute_base* clone() const
	    {
		return new attribute_impl(value_);
	    }
	    const std::type_info& type() const 
	    {
		return typeid(value_);
	    }
	    value_type value_;
	};
	template<typename value_type>
	friend value_type* attribute_cast(attribute*);
    public:
	attribute_base* base_;
    };
    
    
    template<typename T>
    struct bad_attribute_cast : public std::bad_cast
    {
	virtual const char* what() const throw()
	{
	    return "attribute::failed conversion";
	}
    };
    
    template <typename T> 
    bool is_type(const attribute& attr)
    {
	return attr.type()==typeid(T);
    }
    
    template <typename T>
    T* attribute_cast(attribute* attr)
    {
	return attr && attr->type() == typeid(T)
	    ? &static_cast<attribute::attribute_impl<T>*>(attr->base_)->value_
	    : 0;
    }

    template <typename T>
    T* attribute_cast(const attribute* attr)
    {
	return attribute_cast<T>(const_cast<attribute*>(attr));
    }
    
    template <typename T>
    T attribute_cast(const attribute& attr)
    {
	const T* result=attribute_cast<T>(&attr);
	if (!result)
	    throw bad_attribute_cast<T>();
	return *result;
    }
    
    template <typename T>
    T attribute_cast(attribute& attr)
    {
	T* result=attribute_cast<T>(&attr);
	if (!result)
	    throw bad_attribute_cast<T>();
	return *result;
    }
    
    typedef std::map<std::string,attribute> attributes; 
    
    template <typename T>
    attribute attribute_from_string(const std::string& val)
    {
	std::istringstream is(val);
	T t;
	is >> t;
	return attribute(t);
    }

    template <typename charT, typename traits>
    inline std::basic_ostream<charT,traits>& 
    operator << (std::basic_ostream<charT,traits>& out,
		 const attribute& attr)
    {
	out << attr.to_string();
	return out; 
    }
}

#endif                                            //ATTRIBUTE_HH
