/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2009 Artem Pavlenko
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

#include <mapnik/datasource.hpp>
#include <mapnik/wkb.hpp>

#include "connection_manager.hpp"
#include "cursorresultset.hpp"
// boost
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/program_options.hpp>

//stl
#include <iostream>
#include <fstream>

struct blob_to_hex
{
   std::string operator() (const char* blob, unsigned size)
   {
      std::string buf;
      buf.reserve(size*2);
      std::ostringstream s(buf);
      s.seekp(0);
      char hex[3];
      std::memset(hex,0,3);
      for ( unsigned pos=0; pos < size; ++pos)
      {  
         std::sprintf (hex, "%02X", int(blob[pos]) & 0xff);
         s << hex;
      }
      return s.str();
   }
};

bool valid_envelope(mapnik::Envelope<double> const& e)
{
   return (e.minx() < e.maxx() && e.miny() < e.maxy()) ;
}

template <typename Connection, typename OUT>
void pgsql2sqlite(Connection conn, std::string const& table_name, OUT & out, unsigned tolerance)
{
   using namespace mapnik;
   
   boost::shared_ptr<ResultSet> rs = conn->executeQuery("select * from " + table_name + " limit 0;");
   int count = rs->getNumFields();

   std::ostringstream select_sql;
   
   select_sql << "select ";
   
   for (int i=0; i<count; ++i)
   {
      if (i!=0) select_sql << ",";
      select_sql << "\"" <<  rs->getFieldName(i) << "\"";
   }
   
   select_sql << " from " << table_name ;
   
   std::ostringstream geom_col_sql;
   geom_col_sql << "select f_geometry_column,srid,type from geometry_columns ";
   geom_col_sql << "where f_table_name='" << table_name << "'";
   
   rs = conn->executeQuery(geom_col_sql.str());
   
   int srid = -1;
   std::string geom_col = "UNKNOWN";
   std::string geom_type = "UNKNOWN";
   
   if ( rs->next())
   {
      try 
      {
         srid = boost::lexical_cast<int>(rs->getValue("srid"));
      }
      catch (boost::bad_lexical_cast &ex)
      {
         std::clog << ex.what() << std::endl;
      }
      geom_col = rs->getValue("f_geometry_column");
      geom_type = rs->getValue("type");
   }
   
   // add AsBinary(<geometry_column>) modifier
   std::string select_sql_str = select_sql.str();
   if (tolerance > 0)
   {
      std::string from =  "\"" + geom_col + "\"";
      std::string to   = (boost::format("AsBinary(Simplify(%1%,%2%)) as %1%") % geom_col % tolerance).str();
      boost::algorithm::replace_all(select_sql_str,from ,to);
   }
   else
   {
      boost::algorithm::replace_all(select_sql_str, "\"" + geom_col + "\"","AsBinary(" + geom_col+") as " + geom_col);
   }
   
   std::cout << select_sql_str << "\n";
   
   std::ostringstream cursor_sql;
   std::string cursor_name("my_cursor");
   
   cursor_sql << "DECLARE " << cursor_name << " BINARY INSENSITIVE NO SCROLL CURSOR WITH HOLD FOR " << select_sql_str << " FOR READ ONLY";
   conn->execute(cursor_sql.str());
   
   boost::shared_ptr<CursorResultSet> cursor(new CursorResultSet(conn,cursor_name,10000));
   
   unsigned num_fields = cursor->getNumFields();
   
   std::ostringstream create_sql;
   create_sql << "create table " << table_name << "(PK_UID INTEGER PRIMARY KEY AUTOINCREMENT,";
   
   int geometry_oid = -1;
   
   for ( unsigned pos = 0; pos < num_fields ; ++pos)
   {
      if (pos > 0) create_sql << ",";
      if (geom_col == cursor->getFieldName(pos))
      {
         geometry_oid = cursor->getTypeOID(pos);
         create_sql << "'" << cursor->getFieldName(pos) << "' BLOB";
      }
      else
      {
         create_sql << "'" << cursor->getFieldName(pos) << "' TEXT";
      }
   }
   
   create_sql << ");";
   
   
   std::cout << "client_encoding=" << conn->client_encoding() << "\n";
   std::cout << "geometry_column=" << geom_col << "(" << geom_type 
             <<  ") srid=" << srid << " oid=" << geometry_oid << "\n";
   
   
   // begin
   out << "begin;\n";
   
   out << create_sql.str() << "\n";
   
   // spatial index sql
   out << "create virtual table idx_"<< table_name << "_" << geom_col << " using rtree(pkid, xmin, xmax, ymin, ymax);\n";
   
   blob_to_hex hex;
   int pkid = 0;
   while (cursor->next())
   {
      ++pkid;
      
      std::ostringstream insert_sql;
      insert_sql << "insert into " <<  table_name << " values(" << pkid;

      bool empty_geom = true;
      
      for (unsigned pos=0 ; pos < num_fields; ++pos)
      {
         insert_sql << ",";
         if (! cursor->isNull(pos))
         {
            int size=cursor->getFieldLength(pos);
            int oid = cursor->getTypeOID(pos);
            const char* buf=cursor->getValue(pos);
            
            switch (oid)
            {
               case 25:
               case 1042:
               case 1043:
               {
                  std::string text(buf);
                  boost::algorithm::replace_all(text,"'","''");
                  insert_sql << "'"<< text << "'"; 
                  break;
               }
               case 23:
                  insert_sql << int4net(buf);
                  break;
               default:  
               {
                  if (oid == geometry_oid)
                  {
                     mapnik::Feature feat(pkid);
                     geometry_utils::from_wkb(feat,buf,size,false,wkbGeneric);
                     if (feat.num_geometries() > 0)
                     {
                        geometry2d const& geom=feat.get_geometry(0);
                        Envelope<double> bbox = geom.envelope();
                        if (valid_envelope(bbox))
                        {
                           out << "insert into idx_" << table_name << "_" << geom_col << " values (" ;
                           out << pkid << "," << bbox.minx() << "," << bbox.maxx();
                           out << "," << bbox.miny() << "," << bbox.maxy() << ");\n";
                           empty_geom = false;
                        }
                     }
                     
                     insert_sql << "X'" << hex(buf,size) << "'";
                     
                  }
                  else 
                  {
                     insert_sql << "NULL";
                  }
                  break;
               }
            }    
         }
         else 
         {
            insert_sql << "NULL";
         } 
      }
      insert_sql << ");";
      
      if (!empty_geom) out << insert_sql.str() << "\n";
      
      if (pkid % 1000 == 0)
      {
         std::cout << "\r processing " << pkid << " features";
         std::cout.flush();
      }
      if (pkid % 100000 == 0)
      {
         out << "commit;\n";
         out << "begin;\n";
      }
   }
   // commit
   out << "commit;\n";
   std::cout << "\r processed " << pkid << " features";
   std::cout << "\n Done!" << std::endl;
}


int main ( int argc, char** argv)
{
   namespace po = boost::program_options;
     
   po::options_description desc("Postgresql/PostGIS to SQLite3 converter\n Options");
   
   desc.add_options()
      ("help,?","Display this help screen.")
      ("host,h",po::value<std::string>(),"Allows you to specify connection to a database on a machine other than the default.")
      ("port,p",po::value<std::string>(),"Allows you to specify a database port other than the default.")
      ("user,u",po::value<std::string>(),"Connect to the database as the specified user.")
      ("dbname,d",po::value<std::string>(),"postgresql database name")
      ("password,P",po::value<std::string>(),"Connect to the database with the specified password.")
      ("table,t",po::value<std::string>(),"Name of the table to export")
      ("simplify,s",po::value<unsigned>(),"Use this option to reduce the complexity\nand weight of a geometry using the Douglas-Peucker algorithm.")
      ("file,f",po::value<std::string>(),"Use this option to specify the name of the file to create.")
      ;
   
   po::positional_options_description p;
   p.add("table",1);
   
   po::variables_map vm;
   
   try 
   {     
      po::store(po::command_line_parser(argc,argv).options(desc).positional(p).run(),vm);
      po::notify(vm);
      
      if (vm.count("help") || !vm.count("file") || !vm.count("table"))
      {
         std::cout << desc << "\n";
         return EXIT_SUCCESS;
      }
   }
   catch (...)
   {
      std::cout << desc << "\n";
      return EXIT_FAILURE;
   }
   
   boost::optional<std::string> host;
   boost::optional<std::string> port ;
   boost::optional<std::string> dbname;
   boost::optional<std::string> user;
   boost::optional<std::string> password;
   
   if (vm.count("host")) host = vm["host"].as<std::string>();
   if (vm.count("port")) port = vm["port"].as<std::string>();
   if (vm.count("dbname")) dbname = vm["dbname"].as<std::string>();
   if (vm.count("user")) user = vm["user"].as<std::string>();
   if (vm.count("password")) password = vm["password"].as<std::string>();
   unsigned tolerance = 0;
   if (vm.count("simplify")) tolerance = vm["simplify"].as<unsigned>();
   
   ConnectionCreator<Connection> creator(host,port,dbname,user,password);
   try 
   {
      boost::shared_ptr<Connection> conn(creator());
      
      std::string table_name = vm["table"].as<std::string>();
      std::string output_file = vm["file"].as<std::string>();
      
      std::ofstream file(output_file.c_str());
      if (file)
      {
         pgsql2sqlite(conn,table_name,file,tolerance);
      }
      file.close();
   }
   catch (mapnik::datasource_exception & ex)
   {
      std::cerr << ex.what() << "\n";
   }
   
   return EXIT_SUCCESS;
}
