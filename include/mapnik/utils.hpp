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

//$Id: utils.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef UTILS_HPP
#define UTILS_HPP
// stl
#include <stdexcept>
#include <limits>
#include <ctime>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
// boost
#include <boost/thread/mutex.hpp>

namespace mapnik
{
    using boost::mutex;
    
    template <typename T>
    class CreateUsingNew
    {
    public:
        static T* create()
        {
            return new T;
        }
        static void destroy(T* obj)
        {
            delete obj;
        }
    };

    template <typename T>
    class CreateStatic
    {
    private:
        union MaxAlign
        {
            char t_[sizeof(T)];
            short int shortInt_;
            int int_;
            long int longInt_;
            float float_;
            double double_;
            long double longDouble_;
            struct Test;
            int Test::* pMember_;
            int (Test::*pMemberFn_)(int);
        };
    public:
        static T* create()
        {
            static MaxAlign staticMemory;
            return new(&staticMemory) T;
        }
        static void destroy(volatile T* obj)
        {
            obj->~T();
        }
    };

    template <typename T,
              template <typename T> class CreatePolicy=CreateStatic> class singleton
              {
                  friend class CreatePolicy<T>;
                  static T* pInstance_;
                  static bool destroyed_;
                  singleton(const singleton &rhs);
                  singleton& operator=(const singleton&);
                  static void onDeadReference()
                  {
                      throw std::runtime_error("dead reference!");
                  }
              protected:
                  static mutex mutex_;
                  singleton() {}
                  virtual ~singleton()
                  {
                      CreatePolicy<T>::destroy(pInstance_);
                      destroyed_=true;
                  }
              public:
                  static  T* instance()
                  {
                      if (!pInstance_)
                      {
                          mutex::scoped_lock lock(mutex_);
                          if (!pInstance_)
                          {
                              if (destroyed_)
                              {
                                  onDeadReference();
                              }
                              else
                              {
                                  pInstance_=CreatePolicy<T>::create();
                              }
                          }
                      }
                      return pInstance_;
                  }
              };

    template <typename T,
              template <typename T> class CreatePolicy> mutex singleton<T,CreatePolicy>::mutex_;
    template <typename T,
              template <typename T> class CreatePolicy> T* singleton<T,CreatePolicy>::pInstance_=0;
    template <typename T,
              template <typename T> class CreatePolicy> bool singleton<T,CreatePolicy>::destroyed_=false;

    template <class T> class Handle
    {
        T* ptr_;
        int* pCount_;
    public:
        T* operator->() {return ptr_;}
        const T* operator->() const {return ptr_;}
        Handle(T* ptr)
            :ptr_(ptr),pCount_(new int(1)) {}
        Handle(const Handle& rhs)
            :ptr_(rhs.ptr_),pCount_(rhs.pCount_)
        {
            (*pCount_)++;
        }
        Handle& operator=(const Handle& rhs)
        {
            if (ptr_==rhs.ptr_) return *this;
            if (--(*pCount_)==0)
            {
                delete ptr_;
                delete pCount_;
            }
            ptr_=rhs.ptr_;
            pCount_=rhs.pCount_;
            (*pCount_)++;
            return *this;
        }
        ~Handle()
        {
            if (--(*pCount_)==0)
            {
                delete ptr_;
                delete pCount_;
            }
        }
    };
    
    //converters
    class BadConversion : public std::runtime_error
    {
    public:
        BadConversion(const std::string& s)
            :std::runtime_error(s)
        {}
    };
    
    template <typename T>
    inline std::string toString(const T& x)
    {
        std::ostringstream o;
        if (!(o << x))
            throw BadConversion(std::string("toString(")
                                + typeid(x).name() + ")");
        return o.str();
    }
    
    template<typename T>
    inline void fromString(const std::string& s, T& x,
                           bool failIfLeftoverChars = true)
    {
        std::istringstream i(s);
        char c;
        if (!(i >> x) || (failIfLeftoverChars && i.get(c)))
            throw BadConversion("fromString("+s+")");
    }
}


#endif //UTILS_HPP
