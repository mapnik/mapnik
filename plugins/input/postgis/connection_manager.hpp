/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/params.hpp>
#include <mapnik/pool.hpp>
#include <mapnik/util/singleton.hpp>

// stl
#include <string>
#include <memory>
#include <optional>

using mapnik::CreateStatic;
using mapnik::Pool;
using mapnik::singleton;

template<typename T>
class ConnectionCreator
{
  public:
    ConnectionCreator(mapnik::parameters const& params)
        : host_{params.get<std::string>("host")},
          port_{params.get<std::string>("port")},
          dbname_{params.get<std::string>("dbname")},
          user_{params.get<std::string>("user")},
          password_{params.get<std::string>("password")},
          connect_timeout_{params.get<std::string>("connect_timeout", "4")},
          application_name_{params.get<std::string>("application_name")}
    {}

    T* operator()() const { return new T(connection_string_safe(), password_); }

    inline std::string id() const { return connection_string_safe(); }

    inline std::string connection_string() const
    {
        std::string connect_str = connection_string_safe();
        append_param(connect_str, "password=", password_);
        return connect_str;
    }

    inline std::string connection_string_safe() const
    {
        std::string connect_str;
        append_param(connect_str, "host=", host_);
        append_param(connect_str, "port=", port_);
        append_param(connect_str, "dbname=", dbname_);
        append_param(connect_str, "user=", user_);
        append_param(connect_str, "connect_timeout=", connect_timeout_);
        if (!append_param(connect_str, "application_name=", application_name_))
        {
            // only set fallback_application_name, so that application_name
            // can still be overriden with PGAPPNAME environment variable
            append_param(connect_str, "fallback_application_name=", std::string{"mapnik"});
        }
        return connect_str;
    }

  private:

    static bool append_param(std::string& dest, char const* key, std::string const& val)
    {
        if (val.empty())
            return false;
        if (!dest.empty())
            dest += ' ';
        dest += key;
        dest += val;
        return true;
    }

    static bool append_param(std::string& dest, char const* key, std::optional<std::string> const& opt)
    {
        return opt && append_param(dest, key, *opt);
    }

    std::optional<std::string> host_;
    std::optional<std::string> port_;
    std::optional<std::string> dbname_;
    std::optional<std::string> user_;
    std::optional<std::string> password_;
    std::optional<std::string> connect_timeout_;
    std::optional<std::string> application_name_;
};

class ConnectionManager : public singleton<ConnectionManager, CreateStatic>
{
  public:
    using PoolType = Pool<Connection, ConnectionCreator>;

  private:
    friend class CreateStatic<ConnectionManager>;

    using ContType = std::map<std::string, std::shared_ptr<PoolType>>;
    using HolderType = std::shared_ptr<Connection>;
    ContType pools_;

  public:

    bool registerPool(ConnectionCreator<Connection> const& creator, unsigned initialSize, unsigned maxSize)
    {
#ifdef MAPNIK_THREADSAFE
        std::lock_guard<std::mutex> lock(mutex_);
#endif
        ContType::const_iterator itr = pools_.find(creator.id());

        if (itr != pools_.end())
        {
            itr->second->set_initial_size(initialSize);
            itr->second->set_max_size(maxSize);
        }
        else
        {
            return pools_
              .insert(std::make_pair(creator.id(), std::make_shared<PoolType>(creator, initialSize, maxSize)))
              .second;
        }
        return false;
    }

    std::shared_ptr<PoolType> getPool(std::string const& key)
    {
#ifdef MAPNIK_THREADSAFE
        std::lock_guard<std::mutex> lock(mutex_);
#endif
        ContType::const_iterator itr = pools_.find(key);
        if (itr != pools_.end())
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
