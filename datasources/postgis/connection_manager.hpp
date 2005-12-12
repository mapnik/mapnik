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

//$Id: connection_manager.hpp 17 2005-03-08 23:58:43Z pavlenko $

#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include <string>
#include "pool.hpp"
#include "utils.hpp"
#include "connection.hpp"
#include <boost/shared_ptr.hpp>

using namespace mapnik;
using std::string;

template <typename T>
class ConnectionCreator
{
    string url_;
    string dbname_;
    string user_;
    string pass_;
public:
    ConnectionCreator(const string& url,const string& dbname,
		      const string& user,const string& pass)
	: url_(url),
	  dbname_(dbname),
	  user_(user),
	  pass_(pass) {}
    
    T* operator()() const
    {
	return new T(url_,dbname_,user_,pass_);
    }
    std::string id() const 
    {
	return url_+":"+dbname_+":"+user_+":"+pass_;
    }
};

class ConnectionManager : public singleton <ConnectionManager,CreateStatic>
{

    friend class CreateStatic<ConnectionManager>;
    typedef Pool<Connection,ConnectionCreator> PoolType;
    typedef std::map<std::string,boost::shared_ptr<PoolType> > ContType;
    typedef boost::shared_ptr<Connection> HolderType;   
    ContType pools_;

public:
	
    bool registerPool(const ConnectionCreator<Connection>& creator,int initialSize,int maxSize) 
    {	    
	Lock lock(&mutex_);
	if (pools_.find(creator.id())==pools_.end())
	{
	    return pools_.insert(std::make_pair(creator.id(),
						boost::shared_ptr<PoolType>(new PoolType(creator,initialSize,maxSize)))).second;
	}

	return false;
	   	     
    }
    const boost::shared_ptr<PoolType>& getPool(const std::string& key) 
    {
	Lock lock(&mutex_);
	ContType::const_iterator itr=pools_.find(key);
	if (itr!=pools_.end())
	{
	    return itr->second;
	}
	static const boost::shared_ptr<PoolType> emptyPool;
	return emptyPool;
    }
	
    const HolderType& get(const std::string& key)
    {
	Lock lock(&mutex_);
	ContType::const_iterator itr=pools_.find(key);
	if (itr!=pools_.end()) 
	{
	    boost::shared_ptr<PoolType> pool=itr->second;
	    return pool->borrowObject();
	}
	static const HolderType EmptyConn;
	return EmptyConn;
    }
        
private:
    ConnectionManager() {}
    ConnectionManager(const ConnectionManager&);
    ConnectionManager& operator=(const ConnectionManager);
};    

#endif //CONNECTION_MANAGER_HPP
