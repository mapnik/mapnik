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

using namespace boost;
namespace mapnik {

	typedef variant<std::string,int,double> value_base;
    
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
    
    class value : public value_base
    {
    public:
	value ()
	    : value_base(0) {}
	
	template <typename T> value(T _val_)
	    : value_base(_val_) {}
	
	value (const value& rhs)
	{
        //todo!!!!!!!!!
	}
	value& operator=(value const& rhs)
	{
		if (this == &rhs)
			return  *this;
		//TODO!!!!!	
		return *this;	
	}
	bool operator==(value const& other) const
	{
	    return boost::apply_visitor(impl::equals(),*this,other);
	}

	bool operator!=(value const& other) const
	{
	    return !(boost::apply_visitor(impl::equals(),*this,other));
	}
	
	bool operator>(value const& other) const
	{
	    return boost::apply_visitor(impl::greater_than(),*this,other);
	}

	bool operator>=(value const& other) const
	{
	    return boost::apply_visitor(impl::greater_or_equal(),*this,other);
	}

	bool operator<(value const& other) const
	{
	    return boost::apply_visitor(impl::less_than(),*this,other);
	}

	bool operator<=(value const& other) const
	{
	    return boost::apply_visitor(impl::less_or_equal(),*this,other);
	}

	value& operator+=(value const& other)
	{
	    *this = boost::apply_visitor(impl::add<value>(),*this,other);
	    return *this;
	}

	value& operator-=(value const& other)
	{
	    *this = boost::apply_visitor(impl::sub<value>(),*this,other);
	    return *this;
	}

	value& operator*=(value const& other)
	{
	    *this = boost::apply_visitor(impl::mult<value>(),*this,other);
	    return *this;
	}
	
	value& operator/=(value const& other)
	{
	    *this = boost::apply_visitor(impl::div<value>(),*this,other);
	    return *this;
	}

	std::string to_expression_string() const
	{
	    return boost::apply_visitor(impl::to_expression_string(),*this);
	}

	std::string to_string() const
	{
	    return boost::apply_visitor(impl::to_string(),*this);
	}
    };
    
    inline const value operator+(value const& p1,value const& p2)
    {
	//value tmp(p1);
	//tmp+=p2;
	//return tmp;
		return boost::apply_visitor(impl::add<value>(),p1, p2);
    }

    inline const value operator-(value const& p1,value const& p2)
    {
	//value tmp(p1);
	//tmp-=p2;
	//return tmp;
		return boost::apply_visitor(impl::sub<value>(),p1, p2);
    }

    inline const value operator*(value const& p1,value const& p2)
    {
	//value tmp(p1);
	//tmp*=p2;
	//return tmp;
		return boost::apply_visitor(impl::mult<value>(),p1, p2);
    }

    inline const value operator/(value const& p1,value const& p2)
    {
	//value tmp(p1);
	//tmp/=p2;
	//return tmp;
	    return boost::apply_visitor(impl::div<value>(),p1, p2);
	}

    //template <typename charT, typename traits>
    //inline std::basic_ostream<charT,traits>& 
    //operator << (std::basic_ostream<charT,traits>& out,/
    //		 value const& v)
    // {
    //	out << v.get();
    //	return out; 
    //}
}

#endif //VALUE_HPP
