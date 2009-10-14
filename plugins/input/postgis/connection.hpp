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

#include <mapnik/datasource.hpp>

extern "C" 
{
#include "libpq-fe.h"
}

#include "resultset.hpp"

class ResultSet;
class Connection
{
   private:
      PGconn *conn_;
      int cursorId;
      bool closed_;
   public:
      Connection(std::string const& connection_str)
         :cursorId(0),
         closed_(false)
      {
         conn_=PQconnectdb(connection_str.c_str());
         if (PQstatus(conn_) != CONNECTION_OK)
         {
             std::string s("PSQL error");
             if (conn_ )
             {
                 std::string msg = PQerrorMessage( conn_ );
                 if ( ! msg.empty() )
                 {
                     s += ":\n" + msg.substr( 0, msg.size() - 1 );
                 }
             } 
             throw mapnik::datasource_exception( s );
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
         }
         else
         {
             result=PQexec(conn_,sql.c_str());
         }
         if(!result || PQresultStatus(result) != PGRES_TUPLES_OK)
         {
             std::string s("PSQL error");
             if (conn_ )
             {
                 std::string msg = PQerrorMessage( conn_ );
                 if ( ! msg.empty() )
                 {
                     s += ":\n" + msg.substr( 0, msg.size() - 1 );
                 }
                 
                 s += "\nFull sql was: '" + sql + "'\n";
             } 
             throw mapnik::datasource_exception( s );
         }

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
         if (!closed_)
         {
             PQfinish(conn_);
#ifdef MAPNIK_DEBUG
             std::clog << "PostGIS: datasource closed, also closing connection - " << conn_ << "\n";
#endif
             closed_ = true;
         }
      }
      
      std::string new_cursor_name()
      {
          std::ostringstream s;
          s << "mapnik_" << (cursorId++);
          return s.str();
      }
      
      ~Connection()
      {
         if (!closed_)
         {
             PQfinish(conn_);
#ifdef MAPNIK_DEBUG
             std::clog << "PostGIS: postgresql connection closed - " << conn_ << "\n";
#endif
             closed_ = true;
         }
      }
};

#endif //CONNECTION_HPP
