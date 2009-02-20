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

#include "pgsql2sqlite.hpp"
#include "sqlite.hpp"

#include <mapnik/datasource.hpp>
#include <mapnik/wkb.hpp>

#include "connection_manager.hpp"
//#include "cursorresultset.hpp"
// boost
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
//#include <boost/algorithm/string/replace.hpp>
#include <boost/program_options.hpp>

//stl
#include <iostream>
#include <fstream>

int main ( int argc, char** argv)
{

   namespace sqlite = mapnik::sqlite;
   
   sqlite::database db("/tmp/testing.sqlite");
   
   db.execute("create table test(id integer, name text)");
   
   sqlite::prepared_statement stmt(db,"insert into test values(?,?)");
   for (unsigned i=0;i<1000;++i)
   {
      sqlite::record_type rec;
      rec.push_back(sqlite::value_type(int(i)));
      rec.push_back(sqlite::value_type("testing ...."));
      stmt.insert_record(rec);
   }
   
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
         mapnik::pgsql2sqlite(conn,table_name,file,tolerance);
      }
      
      file.close();
   }
   catch (mapnik::datasource_exception & ex)
   {
      std::cerr << ex.what() << "\n";
   }
   
   return EXIT_SUCCESS;
}
