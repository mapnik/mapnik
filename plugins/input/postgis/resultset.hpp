/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id: resultset.hpp 17 2005-03-08 23:58:43Z pavlenko $


#ifndef RESULTSET_HPP
#define RESULTSET_HPP

#include "connection.hpp"

class ResultSet
{
private:
    PGresult* res_;
    int pos_;
    int numTuples_;
    int *refCount_;
public:
    ResultSet(PGresult *res)
	:res_(res),pos_(-1),refCount_(new int(1))
    {
	numTuples_=PQntuples(res_);
    }

    ResultSet(const ResultSet& rhs)
	:res_(rhs.res_),
	 pos_(rhs.pos_),
	 numTuples_(rhs.numTuples_),
	 refCount_(rhs.refCount_)
    {
	(*refCount_)++;
    }

    ResultSet& operator=(const ResultSet& rhs)
    {
	if (this==&rhs) return *this;
	if (--(refCount_)==0)
	{
	    close();
	    delete refCount_,refCount_=0;
	}
	res_=rhs.res_;
	pos_=rhs.pos_;
	numTuples_=rhs.numTuples_;
	refCount_=rhs.refCount_;
	(*refCount_)++;
	return *this;
    }

    void close()
    {
	PQclear(res_),res_=0;
    }

    ~ResultSet()
    {
	if (--(*refCount_)==0)
	{
	    PQclear(res_);
	    delete refCount_,refCount_=0;
	}
    }

    int getNumFields() const
    {
	return PQnfields(res_);
    }

    bool next()
    {
	return (++pos_<numTuples_);
    }

    const char* getFieldName(int index) const
    {
	return PQfname(res_,index);
    }

    int getFieldLength(int index) const
    {
	return PQgetlength(res_,pos_,index);
    }

    int getFieldLength(const char* name) const
    {
	int col=PQfnumber(res_,name);
	if (col>=0)
	    return PQgetlength(res_,pos_,col);
	return 0;
    }

    int getTypeOID(int index) const
    {
	return PQftype(res_,index);
    }

    int getTypeOID(const char* name) const
    {
	int col=PQfnumber(res_,name);
	if (col>=0)
	    return PQftype(res_,col);
	return 0;
    }
    
    const char* getValue(int index) const
    {
	return PQgetvalue(res_,pos_,index);
    }

    const char* getValue(const char* name) const
    {
	int col=PQfnumber(res_,name);
	if (col>=0)
	    return getValue(col);
	return 0;
    }
};

#endif //RESULTSET_HPP



