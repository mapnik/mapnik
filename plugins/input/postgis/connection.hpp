/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef POSTGIS_CONNECTION_HPP
#define POSTGIS_CONNECTION_HPP

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/timer.hpp>

// std
#include <memory>
#include <sstream>
#include <iostream>

extern "C" {
#include "libpq-fe.h"
}

#include "resultset.hpp"

class Connection
{
  public:
    Connection(std::string const& connection_str, boost::optional<std::string> const& password)
        : cursorId(0)
        , closed_(false)
        , pending_(false)
    {
        std::string connect_with_pass = connection_str;
        if (password && !password->empty())
        {
            connect_with_pass += " password=" + *password;
        }
        conn_ = PQconnectdb(connect_with_pass.c_str());
        MAPNIK_LOG_DEBUG(postgis) << "postgis_connection: postgresql connection create - " << this;
        if (PQstatus(conn_) != CONNECTION_OK)
        {
            std::string err_msg = "Postgis Plugin: ";
            err_msg += status();
            err_msg += "Connection string: '";
            err_msg += connection_str;
            err_msg += "'\n";
            MAPNIK_LOG_DEBUG(postgis) << "postgis_connection: creation failed, closing connection - " << this;
            close();
            throw mapnik::datasource_exception(err_msg);
        }
        PGresult* result =
          PQexec(conn_, "SET DEFAULT_TRANSACTION_READ_ONLY = TRUE; SET CLIENT_MIN_MESSAGES = WARNING;");
        bool ok = (result && (PQresultStatus(result) == PGRES_COMMAND_OK));
        if (result)
            PQclear(result);
        if (!ok)
        {
            std::string err_msg = "Postgis Plugin: ";
            err_msg += status();
            err_msg += "Connection string: '";
            err_msg += connection_str;
            err_msg += "'\n";
            close();
            throw mapnik::datasource_exception(err_msg);
        }
    }

    ~Connection()
    {
        if (!closed_)
        {
            PQfinish(conn_);
            MAPNIK_LOG_DEBUG(postgis) << "postgis_connection: postgresql connection closed - " << this;
            closed_ = true;
        }
    }

    bool execute(std::string const& sql)
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats__(std::clog, std::string("postgis_connection::execute ") + sql);
#endif

        if (!executeAsyncQuery(sql))
            return false;
        PGresult* result = 0;
        // fetch multiple times until NULL is returned,
        // to handle multi-statement queries
        while (PGresult* tmp = getResult())
        {
            if (result)
                PQclear(result);
            result = tmp;
        }
        bool ok = (result && (PQresultStatus(result) == PGRES_COMMAND_OK));
        if (result)
            PQclear(result);
        return ok;
    }

    std::shared_ptr<ResultSet> executeQuery(std::string const& sql, int type = 0)
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats__(std::clog, std::string("postgis_connection::execute_query ") + sql);
#endif
        PGresult* result = 0;
        if (executeAsyncQuery(sql, type))
        {
            // fetch multiple times until NULL is returned,
            // to handle multi-statement queries
            while (PGresult* tmp = getResult())
            {
                if (result)
                    PQclear(result);
                result = tmp;
            }
        }

        if (!result || (PQresultStatus(result) != PGRES_TUPLES_OK))
        {
            std::string err_msg = "Postgis Plugin: ";
            err_msg += status();
            err_msg += "in executeQuery Full sql was: '";
            err_msg += sql;
            err_msg += "'\n";
            if (result)
                PQclear(result);
            throw mapnik::datasource_exception(err_msg);
        }

        return std::make_shared<ResultSet>(result);
    }

    std::string status() const
    {
        std::string status;
        if (conn_)
        {
            char* err_msg = PQerrorMessage(conn_);
            if (err_msg == nullptr)
            {
                status = "Bad connection\n";
            }
            else
            {
                status = std::string(err_msg);
            }
        }
        else
        {
            status = "Uninitialized connection\n";
        }
        return status;
    }

    bool executeAsyncQuery(std::string const& sql, int type = 0)
    {
        int result = 0;
        if (type == 1)
        {
            result = PQsendQueryParams(conn_, sql.c_str(), 0, 0, 0, 0, 0, 1);
        }
        else
        {
            result = PQsendQuery(conn_, sql.c_str());
        }
        if (result != 1)
        {
            std::string err_msg = "Postgis Plugin: ";
            err_msg += status();
            err_msg += "in executeAsyncQuery Full sql was: '";
            err_msg += sql;
            err_msg += "'\n";
            clearAsyncResult(PQgetResult(conn_));
            close();
            throw mapnik::datasource_exception(err_msg);
        }
        pending_ = true;
        return result;
    }

    PGresult* getResult()
    {
        PGresult* result = PQgetResult(conn_);
        return result;
    }

    std::shared_ptr<ResultSet> getNextAsyncResult()
    {
        PGresult* result = getResult();
        if (result && (PQresultStatus(result) != PGRES_TUPLES_OK))
        {
            std::string err_msg = "Postgis Plugin: ";
            err_msg += status();
            err_msg += "in getNextAsyncResult";
            clearAsyncResult(result);
            // We need to guarde against losing the connection
            // (i.e db restart) so here we invalidate the full connection
            close();
            throw mapnik::datasource_exception(err_msg);
        }
        return std::make_shared<ResultSet>(result);
    }

    std::shared_ptr<ResultSet> getAsyncResult()
    {
        PGresult* result = getResult();
        if (!result || (PQresultStatus(result) != PGRES_TUPLES_OK))
        {
            std::string err_msg = "Postgis Plugin: ";
            err_msg += status();
            err_msg += "in getAsyncResult";
            clearAsyncResult(result);
            // We need to be guarded against losing the connection
            // (i.e db restart), we invalidate the full connection
            close();
            throw mapnik::datasource_exception(err_msg);
        }
        return std::make_shared<ResultSet>(result);
    }

    std::string client_encoding() const
    {
        return PQparameterStatus(conn_, "client_encoding");
    }

    bool isOK() const
    {
        return (!closed_) && (PQstatus(conn_) != CONNECTION_BAD);
    }

    bool isPending() const
    {
        return pending_;
    }

    void close()
    {
        if (!closed_)
        {
            PQfinish(conn_);
            MAPNIK_LOG_DEBUG(postgis) << "postgis_connection: closing connection (close)- " << this;
            closed_ = true;
        }
    }

    std::string new_cursor_name()
    {
        std::ostringstream s;
        s << "mapnik_" << (cursorId++);
        return s.str();
    }

  private:
    PGconn* conn_;
    int cursorId;
    bool closed_;
    bool pending_;

    void clearAsyncResult(PGresult* result)
    {
        // Clear all pending results
        while (result)
        {
            PQclear(result);
            result = PQgetResult(conn_);
        }
        pending_ = false;
    }
};

#endif // CONNECTION_HPP
