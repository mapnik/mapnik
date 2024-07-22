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

#ifndef POSTGIS_CURSORRESULTSET_HPP
#define POSTGIS_CURSORRESULTSET_HPP

#include <mapnik/debug.hpp>

#include "connection.hpp"
#include "resultset.hpp"

class CursorResultSet : public IResultSet,
                        private mapnik::util::noncopyable
{
  public:
    CursorResultSet(std::shared_ptr<Connection> const& conn, std::string cursorName, int fetch_count)
        : conn_(conn)
        , cursorName_(cursorName)
        , fetch_size_(fetch_count)
        , is_closed_(false)
    {
        getNextResultSet();
    }

    virtual ~CursorResultSet() { close(); }

    virtual void close()
    {
        if (!is_closed_)
        {
            rs_.reset();

            std::ostringstream s;
            s << "CLOSE " << cursorName_;

            MAPNIK_LOG_DEBUG(postgis) << "postgis_cursor_resultset: " << s.str();

            conn_->execute(s.str());
            is_closed_ = true;
            conn_.reset();
        }
    }

    virtual int getNumFields() const { return rs_->getNumFields(); }

    virtual bool next()
    {
        if (rs_->next())
        {
            return true;
        }
        else if (rs_->size() == 0)
        {
            close();
            return false;
        }
        else
        {
            getNextResultSet();
            return rs_->next();
        }
    }

    virtual const char* getFieldName(int index) const { return rs_->getFieldName(index); }

    virtual int getFieldLength(int index) const { return rs_->getFieldLength(index); }

    virtual int getFieldLength(const char* name) const { return rs_->getFieldLength(name); }

    virtual int getTypeOID(int index) const { return rs_->getTypeOID(index); }

    virtual int getTypeOID(const char* name) const { return rs_->getTypeOID(name); }

    virtual bool isNull(int index) const { return rs_->isNull(index); }

    virtual const char* getValue(int index) const { return rs_->getValue(index); }

    virtual const char* getValue(const char* name) const { return rs_->getValue(name); }

  private:
    void getNextResultSet()
    {
        std::ostringstream s;
        s << "FETCH FORWARD " << fetch_size_ << " FROM " << cursorName_;

        MAPNIK_LOG_DEBUG(postgis) << "postgis_cursor_resultset: " << s.str();

        rs_ = conn_->executeQuery(s.str());
        is_closed_ = false;

        MAPNIK_LOG_DEBUG(postgis) << "postgis_cursor_resultset: FETCH result (" << cursorName_ << "): " << rs_->size()
                                  << " rows";
    }

    std::shared_ptr<Connection> conn_;
    std::string cursorName_;
    std::shared_ptr<ResultSet> rs_;
    int fetch_size_;
    bool is_closed_;
};

#endif // POSTGIS_CURSORRESULTSET_HPP
