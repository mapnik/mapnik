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

#ifndef POSTGIS_CONNECTION_MANAGER_HPP
#define POSTGIS_CONNECTION_MANAGER_HPP

#include "connection.hpp"

// mapnik
#include <mapnik/pool.hpp>
#include <mapnik/utils.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
//using boost::mutex;
#endif

// stl
#include <string>
#include <sstream>

using mapnik::Pool;
using mapnik::singleton;
using mapnik::CreateStatic;

template <typename T>
class ConnectionCreator
{

public:
    ConnectionCreator(boost::optional<std::string> const& host,
                      boost::optional<std::string> const& port,
                      boost::optional<std::string> const& dbname,
                      boost::optional<std::string> const& user,
                      boost::optional<std::string> const& pass,
                      boost::optional<std::string> const& connect_timeout)
        : host_(host),
          port_(port),
          dbname_(dbname),
          user_(user),
          pass_(pass),
          connect_timeout_(connect_timeout) {}

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
        if (connect_timeout_ && (*connect_timeout_).size())
            connect_str +=" connect_timeout=" + *connect_timeout_;
        return connect_str;
    }

private:
    boost::optional<std::string> host_;
    boost::optional<std::string> port_;
    boost::optional<std::string> dbname_;
    boost::optional<std::string> user_;
    boost::optional<std::string> pass_;
    boost::optional<std::string> connect_timeout_;
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
        //mutex::scoped_lock lock(mutex_);
#endif
        if (pools_.find(creator.id())==pools_.end())
        {
            return pools_.insert(std::make_pair(creator.id(),
                                                boost::make_shared<PoolType>(creator,initialSize,maxSize))).second;
        }

        return false;

    }

    boost::shared_ptr<PoolType> getPool(std::string const& key)
    {
#ifdef MAPNIK_THREADSAFE
        //mutex::scoped_lock lock(mutex_);
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
        //mutex::scoped_lock lock(mutex_);
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

#endif // POSTGIS_CONNECTION_MANAGER_HPP
