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

#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>
#include <sstream>
#include <boost/variant.hpp>

using namespace boost;
namespace mapnik {

    typedef variant<int,double,std::string> value_base;
    
    namespace impl {
	struct equals
	    : public boost::static_visitor<bool>
	{
	    template <typename T, typename U>
	    bool operator() (const T &, const U & ) const
	    {
		return false;
	    }
	
	    template <typename T>
	    bool operator() (T lhs, T rhs) const
	    {
		return lhs == rhs;
	    }
	
	    bool operator() (int lhs, double rhs) const
	    {
		return  lhs == rhs;
	    }
	
	    bool operator() (double lhs, int rhs) const
	    {
		return  lhs == rhs;
	    }
	
	    bool operator() (std::string const& lhs, std::string const& rhs) const
	    {
		return  lhs == rhs;
	    }
	};
    
	struct greater_than
	    : public boost::static_visitor<bool>
	{
	    template <typename T, typename U>
	    bool operator()( const T &, const U & ) const
	    {
		return false;
	    }
	
	    template <typename T>
	    bool operator()( T lhs, T rhs ) const
	    {
		return lhs > rhs;
	    }
	
	    bool operator() (int lhs, double rhs) const
	    {
		return  lhs > rhs;
	    }
	
	    bool operator() (double lhs, int rhs) const
	    {
		return  lhs > rhs;
	    }
	
	    bool operator() (std::string const& lhs, std::string const& rhs) const
	    {
		return  lhs > rhs;
	    }
	};
    
	struct greater_or_equal
	    : public boost::static_visitor<bool>
	{	
	    template <typename T, typename U>
	    bool operator()( const T &, const U & ) const
	    {
		return false;
	    }
	
	    template <typename T>
	    bool operator() (T lhs, T rhs) const
	    {
		return lhs >= rhs;
	    }
      
	    bool operator() (int lhs, double rhs) const
	    {
		return  lhs >= rhs;
	    }
	
	    bool operator() (double lhs, int rhs) const
	    {
		return  lhs >= rhs;
	    }
	
	    bool operator() (std::string const& lhs, std::string const& rhs ) const
	    {
		return lhs >= rhs;
	    }
	};
    
	struct less_than
	    : public boost::static_visitor<bool>
	{	
	    template <typename T, typename U>
	    bool operator()( const T &, const U & ) const
	    {
		return false;
	    }
	
	    template <typename T>
	    bool operator()( T  lhs,T  rhs) const
	    {
		return lhs < rhs;
	    }
	
	    bool operator() (int lhs, double rhs) const
	    {
		return  lhs < rhs;
	    }
	   
	    bool operator() (double lhs, int rhs) const
	    {
		return  lhs < rhs;
	    }
	
	    bool operator()( std::string const& lhs, std::string const& rhs ) const
	    {
		return lhs < rhs;
	    }
	};

	struct less_or_equal
	    : public boost::static_visitor<bool>
	{	
	    template <typename T, typename U>
	    bool operator()( const T &, const U & ) const
	    {
		return false;
	    }
	
	    template <typename T>
	    bool operator()(T lhs, T rhs ) const
	    {
		return lhs <= rhs;
	    }
	    
	    bool operator() (int lhs, double rhs) const
	    {
		return  lhs <= rhs;
	    }
	
	    bool operator() (double lhs, int rhs) const
	    {
		return  lhs <= rhs;
	    }
	
	    template <typename T>
	    bool operator()( std::string const& lhs, std::string const& rhs ) const
	    {
		return lhs <= rhs;
	    }
	};
    
	template <typename V>
	struct add : public boost::static_visitor<V>
	{ 
	    typedef V value_type;
	    template <typename T1, typename T2>
	    value_type operator() (T1 const& lhs, T2 const&) const
	    {
		return lhs;
	    }
	    template <typename T>
	    value_type operator() (T lhs, T rhs) const
	    {
		return lhs + rhs ;
	    }
	
	    value_type operator() (std::string const& lhs,std::string const& rhs ) const
	    {
		return lhs + rhs;
	    }
	
	    value_type operator() (double lhs, int rhs) const
	    {
		return lhs + rhs;
	    }
	
	    value_type operator() (int lhs, double rhs) const
	    {
		return lhs + rhs;
	    }
	};
	template <typename V>
	struct sub : public boost::static_visitor<V>
	{ 
	    typedef V value_type;
	    template <typename T1, typename T2>
	    value_type operator() (T1 const& lhs, T2 const&) const
	    {
		return lhs;
	    }

	    template <typename T>
	    value_type operator() (T  lhs, T rhs) const
	    {
		return lhs - rhs ;
	    }

	    value_type operator() (std::string const& lhs,std::string const& ) const
	    {
		return lhs;
	    }
        	
	    value_type operator() (double lhs, int rhs) const
	    {
		return lhs - rhs;
	    }
	
	    value_type operator() (int lhs, double rhs) const
	    {
		return lhs - rhs;
	    }
	};
    
	template <typename V>
	struct mult : public boost::static_visitor<V>
	{ 
	    typedef V value_type;
	    template <typename T1, typename T2>
	    value_type operator() (T1 const& lhs , T2 const& ) const
	    {
		return lhs;
	    }
	    template <typename T>
	    value_type operator() (T lhs, T rhs) const
	    {
		return lhs * rhs;
	    }
	
	    value_type operator() (std::string const& lhs,std::string const& ) const
	    {
		return lhs;
	    }	
	
	    value_type operator() (double lhs, int rhs) const
	    {
		return lhs * rhs;
	    }
	
	    value_type operator() (int lhs, double rhs) const
	    {
		return lhs * rhs;
	    }
	};

	template <typename V>
	struct div: public boost::static_visitor<V>
	{ 
	    typedef V value_type;
	    template <typename T1, typename T2>
	    value_type operator() (T1 const& lhs, T2 const&) const
	    {
		return lhs;
	    }
	    
	    template <typename T>
	    value_type operator() (T lhs, T rhs) const
	    {
		return lhs / rhs;
	    }
	
	    value_type operator() (std::string const& lhs,std::string const&) const
	    {
		return lhs;
	    }
	
	    value_type operator() (double lhs, int rhs) const
	    {
		return lhs / rhs;
	    }
	
	    value_type operator() (int lhs, double rhs) const
	    {
		return lhs / rhs;
	    }
	};
    
	struct to_string : public boost::static_visitor<std::string>
	{
	    template <typename T>
	    std::string operator() (T val) const
	    {
		std::stringstream ss;
		ss << val;
		return ss.str();
	    } 
	    std::string const& operator() (std::string const& val) const
	    {
		return val;
	    }
	};
	
	struct to_expression_string : public boost::static_visitor<std::string>
	{
	    template <typename T>
	    std::string operator() (T val) const
	    {
		std::stringstream ss;
		ss << val;
		return ss.str();
	    } 
	    std::string operator() (std::string const& val) const
	    {
		return "'" + val + "'";
	    }
	};
    }
    
    class value
    {
        value_base base_;
        friend const value operator+(value const&,value const&);
        friend const value operator-(value const&,value const&);
        friend const value operator*(value const&,value const&);
        friend const value operator/(value const&,value const&);

    public:
	value ()
	    : base_(0) {}
	
	template <typename T> value(T _val_)
	    : base_(_val_) {}

	bool operator==(value const& other) const
	{
	    return boost::apply_visitor(impl::equals(),base_,other.base_);
	}

	bool operator!=(value const& other) const
	{
	    return !(boost::apply_visitor(impl::equals(),base_,other.base_));
	}
	
	bool operator>(value const& other) const
	{
	    return boost::apply_visitor(impl::greater_than(),base_,other.base_);
	}

	bool operator>=(value const& other) const
	{
	    return boost::apply_visitor(impl::greater_or_equal(),base_,other.base_);
	}

	bool operator<(value const& other) const
	{
	    return boost::apply_visitor(impl::less_than(),base_,other.base_);
	}

	bool operator<=(value const& other) const
	{
	    return boost::apply_visitor(impl::less_or_equal(),base_,other.base_);
	}
	value_base const& base() const
	{
	    return base_;
	}

	std::string to_expression_string() const
	{
	    return boost::apply_visitor(impl::to_expression_string(),base_);
	}

	std::string to_string() const
	{
	    return boost::apply_visitor(impl::to_string(),base_);
	}
    };
    
    inline const value operator+(value const& p1,value const& p2)
    {

	return value(boost::apply_visitor(impl::add<value>(),p1.base_, p2.base_));
    }

    inline const value operator-(value const& p1,value const& p2)
    {

	return value(boost::apply_visitor(impl::sub<value>(),p1.base_, p2.base_));
    }

    inline const value operator*(value const& p1,value const& p2)
    {

	return value(boost::apply_visitor(impl::mult<value>(),p1.base_, p2.base_));
    }

    inline const value operator/(value const& p1,value const& p2)
    {

	return value(boost::apply_visitor(impl::div<value>(),p1.base_, p2.base_));
    }

    template <typename charT, typename traits>
    inline std::basic_ostream<charT,traits>& 
    operator << (std::basic_ostream<charT,traits>& out,
    		 value const& v)
    {
    	out << v.base();
    	return out; 
    }
}

#endif //VALUE_HPP
