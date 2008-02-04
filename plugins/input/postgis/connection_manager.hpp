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
#include <boost/optional.hpp>

#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
using boost::mutex;
#endif

using mapnik::Pool;
using mapnik::singleton;
using mapnik::CreateStatic;
using std::string;

template <typename T>
class ConnectionCreator
{

public:
      ConnectionCreator(boost::optional<string> const& host,
                        boost::optional<string> const& port,
                        boost::optional<string> const& dbname,
                        boost::optional<string> const& user,
                        boost::optional<string> const& pass)
         : host_(host),
           port_(port),
           dbname_(dbname),
           user_(user),
           pass_(pass) {}
      
      T* operator()() const
      {
          return new T(connection_string());
      }
      
      inline std::string id() const 
      {
         return connection_string();
      }
      
      inline std::string connection_string() const
      {
         std::string connect_str;
         if (host_   && (*host_).size()) connect_str += "host=" + *host_;
         if (port_   && (*port_).size()) connect_str += " port=" + *port_;
         if (dbname_ && (*dbname_).size()) connect_str += " dbname=" + *dbname_;
         if (user_   && (*user_).size()) connect_str += " user=" + *user_;
         if (pass_   && (*pass_).size()) connect_str += " password=" + *pass_;
         connect_str += " connect_timeout=4"; // todo: set by client (param)
         return connect_str;
      }
      
private:
      boost::optional<string> host_;
      boost::optional<string> port_;
      boost::optional<string> dbname_;
      boost::optional<string> user_;
      boost::optional<string> pass_;

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
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(mutex_);
#endif
        if (pools_.find(creator.id())==pools_.end())
        {
            return pools_.insert(std::make_pair(creator.id(),
                                                boost::shared_ptr<PoolType>(new PoolType(creator,initialSize,maxSize)))).second;
        }

        return false;
	   	     
    }
    
    boost::shared_ptr<PoolType> getPool(std::string const& key) 
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(mutex_);
#endif 
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
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(mutex_);
#endif 
        ContType::const_iterator itr=pools_.find(key);
        if (itr!=pools_.end()) 
        {
            boost::shared_ptr<PoolType> pool=itr->second;
            return pool->borrowObject();
        }
        return HolderType();
    }
    ConnectionManager() {}
private:
    ConnectionManager(const ConnectionManager&);
    ConnectionManager& operator=(const ConnectionManager);
};    

#endif //CONNECTION_MANAGER_HPP
