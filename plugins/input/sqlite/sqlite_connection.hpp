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

#ifndef MAPNIK_SQLITE_CONNECTION_HPP
#define MAPNIK_SQLITE_CONNECTION_HPP

// stl
#include <string.h>

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/sql_utils.hpp>
#include <mapnik/timer.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>

// sqlite
extern "C" {
#include <sqlite3.h>
}

#include "sqlite_resultset.hpp"


//==============================================================================

class sqlite_connection
{
public:

    sqlite_connection (const std::string& file)
        : db_(0),
          file_(file)
    {
#if SQLITE_VERSION_NUMBER >= 3005000
        int mode = SQLITE_OPEN_READWRITE;
#if SQLITE_VERSION_NUMBER >= 3006018
        // shared cache flag not available until >= 3.6.18
        mode |= SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE;
#endif
        const int rc = sqlite3_open_v2 (file_.c_str(), &db_, mode, 0);
#else
#warning "Mapnik's sqlite plugin is compiling against a version of sqlite older than 3.5.x which may make rendering slow..."
        const int rc = sqlite3_open (file_.c_str(), &db_);
#endif
        if (rc != SQLITE_OK)
        {
            std::ostringstream s;
            s << "Sqlite Plugin: " << sqlite3_errmsg (db_);

            throw mapnik::datasource_exception (s.str());
        }

        sqlite3_busy_timeout(db_,5000);
    }

    sqlite_connection (const std::string& file, int flags)
        : db_(0),
          file_(file)
    {
#if SQLITE_VERSION_NUMBER >= 3005000
        const int rc = sqlite3_open_v2 (file_.c_str(), &db_, flags, 0);
#else
#warning "Mapnik's sqlite plugin is compiling against a version of sqlite older than 3.5.x which may make rendering slow..."
        const int rc = sqlite3_open (file_.c_str(), &db_);
#endif
        if (rc != SQLITE_OK)
        {
            std::ostringstream s;
            s << "Sqlite Plugin: " << sqlite3_errmsg (db_);

            throw mapnik::datasource_exception (s.str());
        }
    }

    virtual ~sqlite_connection ()
    {
        if (db_)
        {
            sqlite3_close (db_);
        }
    }

    void throw_sqlite_error(const std::string& sql)
    {
        std::ostringstream s;
        s << "Sqlite Plugin: ";
        if (db_)
            s << "'" << sqlite3_errmsg(db_) << "'";
        else
            s << "unknown error, lost connection";
        s << " (" << file_ << ")"
          << "\nFull sql was: '"
          <<  sql << "'";

        throw mapnik::datasource_exception (s.str());
    }

    boost::shared_ptr<sqlite_resultset> execute_query(const std::string& sql)
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats__(std::clog, std::string("sqlite_resultset::execute_query ") + sql);
#endif
        sqlite3_stmt* stmt = 0;

        const int rc = sqlite3_prepare_v2 (db_, sql.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            throw_sqlite_error(sql);
        }

        return boost::make_shared<sqlite_resultset>(stmt);
    }

    void execute(const std::string& sql)
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats__(std::clog, std::string("sqlite_resultset::execute ") + sql);
#endif

        const int rc = sqlite3_exec(db_, sql.c_str(), 0, 0, 0);
        if (rc != SQLITE_OK)
        {
            throw_sqlite_error(sql);
        }
    }

    int execute_with_code(const std::string& sql)
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats__(std::clog, std::string("sqlite_resultset::execute_with_code ") + sql);
#endif

        const int rc = sqlite3_exec(db_, sql.c_str(), 0, 0, 0);
        return rc;
    }

    sqlite3* operator*()
    {
        return db_;
    }


private:

    sqlite3* db_;
    std::string file_;
};

#endif // MAPNIK_SQLITE_CONNECTION_HPP
