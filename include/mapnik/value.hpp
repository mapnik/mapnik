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

// mapnik
#include <mapnik/unicode.hpp>
#include <mapnik/config_error.hpp>
// boost
#include <boost/variant.hpp>
// stl
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
// uci
#include <unicode/unistr.h>

namespace mapnik  {
   
   typedef boost::variant<bool,int,double,UnicodeString> value_base;
   
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
	
            bool operator() (UnicodeString const& lhs, 
                             UnicodeString const& rhs) const
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
	
            bool operator() (UnicodeString const& lhs, UnicodeString const& rhs) const
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
	
            bool operator() (UnicodeString const& lhs, UnicodeString const& rhs ) const
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
	
            bool operator()( UnicodeString const& lhs, 
                             UnicodeString const& rhs ) const
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
            bool operator()( UnicodeString const& lhs, 
                             UnicodeString const& rhs ) const
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
	
            value_type operator() (UnicodeString const& lhs , 
                                   UnicodeString const& rhs ) const
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

            value_type operator() (UnicodeString const& lhs,
                                   UnicodeString const& ) const
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
	
            value_type operator() (UnicodeString const& lhs,
                                   UnicodeString const& ) const
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
	
            value_type operator() (UnicodeString const& lhs,
                                   UnicodeString const&) const
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
        
      struct to_bool : public boost::static_visitor<bool>
      {
                
            template <typename T>
            bool operator() (T val) const
	    {
               throw config_error("Boolean value expected");
	    }

            bool operator() (bool val) const
            {
	       return val;
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
            // specializations 
            std::string operator() (UnicodeString const& val) const
	    {
               //std::stringstream ss;
               //std::wstring::const_iterator pos = val.begin();
               //ss << std::hex ;
               //for (;pos!=val.end();++pos)
               //{
               //  wchar_t c = *pos;
               //  if (c < 0x80) 
               //  {
               //     ss << char(c);
               //  }
               //  else
               //  {
               //     ss << "\\x";
               //     unsigned c0 = (c >> 8) & 0xff;
               //     if (c0) ss << c0;
               //     ss << (c & 0xff);
               //  }
               //}
	       //return ss.str();
               return "TODO";
	    }
            
            std::string operator() (double val) const
            {
               std::stringstream ss;
	       ss << std::setprecision(16) << val;
	       return ss.str();
            }
      };

      struct to_unicode : public boost::static_visitor<UnicodeString>
      {
                
            template <typename T>
            UnicodeString operator() (T val) const
	    {
               std::basic_ostringstream<char> out;
	       out << val;
               return UnicodeString(out.str().c_str());
	    }

            // specializations 
            UnicodeString const& operator() (UnicodeString const& val) const
	    {
               return val;
	    }

            UnicodeString operator() (double val) const
            {
               std::basic_ostringstream<char> out;
	       out << std::setprecision(16) << val;
               return UnicodeString(out.str().c_str());
            }
      };
      
      struct to_expression_string : public boost::static_visitor<std::string>
      {
            std::string operator() (UnicodeString const& val) const
	    {
               /*
               std::stringstream ss;
               UnicodeString::const_iterator pos = val.begin();
               ss << std::hex ;
               for (;pos!=val.end();++pos)
               {
                  wchar_t c = *pos;
                  if (c < 0x80) 
                  {
                     ss << char(c);
                  }
                  else
                  {
                     ss << "\\x";
                     unsigned c0 = (c >> 8) & 0xff;
                     if (c0) ss << c0;
                     ss << (c & 0xff);
                  }
               }
	       return "\'" + ss.str() + "\'";
               */
               return "TODO";
	    } 
            
            template <typename T>
            std::string operator() (T val) const
	    {
	       std::stringstream ss;
	       ss << val;
	       return ss.str();
	    }
            
            std::string operator() (double val) const
            {
               std::stringstream ss;
	       ss << std::setprecision(16) << val;
	       return ss.str();
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

	 bool to_bool() const
	 {
	    return boost::apply_visitor(impl::to_bool(),base_);
	 }

	 std::string to_expression_string() const
	 {
	    return boost::apply_visitor(impl::to_expression_string(),base_);
	 }

	 std::string to_string() const
	 {
	    return boost::apply_visitor(impl::to_string(),base_);
	 }
         
         UnicodeString to_unicode() const
         {
            return boost::apply_visitor(impl::to_unicode(),base_);
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
    out << v.to_string();
    return out; 
  }
}

#endif //VALUE_HPP
