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

#ifndef VALUE_HH
#define VALUE_HH

#include <string>
#include <sstream>
#include "attribute.hh"
#include "expression.hh"

using std::string;

namespace mapnik
{   
    class value 
    {
	attribute data;
    public:	
	value()
	    : data(0) {}
	value(int value)
	    : data(value) {}
	value(double value)
	    : data(value) {}
	value(string const& value)
	    : data(value) {}
	value(attribute const& attr)
	    : data(attr) {}
	value(value const& other)
	    : data(other.data) {}

	~value() {}
        
	bool is_int() const
	{
	    return data.type() == typeid(int);
	}
	bool is_real() const 
	{
	    return data.type() == typeid(double);
	}
	bool is_string() const
	{
	    return data.type() == typeid(std::string);
	}
	
	int as_int() const
	{
	    return attribute_cast<int>(data);
	}
	
	double as_real() const
	{
	    return attribute_cast<double>(data);
	}
	
	string as_string() const
	{
	    return attribute_cast<string>(data);
	}
	
	string to_string() const
	{
	    return data.to_string();
	}
	
	bool operator<=(value const& other) const
	{
	    if (is_int())
	    {
		if (other.is_int())
		{
		    return  as_int() <= other.as_int();
		}
		else if (other.is_real())
		{
		    return as_int() <= other.as_real();
		}
		else 
		{
		    return to_string() <= other.as_string();
		}
	    }
	    else if (is_real())
	    {
		if (other.is_int())
		{
		    return as_real() <= other.as_int();
		}
		else if (other.is_real())
		{
		    return as_real() <= other.as_real();
		}
		else 
		{
		    return to_string() <= other.as_string();
		}
	    }
	    else 
	    {
		if (other.is_int())
		{
		    return as_string() <= other.to_string();
		}
		else if (other.is_real())
		{
		    return as_string() <= other.to_string();
		}
		else 
		{
		    return as_string() <= other.as_string();
		}
	    }
	}
	
	bool operator<(value const& other) const
	{
	    if (is_int())
	    {
		if (other.is_int())
		{
		    return  as_int() < other.as_int();
		}
		else if (other.is_real())
		{
		    return as_int() < other.as_real();
		}
		else 
		{
		    return to_string() < other.as_string();
		}
	    }
	    else if (is_real())
	    {
		if (other.is_int())
		{
		    return as_real() < other.as_int();
		}
		else if (other.is_real())
		{
		    return as_real() < other.as_real();
		}
		else 
		{
		    return to_string() < other.as_string();
		}
	    }
	    else 
	    {
		if (other.is_int())
		{
		    return as_string() < other.to_string();
		}
		else if (other.is_real())
		{
		    return as_string() < other.to_string();
		}
		else 
		{
		    return as_string() < other.as_string();
		}
	    }
	}
	
	bool operator>=(value const& other) const
	{
	    if (is_int())
	    {
		if (other.is_int())
		{
		    return  as_int() >= other.as_int();
		}
		else if (other.is_real())
		{
		    return as_int() >= other.as_real();
		}
		else 
		{
		    return to_string() >= other.as_string();
		}
	    }
	    else if (is_real())
	    {
		if (other.is_int())
		{
		    return as_real() >= other.as_int();
		}
		else if (other.is_real())
		{
		    return as_real() >= other.as_real();
		}
		else 
		{
		    return to_string() >= other.as_string();
		}
	    }
	    else 
	    {
		if (other.is_int())
		{
		    return as_string() >= other.to_string();
		}
		else if (other.is_real())
		{
		    return as_string() >= other.to_string();
		}
		else 
		{
		    return as_string() >= other.as_string();
		}
	    }
	}
	
	bool operator>(value const& other) const
	{
	    if (is_int())
	    {
		if (other.is_int())
		{
		    return  as_int() > other.as_int();
		}
		else if (other.is_real())
		{
		    return as_int() > other.as_real();
		}
		else 
		{
		    return to_string() > other.as_string();
		}
	    }
	    else if (is_real())
	    {
		if (other.is_int())
		{
		    return as_real() > other.as_int();
		}
		else if (other.is_real())
		{
		    return as_real() > other.as_real();
		}
		else 
		{
		    return to_string() > other.as_string();
		}
	    }
	    else 
	    {
		if (other.is_int())
		{
		    return as_string() > other.to_string();
		}
		else if (other.is_real())
		{
		    return as_string() > other.to_string();
		}
		else 
		{
		    return as_string() > other.as_string();
		}
	    }
	}

	bool operator!=(value const& other) const
	{
	    if (is_int())
	    {
		if (other.is_int())
		{
		    return  as_int() != other.as_int();
		}
		else if (other.is_real())
		{
		    return as_int() != other.as_real();
		}
		else 
		{
		    return to_string() != other.as_string();
		}
	    }
	    else if (is_real())
	    {
		if (other.is_int())
		{
		    return as_real() != other.as_int();
		}
		else if (other.is_real())
		{
		    return as_real() != other.as_real();
		}
		else 
		{
		    return to_string() != other.as_string();
		}
	    }
	    else 
	    {
		if (other.is_int())
		{
		    return as_string() != other.to_string();
		}
		else if (other.is_real())
		{
		    return as_string() != other.to_string();
		}
		else 
		{
		    return as_string() != other.as_string();
		}
	    }
	}
	
	bool operator==(value const& other) const
	{
	    if (is_int())
	    {
		if (other.is_int())
		{
		    return  as_int() ==  other.as_int();
		}
		else if (other.is_real())
		{
		    return as_int() == other.as_real();
		}
		else 
		{
		    return to_string() == other.as_string();
		}
	    }
	    else if (is_real())
	    {
		if (other.is_int())
		{
		    return as_real() == other.as_int();
		}
		else if (other.is_real())
		{
		    return as_real() == other.as_real();
		}
		else 
		{
		    return to_string() == other.as_string();
		}
	    }
	    else 
	    {
		if (other.is_int())
		{
		    return as_string() == other.to_string();
		}
		else if (other.is_real())
		{
		    return as_string() == other.to_string();
		}
		else 
		{
		    return as_string() == other.as_string();
		}
	    }
	}

	
	value operator-() const
	{
	    if (is_int())
	    {
		return -(as_int());
	    }
	    else if (is_real())
	    {
		return -(as_real());
	    }
	    return value();
	}
	
	value& operator+=(value const& other)
	{
	    if (is_int())
	    {
		if (other.is_int())
		{
		    data = as_int() + other.as_int();
		}
		else if (other.is_real())
		{
		    data = as_int() + other.as_real();
		}
		else 
		{
		    data = to_string() + other.as_string();
		}
	    }
	    else if (is_real())
	    {
		if (other.is_int())
		{
		    data = as_real() + other.as_int();
		}
		else if (other.is_real())
		{
		    data = as_real() + other.as_real();
		}
		else 
		{
		    data = to_string() + other.as_string();
		}
	    }
	    else 
	    {
		if (other.is_int())
		{
		    data = as_string() + other.to_string();
		}
		else if (other.is_real())
		{
		    data = as_string() + other.to_string();
		}
		else 
		{
		    data = as_string() + other.as_string();
		}
	    }
	    return *this;
	} 

	value&  operator-=(value const& other)
	{
	    if (is_int())
	    {
		if (other.is_int())
		{
		    data = as_int() - other.as_int();
		}
		else if (other.is_real())
		{
		    data = as_int() - other.as_real();
		}
	    }
	    else if (is_real())
	    {
		if (other.is_int())
		{
		    data = as_real() - other.as_int();
		}
		else if (other.is_real())
		{
		    data = as_real() - other.as_real();
		}	
	    }
	    
	    return *this;
	} 

	value&  operator*=(value const& other)
	{
	    if (is_int())
	    {
		if (other.is_int())
		{
		    data = as_int() * other.as_int();
		}
		else if (other.is_real())
		{
		    data = as_int() * other.as_real();
		}
	    }
	    else if (is_real())
	    {
		if (other.is_int())
		{
		    data = as_real() * other.as_int();
		}
		else if (other.is_real())
		{
		    data = as_real() * other.as_real();
		}	
	    }
	    
	    return *this;
	} 
	value&  operator/=(value const& other)
	{
	    if (is_int())
	    {
		if (other.is_int())
		{
		    data = as_int() / other.as_int();
		}
		else if (other.is_real())
		{
		    data = as_int() / other.as_real();
		}
	    }
	    else if (is_real())
	    {
		if (other.is_int())
		{
		    data = as_real() / other.as_int();
		}
		else if (other.is_real())
		{
		    data = as_real() / other.as_real();
		}	
	    }
	    
	    return *this;
	} 
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
		 const value& p)
    {
	out << p.to_string();
	return out; 
    }
}

#endif //VALUE_HH
