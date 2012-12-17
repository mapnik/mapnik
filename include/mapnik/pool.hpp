/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_POOL_HPP
#define MAPNIK_POOL_HPP

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/noncopyable.hpp>

// boost
#include <boost/shared_ptr.hpp>
#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
#endif

// stl
#include <map>
#include <deque>
#include <ctime>
#include <cassert>

namespace mapnik
{
template <typename T, typename PoolT>
class PoolGuard
{
private:
    const T& obj_;
    PoolT& pool_;
public:
    explicit PoolGuard(const T& ptr,PoolT& pool)
        : obj_(ptr),
          pool_(pool) {}

    ~PoolGuard()
    {
        pool_->returnObject(obj_);
    }

private:
    PoolGuard();
    PoolGuard(const PoolGuard&);
    PoolGuard& operator=(const PoolGuard&);
};

template <typename T,template <typename> class Creator>
class Pool : private mapnik::noncopyable
{
    typedef boost::shared_ptr<T> HolderType;
    typedef std::deque<HolderType> ContType;

    Creator<T> creator_;
    unsigned initialSize_;
    unsigned maxSize_;
    ContType usedPool_;
    ContType unusedPool_;
#ifdef MAPNIK_THREADSAFE
    mutable boost::mutex mutex_;
#endif
public:

    Pool(const Creator<T>& creator,unsigned initialSize, unsigned maxSize)
        :creator_(creator),
         initialSize_(initialSize),
         maxSize_(maxSize)
    {
        for (unsigned i=0; i < initialSize_; ++i)
        {
            HolderType conn(creator_());
            if (conn->isOK())
                unusedPool_.push_back(conn);
        }
    }

    HolderType borrowObject()
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(mutex_);
#endif
        typename ContType::iterator itr=unusedPool_.begin();
        while ( itr!=unusedPool_.end())
        {
            MAPNIK_LOG_DEBUG(pool) << "pool: Borrow instance=" << (*itr).get();

            if ((*itr)->isOK())
            {
                usedPool_.push_back(*itr);
                unusedPool_.erase(itr);
                return usedPool_.back();
            }
            else
            {
                MAPNIK_LOG_DEBUG(pool) << "pool: Bad connection (erase) instance=" << (*itr).get();

                itr=unusedPool_.erase(itr);
            }
        }
        // all connection have been taken, check if we allowed to grow pool
        if (usedPool_.size() < maxSize_)
        {
            HolderType conn(creator_());
            if (conn->isOK())
            {
                usedPool_.push_back(conn);

                MAPNIK_LOG_DEBUG(pool) << "pool: Create connection=" << conn.get();

                return conn;
            }
        }
        return HolderType();
    }

    void returnObject(HolderType obj)
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(mutex_);
#endif
        typename ContType::iterator itr=usedPool_.begin();
        while (itr != usedPool_.end())
        {
            if (obj.get()==(*itr).get())
            {
                MAPNIK_LOG_DEBUG(pool) << "pool: Return instance=" << (*itr).get();

                unusedPool_.push_back(*itr);
                usedPool_.erase(itr);
                return;
            }
            ++itr;
        }
    }

    std::pair<unsigned,unsigned> size() const
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(mutex_);
#endif
        std::pair<unsigned,unsigned> size(unusedPool_.size(),usedPool_.size());
        return size;
    }

    unsigned max_size() const
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(mutex_);
#endif
        return maxSize_;
    }

    void set_max_size(unsigned size)
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(mutex_);
#endif
        maxSize_ = std::max(maxSize_,size);
    }

    unsigned initial_size() const
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(mutex_);
#endif
        return initialSize_;
    }

    void set_initial_size(unsigned size)
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(mutex_);
#endif
        if (size > initialSize_)
        {
            initialSize_ = size;
            unsigned total_size = usedPool_.size() + unusedPool_.size();
            // ensure we don't have ghost obj's in the pool.
            if (total_size < initialSize_)
            {
                unsigned grow_size = initialSize_ - total_size ;

                for (unsigned i=0; i < grow_size; ++i)
                {
                    HolderType conn(creator_());
                    if (conn->isOK())
                        unusedPool_.push_back(conn);
                }
            }
        }
    }
};

}

#endif // MAPNIK_POOL_HPP
