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

#ifndef POSTGIS_RESULTSET_HPP
#define POSTGIS_RESULTSET_HPP

extern "C" {
#include "libpq-fe.h"
}

class IResultSet
{
public:
    //virtual IResultSet& operator=(const IResultSet& rhs) = 0;
    virtual ~IResultSet() {}
    virtual void close() = 0;
    virtual int getNumFields() const = 0;
    virtual bool next() = 0;
    virtual const char* getFieldName(int index) const = 0;
    virtual int getFieldLength(int index) const = 0;
    virtual int getFieldLength(const char* name) const = 0;
    virtual int getTypeOID(int index) const = 0;
    virtual int getTypeOID(const char* name) const = 0;
    virtual bool isNull(int index) const = 0;
    virtual const char* getValue(int index) const = 0;
    virtual const char* getValue(const char* name) const = 0;
};

class ResultSet : public IResultSet, private mapnik::util::noncopyable
{
public:
    ResultSet(PGresult *res)
      : res_(res),
        pos_(-1)
    {
        numTuples_ = PQntuples(res_);
    }

    virtual void close()
    {
        PQclear(res_);
        res_ = 0;
    }

    virtual ~ResultSet()
    {
        PQclear(res_);
    }

    virtual int getNumFields() const
    {
        return PQnfields(res_);
    }

    int pos() const
    {
        return pos_;
    }

    int size() const
    {
        return numTuples_;
    }

    virtual bool next()
    {
        return (++pos_ < numTuples_);
    }

    virtual const char* getFieldName(int index) const
    {
        return PQfname(res_, index);
    }

    virtual int getFieldLength(int index) const
    {
        return PQgetlength(res_, pos_, index);
    }

    virtual int getFieldLength(const char* name) const
    {
        int col = PQfnumber(res_, name);
        if (col >= 0)
        {
            return PQgetlength(res_, pos_, col);
        }
        return 0;
    }

    virtual int getTypeOID(int index) const
    {
        return PQftype(res_, index);
    }

    virtual int getTypeOID(const char* name) const
    {
        int col = PQfnumber(res_, name);
        if (col >= 0)
        {
            return PQftype(res_, col);
        }
        return 0;
    }

    virtual bool isNull(int index) const
    {
        return static_cast<bool>(PQgetisnull(res_, pos_, index));
    }

    virtual const char* getValue(int index) const
    {
        return PQgetvalue(res_, pos_, index);
    }

    virtual const char* getValue(const char* name) const
    {
        int col = PQfnumber(res_, name);
        if (col >= 0)
        {
            return getValue(col);
        }
        return 0;
    }

private:
    PGresult* res_;
    int pos_;
    int numTuples_;
};

#endif // POSTGIS_RESULTSET_HPP
