/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_UTIL_SINGLETON_HPP
#define MAPNIK_UTIL_SINGLETON_HPP

#include <mapnik/config.hpp>

// stl
#include <stdexcept> // std::runtime_error
#include <cstdlib>   // std::atexit
#include <new>       // operator new
#include <type_traits>
#include <atomic>

#ifdef MAPNIK_THREADSAFE
#include <mutex>
#endif

namespace mapnik {

template<typename T>
class CreateUsingNew
{
  public:
    static T* create() { return new T; }
    static void destroy(T* obj) { delete obj; }
};

template<typename T>
class CreateStatic
{
  private:
    using storage_type = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

  public:

    static T* create()
    {
        static storage_type static_memory;
        return new (&static_memory) T;
    }
    static void destroy(volatile T* obj) { obj->~T(); }
};

template<typename T>
class singleton_cxx11
{
public:
    static T& instance()
    {
        static T instance;
        return instance;
    }
protected:
#ifdef MAPNIK_THREADSAFE
    static std::mutex mutex_;
#endif
};


#ifdef __GNUC__
template<typename T, template<typename U> class CreatePolicy = CreateStatic>
class MAPNIK_DECL singleton
{
#else
template<typename T, template<typename U> class CreatePolicy = CreateStatic>
class singleton
{
#endif
    friend class CreatePolicy<T>;
    static std::atomic<T*> pInstance_;
    static std::atomic<bool> destroyed_;
    singleton(const singleton& rhs);
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
        T* tmp = pInstance_.load(std::memory_order_acquire);
        if (tmp == nullptr)
        {
#ifdef MAPNIK_THREADSAFE
            std::lock_guard<std::mutex> lock(mutex_);
#endif
            tmp = pInstance_.load(std::memory_order_relaxed);
            if (tmp == nullptr)
            {
                if (destroyed_)
                {
                    destroyed_ = false;
                    onDeadReference();
                }
                else
                {
                    tmp = CreatePolicy<T>::create();
                    pInstance_.store(tmp, std::memory_order_release);
#ifndef MAPNIK_NO_ATEXIT
                    // register destruction
                    std::atexit(&DestroySingleton);
#endif
                }
            }
        }
        return *tmp;
    }
};

#ifdef MAPNIK_THREADSAFE
template<typename T, template<typename U> class CreatePolicy>
std::mutex singleton<T, CreatePolicy>::mutex_;
template<typename T>
std::mutex singleton_cxx11<T>::mutex_;
#endif
template<typename T, template<typename U> class CreatePolicy>
std::atomic<T*> singleton<T, CreatePolicy>::pInstance_;
template<typename T, template<typename U> class CreatePolicy>
std::atomic<bool> singleton<T, CreatePolicy>::destroyed_(false);
} // namespace mapnik

#endif // MAPNIK_UTIL_SINGLETON_HPP
