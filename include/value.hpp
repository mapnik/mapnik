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

#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>
#include <sstream>
#include <boost/variant.hpp>

using std::string;
using namespace boost;
namespace mapnik { namespace impl {

    typedef variant<int,double,string> value_holder;
    
    class equals
	: public boost::static_visitor<bool>
    {
    public:
	template <typename T, typename U>
	bool operator()( const T &, const U & ) const
	{
	    return false;
	}
	
	bool operator() (int lhs, int rhs) const
	{
	    return  lhs == rhs;
	}
	
	bool operator() (double lhs, double rhs) const
	{
	    return  lhs == rhs;
	}

	bool operator() (int lhs, double rhs) const
	{
	    return  lhs == rhs;
	}
	
	bool operator() (double lhs, int rhs) const
	{
	    return  lhs == rhs;
	}
	
	template <typename T>
	bool operator()( const T & lhs, const T & rhs ) const
	{
	    return lhs == rhs;
	}
    };
    
    class greater_than
	: public boost::static_visitor<bool>
    {
    public:	
	template <typename T, typename U>
	bool operator()( const T &, const U & ) const
	{
	    return false;
	}
	
	bool operator() (int lhs, int rhs) const
	{
	    return  lhs > rhs;
	}
	
	bool operator() (double lhs, double rhs) const
	{
	    return  lhs > rhs;
	}
	
	bool operator() (int lhs, double rhs) const
	{
	    return  lhs > rhs;
	}
	
	bool operator() (double lhs, int rhs) const
	{
	    return  lhs > rhs;
	}
	
	template <typename T>
	bool operator()( const T & lhs, const T & rhs ) const
	{
	    return lhs > rhs;
	}
    };
    class greater_or_equal
	: public boost::static_visitor<bool>
    {
    public:	
	template <typename T, typename U>
	bool operator()( const T &, const U & ) const
	{
	    return false;
	}
	
	bool operator() (int lhs, int rhs) const
	{
	    return  lhs >= rhs;
	}
	
	bool operator() (double lhs, double rhs) const
	{
	    return  lhs >= rhs;
	}
	
	bool operator() (int lhs, double rhs) const
	{
	    return  lhs >= rhs;
	}
	
	bool operator() (double lhs, int rhs) const
	{
	    return  lhs >= rhs;
	}
	
	template <typename T>
	bool operator()( const T & lhs, const T & rhs ) const
	{
	    return lhs >= rhs;
	}
    };
    
    class less_than
	: public boost::static_visitor<bool>
    {
    public:	
	template <typename T, typename U>
	bool operator()( const T &, const U & ) const
	{
	    return false;
	}
	
	bool operator() (int lhs, int rhs) const
	{
	    return  lhs < rhs;
	}
	
	bool operator() (double lhs, double rhs) const
	{
	    return  lhs < rhs;
	}
	
	bool operator() (int lhs, double rhs) const
	{
	    return  lhs < rhs;
	}
	
	bool operator() (double lhs, int rhs) const
	{
	    return  lhs < rhs;
	}
	
	template <typename T>
	bool operator()( const T & lhs, const T & rhs ) const
	{
	    return lhs < rhs;
	}
    };

    class less_or_equal
	: public boost::static_visitor<bool>
    {
    public:	
	template <typename T, typename U>
	bool operator()( const T &, const U & ) const
	{
	    return false;
	}
	
	bool operator() (int lhs, int rhs) const
	{
	    return  lhs <= rhs;
	}
	
	bool operator() (double lhs, double rhs) const
	{
	    return  lhs <= rhs;
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
	bool operator()( const T & lhs, const T & rhs ) const
	{
	    return lhs <= rhs;
	}
    };
    
    struct add : public boost::static_visitor<value_holder>
    { 
	template <typename T1, typename T2>
	value_holder operator() (T1 const& , T2 const&) const
	{
	    return value_holder();
	}
	template <typename T>
	value_holder operator() (T const& lhs, T const&  rhs) const
	{
	    return lhs + rhs ;
	}
	value_holder operator() (int lhs,int rhs) const
	{
	    return lhs + rhs;
	}
	value_holder operator() (double lhs, double rhs) const
	{
	    return lhs + rhs;
	}
	
	value_holder operator() (double lhs, int rhs) const
	{
	    return lhs + rhs;
	}
	
	value_holder operator() (int lhs, double rhs) const
	{
	    return lhs + rhs;
	}
    };
    
    struct sub : public boost::static_visitor<value_holder>
    { 
	template <typename T1, typename T2>
	value_holder operator() (T1 const& lhs, T2 const&) const
	{
	    return value_holder(lhs);
	}

	template <typename T>
	value_holder operator() (T const& lhs, T const&  rhs) const
	{
	    return lhs - rhs ;
	}

	value_holder operator() (string const& lhs,string const& ) const
	{
	    return lhs;
	}

	value_holder operator() (int lhs,int rhs) const
	{
	    return lhs - rhs;
	}

	value_holder operator() (double lhs, double rhs) const
	{
	    return lhs - rhs;
	}
	
	value_holder operator() (double lhs, int rhs) const
	{
	    return lhs - rhs;
	}
	
	value_holder operator() (int lhs, double rhs) const
	{
	    return lhs - rhs;
	}
    };
    
    struct mult : public boost::static_visitor<value_holder>
    { 
	template <typename T1, typename T2>
	value_holder operator() (T1 const&, T2 const& ) const
	{
	    return value_holder();
	}
	template <typename T>
	value_holder operator() (T const& lhs, T const&  rhs) const
	{
	    return lhs * rhs;
	}
	
	value_holder operator() (string const& lhs,string const& ) const
	{
	    return lhs;
	}

	value_holder operator() (int lhs,int rhs) const
	{
	    return lhs * rhs;
	}

	value_holder operator() (double lhs, double rhs) const
	{
	    return lhs * rhs;
	}
	
	value_holder operator() (double lhs, int rhs) const
	{
	    return lhs * rhs;
	}
	
	value_holder operator() (int lhs, double rhs) const
	{
	    return lhs * rhs;
	}
    };

    struct div: public boost::static_visitor<value_holder>
    { 
	template <typename T1, typename T2>
	value_holder operator() (T1 const&, T2 const&) const
	{
	    return value_holder();
	}
	template <typename T>
	value_holder operator() (T const& lhs, T const&  rhs) const
	{
	    return lhs / rhs;
	}
	
	value_holder operator() (string const& lhs,string const&) const
	{
	    return lhs;
	}

	value_holder operator() (int lhs,int rhs) const
	{
	    return lhs / rhs;
	}

	value_holder operator() (double lhs, double rhs) const
	{
	    return lhs / rhs;
	}
	
	value_holder operator() (double lhs, int rhs) const
	{
	    return lhs / rhs;
	}
	
	value_holder operator() (int lhs, double rhs) const
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
	std::string operator() (std::string const& val) const
	{
	    return "'" + val + "'";
	}
    };
}
    
    using namespace impl;

    class value 
    {
    public:
	value(int i)
	    : v_(i) {}
	
	value(double d)
	    : v_(d) {}
	
	value(string const& str)
	    : v_(str) {}
	
	value(value const& other)
	    : v_ (other.v_) {}
	
	bool operator==(value const& other) const
	{
	    return apply_visitor(equals(),v_,other.get());
	}

	bool operator!=(value const& other) const
	{
	    return !(apply_visitor(equals(),v_,other.get()));
	}
	
	bool operator>(value const& other) const
	{
	    return apply_visitor(greater_than(),v_,other.get());
	}

	bool operator>=(value const& other) const
	{
	    return apply_visitor(greater_or_equal(),v_,other.get());
	}

	bool operator<(value const& other) const
	{
	    return apply_visitor(less_than(),v_,other.get());
	}

	bool operator<=(value const& other) const
	{
	    return apply_visitor(less_or_equal(),v_,other.get());
	}

	value& operator+=(value const& other)
	{
	    v_ = apply_visitor(add(),v_,other.get());
	    return *this;
	}

	value& operator-=(value const& other)
	{
	    v_ = apply_visitor(sub(),v_,other.get());
	    return *this;
	}

	value& operator*=(value const& other)
	{
	    v_ = apply_visitor(mult(),v_,other.get());
	    return *this;
	}
	
	value& operator/=(value const& other)
	{
	    v_ = apply_visitor(div(),v_,other.get());
	    return *this;
	}
	
	value_holder const& get() const
	{
	    return v_;
	}

	string to_string() const
	{
	    return apply_visitor(impl::to_string(),v_);
	}
     
    private:
	value_holder v_;
    };
    
    inline const value operator+(value const& p1,value const& p2)
    {
	value tmp(p1);
	tmp+=p2;
	return tmp;
    }

    inline const value operator-(value const& p1,value const& p2)
    {
	value tmp(p1);
	tmp-=p2;
	return tmp;
    }

    inline const value operator*(value const& p1,value const& p2)
    {
	value tmp(p1);
	tmp*=p2;
	return tmp;
    }

    inline const value operator/(value const& p1,value const& p2)
    {
	value tmp(p1);
	tmp/=p2;
	return tmp;
    }

    template <typename charT, typename traits>
    inline std::basic_ostream<charT,traits>& 
    operator << (std::basic_ostream<charT,traits>& out,
		 value const& v)
    {
	out << v.get();
	return out; 
    }
}

#endif //VALUE_HPP
