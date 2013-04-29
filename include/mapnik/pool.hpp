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
template <typename T,template <typename> class Creator>
class Pool : private mapnik::noncopyable
{
    typedef boost::shared_ptr<T> HolderType;
    typedef std::deque<HolderType> ContType;

    Creator<T> creator_;
    unsigned initialSize_;
    unsigned maxSize_;
    ContType pool_;
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
                pool_.push_back(conn);
        }
    }

    HolderType borrowObject()
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(mutex_);
#endif

        typename ContType::iterator itr=pool_.begin();
        while ( itr!=pool_.end())
        {
            if (!itr->unique())
            {
                ++itr;
            }
            else if ((*itr)->isOK())
            {
                MAPNIK_LOG_DEBUG(pool) << "pool: Borrow instance=" << (*itr).get();
                return *itr;
            }
            else
            {
                MAPNIK_LOG_DEBUG(pool) << "pool: Bad connection (erase) instance=" << (*itr).get();

                itr=pool_.erase(itr);
            }
        }
        // all connection have been taken, check if we allowed to grow pool
        if (pool_.size() < maxSize_)
        {
            HolderType conn(creator_());
            if (conn->isOK())
            {
                pool_.push_back(conn);

                MAPNIK_LOG_DEBUG(pool) << "pool: Create connection=" << conn.get();

                return conn;
            }
        }
        return HolderType();
    }

    unsigned size() const
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(mutex_);
#endif
        return pool_.size();
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
            unsigned total_size = pool_.size();
            // ensure we don't have ghost obj's in the pool.
            if (total_size < initialSize_)
            {
                unsigned grow_size = initialSize_ - total_size ;

                for (unsigned i=0; i < grow_size; ++i)
                {
                    HolderType conn(creator_());
                    if (conn->isOK())
                        pool_.push_back(conn);
                }
            }
        }
    }
};

}

#endif // MAPNIK_POOL_HPP
