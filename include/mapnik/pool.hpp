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

//$Id: pool.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef POOL_HPP
#define POOL_HPP

// mapnik
#include <mapnik/utils.hpp>
// boost
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
#endif

// stl
#include <iostream>
#include <map>
#include <deque>
#include <ctime>

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
   class Pool : private boost::noncopyable
   {
         typedef boost::shared_ptr<T> HolderType;
         typedef std::deque<HolderType> ContType;	
	
         Creator<T> creator_;
         const unsigned initialSize_; 
         const unsigned maxSize_;
         ContType usedPool_;
         ContType unusedPool_;
#ifdef MAPNIK_THREADSAFE
         mutable boost::mutex mutex_;
#endif
      public:

         Pool(const Creator<T>& creator,unsigned initialSize=1, unsigned maxSize=10)
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
            if (itr!=unusedPool_.end())
            { 
#ifdef MAPNIK_DEBUG
               std::clog<<"borrow "<<(*itr).get()<<"\n";
#endif
               usedPool_.push_back(*itr);
               itr=unusedPool_.erase(itr);
               return usedPool_[usedPool_.size()-1];
            }
            else if (unusedPool_.size() < maxSize_)
            {
               HolderType conn(creator_());
               if (conn->isOK())
               {
                  usedPool_.push_back(conn);
#ifdef MAPNIK_DEBUG
                  std::clog << "create << " << conn.get() << "\n";
#endif
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
#ifdef MAPNIK_DEBUG
                  std::clog<<"return "<<(*itr).get()<<"\n";
#endif
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
   };
}
#endif //POOL_HPP
