/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_UTILS_HPP
#define MAPNIK_UTILS_HPP

#include <mapnik/config.hpp>

// stl
#include <stdexcept> // std::runtime_error
#include <cstdlib> // std::atexit
#include <new> // operator new

#ifdef MAPNIK_THREADSAFE
#include <mutex>
#endif

namespace mapnik
{

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


#ifdef __GNUC__
template <typename T,
          template <typename U> class CreatePolicy=CreateStatic> class MAPNIK_DECL singleton
{
#else
template <typename T,
          template <typename U> class CreatePolicy=CreateStatic> class singleton
{
#endif
    friend class CreatePolicy<T>;
    static T* pInstance_;
    static bool destroyed_;
    singleton(const singleton &rhs);
    singleton& operator=(const singleton&);

    static void onDeadReference()
    {
        throw std::runtime_error("dead reference!");
    }

    static void DestroySingleton()
    {
        CreatePolicy<T>::destroy(pInstance_);
        pInstance_ = 0;
        destroyed_ = true;
    }

protected:

#ifdef MAPNIK_THREADSAFE
        static std::mutex mutex_;
#endif
        singleton() {}
    public:
        static T& instance()
        {
            if (! pInstance_)
            {
#ifdef MAPNIK_THREADSAFE
                std::lock_guard<std::mutex> lock(mutex_);
#endif
                if (! pInstance_)
                {
                    if (destroyed_)
                    {
                        destroyed_ = false;
                        onDeadReference();
                    }
                    else
                    {
                        pInstance_ = CreatePolicy<T>::create();

                        // register destruction
                        std::atexit(&DestroySingleton);
                    }
                }
            }
            return *pInstance_;
        }
};

#ifdef MAPNIK_THREADSAFE
    template <typename T,
              template <typename U> class CreatePolicy> std::mutex singleton<T,CreatePolicy>::mutex_;
#endif

    template <typename T,
              template <typename U> class CreatePolicy> T* singleton<T,CreatePolicy>::pInstance_=0;
    template <typename T,
              template <typename U> class CreatePolicy> bool singleton<T,CreatePolicy>::destroyed_=false;


#ifdef _WINDOWS

// UTF8 <--> UTF16 conversion routines

MAPNIK_DECL std::string utf16_to_utf8(std::wstring const& wstr);
MAPNIK_DECL std::wstring utf8_to_utf16(std::string const& str);

#endif  // _WINDOWS

}

#endif // MAPNIK_UTILS_HPP
