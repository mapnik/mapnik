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

//$Id: connection.hpp 17 2005-03-08 23:58:43Z pavlenko $

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

extern "C" 
{
#include "libpq-fe.h"
}

#include "resultset.hpp"

using namespace mapnik;

class ResultSet;

class Connection
{
   private:
      PGconn *conn_;
   public:
      Connection(std::string const& host, 
                 std::string const& port,
                 std::string const& dbname, 
                 std::string const& username,
                 std::string const& password)
      {
         
         std::string connStr;
         if (host.length()) connStr += "host="+host;
         if (port.length()) connStr += " port="+port;
         connStr+=" dbname="+dbname;
         connStr+=" user="+username;
         connStr+=" password="+password;
         connStr+=" connect_timeout=4"; // todo: set by client (param) 
         
         conn_=PQconnectdb(connStr.c_str());
         if (PQstatus(conn_) == CONNECTION_BAD)
         {
            std::clog << "connection to "<< connStr << " failed\n"
                      << PQerrorMessage(conn_)<< std::endl;
         }
      }
    
      bool execute(const std::string& sql) const
      {
         PGresult *result=PQexec(conn_,sql.c_str());
         bool ok=(result && PQresultStatus(result)==PGRES_COMMAND_OK);
         PQclear(result);
         return ok;
      }
      boost::shared_ptr<ResultSet> executeQuery(const std::string& sql,int type=0) const
      {
         PGresult *result=0;
         if (type==1)
         {
            result=PQexecParams(conn_,sql.c_str(),0,0,0,0,0,1);
            return boost::shared_ptr<ResultSet>(new ResultSet(result));
         }
         result=PQexec(conn_,sql.c_str());
         return boost::shared_ptr<ResultSet>(new ResultSet(result));
      }
      
      std::string client_encoding() const
      {
         return PQparameterStatus(conn_,"client_encoding");
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
#ifdef MAPNIK_DEBUG
         std::clog << "close connection " << conn_ << "\n";
#endif 
      }
};

#endif //CONNECTION_HPP
