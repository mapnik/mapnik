// SPDX-License-Identifier: LGPL-2.1-or-later
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

#ifndef MAPNIK_SQLITE_RESULTSET_HPP
#define MAPNIK_SQLITE_RESULTSET_HPP

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>

// stl
#include <string.h>

// sqlite
extern "C" {
#include <sqlite3.h>
}

//==============================================================================

class sqlite_resultset
{
  public:

    sqlite_resultset(sqlite3_stmt* stmt)
        : stmt_(stmt)
    {}

    ~sqlite_resultset()
    {
        if (stmt_)
        {
            sqlite3_finalize(stmt_);
        }
    }

    bool is_valid() { return stmt_ != 0; }

    bool step_next()
    {
        const int status = sqlite3_step(stmt_);
        if (status != SQLITE_ROW && status != SQLITE_DONE)
        {
            std::ostringstream s;
            s << "SQLite Plugin: retrieving next row failed";
            std::string msg(sqlite3_errmsg(sqlite3_db_handle(stmt_)));
            if (msg != "unknown error")
            {
                s << ": " << msg;
            }

            throw mapnik::datasource_exception(s.str());
        }
        return status == SQLITE_ROW;
    }

    int column_count() { return sqlite3_column_count(stmt_); }

    int column_type(int col) { return sqlite3_column_type(stmt_, col); }

    const char* column_name(int col) { return sqlite3_column_name(stmt_, col); }

    bool column_isnull(int col) { return sqlite3_column_type(stmt_, col) == SQLITE_NULL; }

    int column_integer(int col) { return sqlite3_column_int(stmt_, col); }

    sqlite_int64 column_integer64(int col) { return sqlite3_column_int64(stmt_, col); }

    double column_double(int col) { return sqlite3_column_double(stmt_, col); }

    const char* column_text(int col, int& len)
    {
        len = sqlite3_column_bytes(stmt_, col);
        return (const char*)sqlite3_column_text(stmt_, col);
    }

    const char* column_text(int col) { return (const char*)sqlite3_column_text(stmt_, col); }

    const char* column_blob(int col, int& bytes)
    {
        bytes = sqlite3_column_bytes(stmt_, col);
        return (const char*)sqlite3_column_blob(stmt_, col);
    }

    sqlite3_stmt* get_statement() { return stmt_; }

  private:

    sqlite3_stmt* stmt_;
};

#endif // MAPNIK_SQLITE_RESULTSET_HPP
