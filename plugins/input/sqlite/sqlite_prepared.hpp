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

#ifndef MAPNIK_SQLITE_PREPARED_HPP
#define MAPNIK_SQLITE_PREPARED_HPP

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/noncopyable.hpp>

// boost
#include <memory>

// stl
#include <string.h>

#include "sqlite_connection.hpp"

// sqlite
extern "C" {
#include <sqlite3.h>
}

class prepared_index_statement : mapnik::noncopyable
{

public:
    prepared_index_statement(std::shared_ptr<sqlite_connection> ds, std::string const& sql)
        : ds_(ds),
          stmt_(0)
    {

        const int rc = sqlite3_prepare_v2(*(*ds_),
                                          sql.c_str(),
                                          -1,
                                          &stmt_,
                                          0);

        if (rc != SQLITE_OK)
        {
            std::ostringstream index_error;
            index_error << "Sqlite Plugin: auto-index table creation failed: '"
                        << sqlite3_errmsg(*(*ds_)) << "' query was: "
                        << sql;

            throw mapnik::datasource_exception(index_error.str());
        }
    }

    ~prepared_index_statement()
    {
        if (stmt_)
        {
            int res = sqlite3_finalize(stmt_);
            if (res != SQLITE_OK)
            {
                if (*(*ds_))
                {
                    MAPNIK_LOG_ERROR(sqlite) << "~prepared_index_statement:"
                                             << sqlite3_errmsg(*(*ds_));
                }
                else
                {
                    MAPNIK_LOG_ERROR(sqlite) << "~prepared_index_statement:"
                                             << res;
                }
            }
        }
    }

    void bind(sqlite_int64 const pkid)
    {
        if (sqlite3_bind_int64(stmt_, 1, pkid) != SQLITE_OK)
        {
            throw mapnik::datasource_exception("SQLite Plugin: invalid value for for key field while generating index");
        }

    }

    void bind(mapnik::box2d<double> const& bbox)
    {
        if ((sqlite3_bind_double(stmt_, 2, bbox.minx()) != SQLITE_OK) ||
            (sqlite3_bind_double(stmt_, 3, bbox.maxx()) != SQLITE_OK) ||
            (sqlite3_bind_double(stmt_, 4, bbox.miny()) != SQLITE_OK) ||
            (sqlite3_bind_double(stmt_, 5, bbox.maxy()) != SQLITE_OK))
        {
            throw mapnik::datasource_exception("SQLite Plugin: invalid value for for extent while generating index");
        }
    }

    bool step_next ()
    {
        const int status = sqlite3_step(stmt_);
        if (status != SQLITE_DONE)
        {
            std::ostringstream s;
            s << "SQLite Plugin: inserting bbox into rtree index failed";
            std::string msg(sqlite3_errmsg(sqlite3_db_handle(stmt_)));
            if (msg != "unknown error")
            {
                s << ": " << msg;
            }
            throw mapnik::datasource_exception(s.str());
        }
        sqlite3_clear_bindings(stmt_);
        if (sqlite3_reset(stmt_) != SQLITE_OK)
        {
            throw mapnik::datasource_exception("sqlite3_reset failed");
        }
        return true;
    }

private:
    std::shared_ptr<sqlite_connection> ds_;
    sqlite3_stmt * stmt_;
};

#endif // MAPNIK_SQLITE_PREPARED_HPP
