/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2007 Artem Pavlenko
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
//$Id$

#ifndef SQLITE_TYPES_HPP
#define SQLITE_TYPES_HPP

// mapnik
#include <mapnik/datasource.hpp>

// boost
#include <boost/shared_ptr.hpp>

// sqlite
extern "C" {
  #include <sqlite3.h>
}


class sqlite_resultset
{
public:

    sqlite_resultset (sqlite3_stmt* stmt)
        : stmt_(stmt)
    {
    }

    ~sqlite_resultset ()
    {
        if (stmt_)
            sqlite3_finalize (stmt_);
    }

    bool is_valid ()
    {
        return stmt_ != 0;
    }

    bool step_next ()
    {
        return (sqlite3_step (stmt_) == SQLITE_ROW);
    }

    int column_count ()
    {
        return sqlite3_column_count (stmt_);
    }

    int column_type (int col)
    {
        return sqlite3_column_type (stmt_, col);
    }
    
    const char* column_name (int col)
    {
        return sqlite3_column_name (stmt_, col);
    }

    bool column_isnull (int col)
    {
        return sqlite3_column_type (stmt_, col) == SQLITE_NULL;
    }

    int column_integer (int col)
    {
        return sqlite3_column_int (stmt_, col);
    }

    double column_double (int col)
    {
        return sqlite3_column_double (stmt_, col);
    }

    const char* column_text (int col)
    {
        return (const char*) sqlite3_column_text (stmt_, col);
    }

    const void* column_blob (int col, int& bytes)
    {
        bytes = sqlite3_column_bytes (stmt_, col);
    
        return sqlite3_column_blob (stmt_, col);
    }

    sqlite3_stmt* get_statement()
    {
        return stmt_;
    }

private:

    sqlite3_stmt* stmt_;
};



class sqlite_connection
{
public:

    sqlite_connection (const std::string& file)
        : db_(0)
    {
        if (sqlite3_open (file.c_str(), &db_))
            throw mapnik::datasource_exception (sqlite3_errmsg (db_));

        //sqlite3_enable_load_extension(db_, 1);
    }

    ~sqlite_connection ()
    {
        if (db_)
            sqlite3_close (db_);
    }

    sqlite_resultset* execute_query (const std::string& sql)
    {
        sqlite3_stmt* stmt = 0;

        int rc = sqlite3_prepare_v2 (db_, sql.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            std::clog << sqlite3_errmsg(db_) << std::endl;
        }

        return new sqlite_resultset (stmt);
	}

    sqlite3* operator*()
    {
        return db_;
    }

private:

    sqlite3* db_;
};

#endif //SQLITE_TYPES_HPP

