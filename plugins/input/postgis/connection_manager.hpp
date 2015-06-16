/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/util/singleton.hpp>

// boost
#include <memory>
#include <boost/optional.hpp>

// stl
#include <string>
#include <sstream>
#include <memory>

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
        return new T(connection_string_safe(),pass_);
    }

    inline std::string id() const
    {
        return connection_string();
    }

    inline std::string connection_string() const
    {
        std::string connect_str = connection_string_safe();
        if (pass_   && !pass_->empty()) connect_str += " password=" + *pass_;
        return connect_str;
    }

    inline std::string connection_string_safe() const
    {
        std::string connect_str;
        if (host_   && !host_->empty()) connect_str += "host=" + *host_;
        if (port_   && !port_->empty()) connect_str += " port=" + *port_;
        if (dbname_ && !dbname_->empty()) connect_str += " dbname=" + *dbname_;
        if (user_   && !user_->empty()) connect_str += " user=" + *user_;
        if (connect_timeout_ && !connect_timeout_->empty())
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

public:
    using PoolType = Pool<Connection,ConnectionCreator>;

private:
    friend class CreateStatic<ConnectionManager>;

    using ContType = std::map<std::string,std::shared_ptr<PoolType> >;
    using HolderType = std::shared_ptr<Connection>;
    ContType pools_;

public:

    bool registerPool(const ConnectionCreator<Connection>& creator,unsigned initialSize,unsigned maxSize)
    {
        ContType::const_iterator itr = pools_.find(creator.id());

        if (itr != pools_.end())
        {
            itr->second->set_initial_size(initialSize);
            itr->second->set_max_size(maxSize);
        }
        else
        {
            return pools_.insert(
                std::make_pair(creator.id(),
                               std::make_shared<PoolType>(creator,initialSize,maxSize))).second;
        }
        return false;

    }

    std::shared_ptr<PoolType> getPool(std::string const& key)
    {
        ContType::const_iterator itr=pools_.find(key);
        if (itr!=pools_.end())
        {
            return itr->second;
        }
        static const std::shared_ptr<PoolType> emptyPool;
        return emptyPool;
    }

    ConnectionManager() {}
private:
    ConnectionManager(const ConnectionManager&);
    ConnectionManager& operator=(const ConnectionManager);
};

#endif // POSTGIS_CONNECTION_MANAGER_HPP
