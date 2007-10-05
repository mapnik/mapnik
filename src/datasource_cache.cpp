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
//$Id: datasource_cache.cpp 23 2005-03-22 22:16:34Z pavlenko $

// stl
#include <algorithm>
#include <stdexcept>
// boost
#include <boost/thread/mutex.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
// mapnik
#include <mapnik/datasource_cache.hpp>
#include <mapnik/config_error.hpp>
// ltdl
#include <ltdl.h>

namespace mapnik
{
   using namespace std;
   using namespace boost;
   
   bool is_input_plugin (std::string const& filename)
   {
      return boost::algorithm::ends_with(filename,std::string(".input"));
   }
   

   datasource_cache::datasource_cache()
   {
      if (lt_dlinit()) throw std::runtime_error("lt_dlinit() failed");
   }

   datasource_cache::~datasource_cache()
   {
      lt_dlexit();
   }

   std::map<string,boost::shared_ptr<PluginInfo> > datasource_cache::plugins_;
   bool datasource_cache::registered_=false;
    
   datasource_ptr datasource_cache::create(const parameters& params) 
   {
       boost::optional<std::string> type = params.get<std::string>("type");
       if ( ! type)
       {
           throw config_error(string("Could not create datasource. Required ") +
                   "parameter 'type' is missing");
       }

       datasource_ptr ds;
       map<string,boost::shared_ptr<PluginInfo> >::iterator itr=plugins_.find(*type);
       if ( itr == plugins_.end() )
       {
           throw config_error(string("Could not create datasource. No plugin ") +
                   "found for type '" + * type + "'");
       }
       if ( ! itr->second->handle())
       {
           throw std::runtime_error(string("Cannot load library: ") + 
                   lt_dlerror());
       }

       create_ds* create_datasource = 
           (create_ds*) lt_dlsym(itr->second->handle(), "create");

       if ( ! create_datasource)
       {
           throw std::runtime_error(string("Cannot load symbols: ") + 
                   lt_dlerror());
       }
       std::cout << "size = " << params.size() << "\n";
       parameters::const_iterator i = params.begin();
       for (;i!=params.end();++i)
       {
          std::cout << i->first << "=" << i->second << "\n";  
       }
       ds=datasource_ptr(create_datasource(params), datasource_deleter());

#ifdef MAPNIK_DEBUG
       std::clog<<"datasource="<<ds<<" type="<<type<<std::endl;
#endif
       return ds;
   }

   bool datasource_cache::insert(const std::string& type,const lt_dlhandle module)
   {	      
      return plugins_.insert(make_pair(type,boost::shared_ptr<PluginInfo>
                                       (new PluginInfo(type,module)))).second;     
   }

   void datasource_cache::register_datasources(const std::string& str)
   {	
      mutex::scoped_lock lock(mapnik::singleton<mapnik::datasource_cache, 
                              mapnik::CreateStatic>::mutex_);
      filesystem::path path(str);
      filesystem::directory_iterator end_itr;
      if (exists(path) && is_directory(path))
      {
         for (filesystem::directory_iterator itr(path);itr!=end_itr;++itr )
         {
            if (!is_directory( *itr )  && is_input_plugin(itr->leaf()))
            {
               try 
               {
                  lt_dlhandle module=lt_dlopen(itr->string().c_str());
                  if (module)
                  {
                     datasource_name* ds_name = 
                        (datasource_name*) lt_dlsym(module, "datasource_name");
                     if (ds_name && insert(ds_name(),module))
                     {            
#ifdef MAPNIK_DEBUG
                        std::clog<<"registered datasource : "<<ds_name()<<std::endl;
#endif 
                        registered_=true;
                     }
                  }
                  else
                  {
                     std::clog << lt_dlerror() << "\n";
                  }
               }
               catch (...) {}
            }
         }   
      }	
   }
}
