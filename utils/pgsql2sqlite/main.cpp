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

// mapnik
#include "pgsql2sqlite.hpp"
#include <mapnik/datasource.hpp>
#include <mapnik/wkb.hpp>
#include "connection_manager.hpp"

// boost
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>

//stl
#include <iostream>
#include <fstream>
#include <exception>

int main ( int argc, char** argv)
{

    namespace po = boost::program_options;
    po::options_description desc("Postgresql/PostGIS to SQLite3 converter\n Options");
    std::string usage = "usage: pgsql2sqlite --dbname db --table planet_osm_line --file osm.sqlite --query \"select * from planet_osm_line\"";
    try
    {

        desc.add_options()
            ("help,?","Display this help screen.")
            ("host,h",po::value<std::string>(),"Allows you to specify connection to a database on a machine other than the default.")
            ("port,p",po::value<std::string>(),"Allows you to specify a database port other than the default.")
            ("user,u",po::value<std::string>(),"Connect to the database as the specified user.")
            ("dbname,d",po::value<std::string>(),"postgresql database name")
            ("password,P",po::value<std::string>(),"Connect to the database with the specified password.")
            ("query,q",po::value<std::string>(),"Name of the table/or query to pass to postmaster")
            ("table,t",po::value<std::string>(),"Name of the output table to create (default: table in query)")
            ("file,f",po::value<std::string>(),"Use this option to specify the name of the file to create.")

            ;

        //po::positional_options_description p;
        //p.add("table",1);

        po::variables_map vm;
        //positional(p)
        po::store(po::command_line_parser(argc,argv).options(desc).run(),vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << "\n";
            std::cout << usage << "\n";
            return EXIT_SUCCESS;
        }
        else if ( !vm.count("dbname") || !vm.count("file") || !vm.count("query") )
        {
            std::cout << desc << "\n";
            std::cout << usage << "\n";
            std::cout << "Both --dbname, --file and, --query are required\n";
            return EXIT_FAILURE;
        }

        boost::optional<std::string> host;
        boost::optional<std::string> port ;
        boost::optional<std::string> dbname;
        boost::optional<std::string> user;
        boost::optional<std::string> password;
        boost::optional<std::string> connect_timeout("4");

        if (vm.count("host")) host = vm["host"].as<std::string>();
        if (vm.count("port")) port = vm["port"].as<std::string>();
        if (vm.count("dbname")) dbname = vm["dbname"].as<std::string>();
        if (vm.count("user")) user = vm["user"].as<std::string>();
        if (vm.count("password")) password = vm["password"].as<std::string>();

        ConnectionCreator<Connection> creator(host,port,dbname,user,password,connect_timeout);
        try
        {
            boost::shared_ptr<Connection> conn(creator());

            std::string query = vm["query"].as<std::string>();
            std::string output_table_name = vm.count("table") ? vm["table"].as<std::string>() : mapnik::sql_utils::table_from_sql(query);
            std::string output_file = vm["file"].as<std::string>();

            std::cout << "output_table : " << output_table_name << "\n";

            mapnik::pgsql2sqlite(conn,query,output_table_name,output_file);
        }
        catch (mapnik::datasource_exception & ex)
        {
            std::cerr << ex.what() << "\n";
        }

    }
    catch(std::exception& e) {
        std::cerr << desc << "\n";
        std::cout << usage << "\n";
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
