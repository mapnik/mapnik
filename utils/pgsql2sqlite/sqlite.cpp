/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#include "sqlite.hpp"

namespace mapnik {
namespace sqlite {

database::database(std::string const& name)
{
    sqlite3* db;
    int res = sqlite3_open(name.c_str(), &db);
    if (res)
    {
        sqlite3_close(db);
        throw;
    }

    db_ = sqlite_db(db, database_closer());
#ifdef MAPNIK_DEBUG
    std::cerr << "Open database " << name << "\n";
#endif
}

database::~database() {}

bool database::execute(std::string const& sql)
{
    char* err_msg;
    int res = sqlite3_exec(db_.get(), sql.c_str(), 0, 0, &err_msg);
    if (res != SQLITE_OK)
    {
        std::cerr << "SQL" << sql << " ERR:" << err_msg << "\n";
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}
} // namespace sqlite
} // namespace mapnik
