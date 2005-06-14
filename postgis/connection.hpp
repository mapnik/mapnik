/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: connection.hpp 17 2005-03-08 23:58:43Z pavlenko $

#ifndef CONNECTION_HPP
#define CONNECTION_HPP


#include "libpq-fe.h"
#include "mapnik.hpp"
#include "resultset.hpp"

using namespace mapnik;

class ResultSet;

class Connection
{
private:
    PGconn *conn_;
public:
    Connection(const std::string& uri,const std::string& dbname, 
	       const std::string& username,const std::string& password)
    {
	std::string connStr="host="+uri+" dbname="+dbname+" user="+username+" password="+password;
	conn_=PQconnectdb(connStr.c_str());
	if (PQstatus(conn_) == CONNECTION_BAD)
	{
	    std::cerr << "connection to "<< connStr << " failed\n"
		      << PQerrorMessage(conn_)<< std::endl;
	}
	else
	{
	    std::cout <<"connected ok "<<std::endl;
	}
    }
    bool execute(const std::string& sql) const
    {
	PGresult *result=PQexec(conn_,sql.c_str());
	bool ok=(result && PQresultStatus(result)==PGRES_COMMAND_OK);
	PQclear(result);
	return ok;
    }
    ref_ptr<ResultSet> executeQuery(const std::string& sql,int type=0) const
    {
	PGresult *result=0;
	if (type==1)
	{
	    result=PQexecParams(conn_,sql.c_str(),0,0,0,0,0,1);
	    return ref_ptr<ResultSet>(new ResultSet(result));
	}
	result=PQexec(conn_,sql.c_str());
	return ref_ptr<ResultSet>(new ResultSet(result));
    }
    bool isOK() const
    {
	return (PQstatus(conn_)!=CONNECTION_BAD);
    }
    void close()
    {
	PQfinish(conn_);
    }
    ~Connection()
    {
	PQfinish(conn_);
    }
};

#endif //CONNECTION_HPP
