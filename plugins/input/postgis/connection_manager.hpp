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

//$Id: connection_manager.hpp 17 2005-03-08 23:58:43Z pavlenko $

#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include <string>
#include <mapnik/pool.hpp>
#include <mapnik/utils.hpp>
#include "connection.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

using mapnik::Pool;
using mapnik::singleton;
using mapnik::CreateStatic;
using std::string;
using boost::mutex;

template <typename T>
class ConnectionCreator
{

public:
    ConnectionCreator(string const& host,
                      string const& port,
                      string const& dbname,
                      string const& user,
                      string const& pass)
        : host_(host),
          port_(port),
          dbname_(dbname),
          user_(user),
          pass_(pass) {}
    
    T* operator()() const
    {
        return new T(host_,port_,dbname_,user_,pass_);
    }
    
    std::string id() const 
    {
        return host_ + ":" 
	  + dbname_ + ":" 
	  + port_ +":" 
	  + user_ ; 
    }
private:
    string host_;
    string port_;
    string dbname_;
    string user_;
    string pass_;

};

class ConnectionManager : public singleton <ConnectionManager,CreateStatic>
{

    friend class CreateStatic<ConnectionManager>;
    typedef Pool<Connection,ConnectionCreator> PoolType;
    typedef std::map<std::string,boost::shared_ptr<PoolType> > ContType;
    typedef boost::shared_ptr<Connection> HolderType;   
    ContType pools_;

public:
	
    bool registerPool(const ConnectionCreator<Connection>& creator,unsigned initialSize,unsigned maxSize) 
    {	    
        mutex::scoped_lock lock(mutex_);
        if (pools_.find(creator.id())==pools_.end())
        {
            return pools_.insert(std::make_pair(creator.id(),
                                                boost::shared_ptr<PoolType>(new PoolType(creator,initialSize,maxSize)))).second;
        }

        return false;
	   	     
    }
    
    boost::shared_ptr<PoolType> getPool(std::string const& key) 
    {
        mutex::scoped_lock lock(mutex_);
        ContType::const_iterator itr=pools_.find(key);
        if (itr!=pools_.end())
        {
            return itr->second;
        }
        static const boost::shared_ptr<PoolType> emptyPool;
        return emptyPool;
    }
	
    HolderType get(std::string const& key)
    {
        mutex::scoped_lock lock(mutex_);
        ContType::const_iterator itr=pools_.find(key);
        if (itr!=pools_.end()) 
        {
            boost::shared_ptr<PoolType> pool=itr->second;
            return pool->borrowObject();
        }
        return HolderType();
    }
    
private:
    ConnectionManager() {}
    ConnectionManager(const ConnectionManager&);
    ConnectionManager& operator=(const ConnectionManager);
};    

#endif //CONNECTION_MANAGER_HPP
