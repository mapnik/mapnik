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

#include <mapnik/util/noncopyable.hpp>
#include <mapnik/util/variant.hpp>
// boost
#include <memory>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <sqlite3.h>
MAPNIK_DISABLE_WARNING_POP

// stl
#ifdef MAPNIK_DEBUG
#include <cassert>
#endif

#include <iostream>
#include <string>
#include <vector>

namespace mapnik {
namespace sqlite {

class database : private util::noncopyable
{
    friend class prepared_statement;

    struct database_closer
    {
        void operator()(sqlite3* db)
        {
#ifdef MAPNIK_DEBUG
            std::cerr << "close database " << db << "\n";
#endif
            sqlite3_close(db);
        }
    };

    using sqlite_db = std::shared_ptr<sqlite3>;
    sqlite_db db_;

  public:
    database(std::string const& name);
    ~database();
    bool execute(std::string const& sql);
};

struct null_type
{};
struct blob
{
    blob(char const* buf, unsigned size)
        : buf_(buf),
          size_(size)
    {}

    char const* buf_;
    unsigned size_;
};

using value_type = mapnik::util::variant<int, double, std::string, blob, null_type>;
using record_type = std::vector<value_type>;

class prepared_statement : util::noncopyable
{
    struct binder
    {
        binder(sqlite3_stmt* stmt, unsigned index)
            : stmt_(stmt),
              index_(index)
        {}

        bool operator()(null_type)
        {
            if (sqlite3_bind_null(stmt_, index_) != SQLITE_OK)
            {
                std::cerr << "cannot bind nullptr\n";
                return false;
            }
            return true;
        }

        bool operator()(int val)
        {
            if (sqlite3_bind_int(stmt_, index_, val) != SQLITE_OK)
            {
                std::cerr << "cannot bind " << val << "\n";
                return false;
            }
            return true;
        }

        bool operator()(double val)
        {
            if (sqlite3_bind_double(stmt_, index_, val) != SQLITE_OK)
            {
                std::cerr << "cannot bind " << val << "\n";
                return false;
            }
            return true;
        }

        bool operator()(std::string const& val)
        {
            if (sqlite3_bind_text(stmt_, index_, val.c_str(), val.length(), SQLITE_STATIC) != SQLITE_OK)
            {
                std::cerr << "cannot bind " << val << "\n";
                return false;
            }
            return true;
        }

        bool operator()(blob const& val)
        {
            if (sqlite3_bind_blob(stmt_, index_, val.buf_, val.size_, SQLITE_STATIC) != SQLITE_OK)
            {
                std::cerr << "cannot bind BLOB\n";
                return false;
            }
            return true;
        }

        sqlite3_stmt* stmt_;
        unsigned index_;
    };

  public:
    prepared_statement(database& db, std::string const& sql)
        : db_(db.db_.get()),
          stmt_(0)
    {
        char const* tail;
        // char * err_msg;
        int res = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt_, &tail);
        if (res != SQLITE_OK)
        {
            std::cerr << "ERR:" << res << "\n";
            throw;
        }
    }

    ~prepared_statement()
    {
        int res = sqlite3_finalize(stmt_);
        if (res != SQLITE_OK)
        {
            std::cerr << "ERR:" << res << "\n";
        }
    }

    bool insert_record(record_type const& rec) const
    {
#ifdef MAPNIK_DEBUG
        assert(unsigned(sqlite3_bind_parameter_count(stmt_)) == rec.size());
#endif
        record_type::const_iterator itr = rec.begin();
        record_type::const_iterator end = rec.end();
        int count = 1;
        for (; itr != end; ++itr)
        {
            binder op(stmt_, count++);
            if (!util::apply_visitor(op, *itr))
            {
                return false;
            }
        }

        sqlite3_step(stmt_);
        sqlite3_reset(stmt_);

        return true;
    }

  private:
    sqlite3* db_;
    sqlite3_stmt* stmt_;
};
} // namespace sqlite
} // namespace mapnik
