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

//$Id: pool.hh 58 2004-10-31 16:21:26Z artem $

#ifndef POOL_HH
#define POOL_HH

#include <iostream>
#include <map>
#include <deque>
#include <ctime>

#include "ptr.hh"
#include "utils.hh"

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
    class Pool 
    {
	typedef ref_ptr<T> HolderType;
	typedef std::deque<HolderType> ContType;	
	
	Creator<T> creator_;
	const int initialSize_; 
	const int maxSize_;
	ContType usedPool_;
	ContType unusedPool_;
	Mutex mutex_;
    public:

	Pool(const Creator<T>& creator,int initialSize=5,int maxSize=20)
	    :creator_(creator),
	     initialSize_(initialSize),
	     maxSize_(maxSize)
	{
	    for (int i=0;i<initialSize_;++i) 
	    {
		unusedPool_.push_back(HolderType(creator_()));
	    }
	}

	const HolderType& borrowObject()
	{	    
	    Lock lock(&mutex_);
	    typename ContType::iterator itr=unusedPool_.begin();
	    if (itr!=unusedPool_.end())
	    {  
		std::cout<<"borrow "<<(*itr).get()<<"\n";
		usedPool_.push_back(*itr);
		itr=unusedPool_.erase(itr);
		mutex_.unlock();
		return usedPool_[usedPool_.size()-1];
	    }
	    static const HolderType defaultObj(0);
	    return defaultObj;
	} 

	void returnObject(const HolderType& obj)
	{
	    Lock lock(&mutex_);    
	    typename ContType::iterator itr=usedPool_.begin();
	    while (itr != usedPool_.end())
	    {
		if (obj.get()==(*itr).get()) 
		{
		    std::cout<<"return "<<(*itr).get()<<"\n";
		    unusedPool_.push_back(*itr);
		    usedPool_.erase(itr);
		    return;
		}
		++itr;
	    }
	}
	
    private:
	Pool(const Pool&);
	Pool& operator=(const Pool&);
    };

}
#endif                                            //POOL_HH
