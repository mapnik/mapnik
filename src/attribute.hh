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

namespace mapnik
{

    class attribute_base
    {
    public:
	virtual std::string to_string() const=0;
	virtual attribute_base* clone() const=0;
	virtual ~attribute_base() {}
    };

    template <typename T>
    struct attribute_traits
    {
        static std::string to_string(const T& val)
        {
            std::stringstream ss;
            ss << val;
            return ss.str();
        }
        static T default_value()
        {
            return 0;
        }
    };

    template <>
    struct attribute_traits<std::string>
    {
        static std::string to_string(const std::string& s)
        {
            return s;
        }
        static std::string default_value()
        {
            return std::string();
        }
    };

    template <typename T,typename ATraits=attribute_traits<T> >
    class attribute : public attribute_base
    {
        typedef T value_type;
    private:
	value_type value_;
    public:
	attribute(const T& value)
	    : attribute_base(),
	      value_(value) {}
	attribute(const attribute& other)
	    : attribute_base(),
	      value_(other.value_) {}
	attribute& operator=(const attribute& other)
	{
	    attribute<T> temp(other);
	    Swap(temp);
	    return *this;
	}
	~attribute()
	{
	}
             
	attribute<T>* clone() const
	{
	    return new attribute<T>(*this);
	}
	value_type value() const
	{
	    return value_;
	}
	std::string to_string() const
	{
	    return ATraits::to_string(value_);
	}
    private:
	void Swap(attribute<T>& other) throw()
	{
	    std::swap(value_,other.value_);
	}
    };

    template <class charT,class traits>
    inline std::basic_ostream<charT,traits>&
    operator << (std::basic_ostream<charT,traits>& out,
		 const attribute_base& base)
    {
        std::basic_ostringstream<charT,traits> s;
        s.copyfmt(out);
        s.width(0);
        s<<base.to_string();
        out << s.str();
        return out;
    }

    
    class invalid_attribute : public attribute<std::string>
    {
    public:
	invalid_attribute()
	    :attribute<std::string>("#INVALID_ATTRIBUTE#") {}
    }; 

    template <typename T>
    attribute<T> attribute_from_string(const std::string& val)
    {
	std::istringstream is(val);
	T t;
	is >> t;
	return attribute<T>(t);
    }
}

#endif                                            //ATTRIBUTE_HH
