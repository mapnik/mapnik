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
// mapnik
#include <mapnik/datasource_cache.hpp>
// ltdl
#include <ltdl.h>

namespace mapnik
{
    using namespace std;
    using namespace boost;
     
    datasource_cache::datasource_cache()
    {
        if (lt_dlinit()) throw;
    }

    datasource_cache::~datasource_cache()
    {
        lt_dlexit();
    }

    std::map<string,boost::shared_ptr<PluginInfo> > datasource_cache::plugins_;
    bool datasource_cache::registered_=false;
    
    datasource_ptr datasource_cache::create(const parameters& params)
    {
        datasource_ptr ds;
        try
        {
            const std::string type=params.get("type");	    
            map<string,boost::shared_ptr<PluginInfo> >::iterator itr=plugins_.find(type);
            if (itr!=plugins_.end())
            {
                if (itr->second->handle())
                {
                    create_ds* create_datasource = 
                        (create_ds*) lt_dlsym(itr->second->handle(), "create");
                    if (!create_datasource)
                    {
                        std::clog << "Cannot load symbols: " << lt_dlerror() << std::endl;
                    }
                    else
                    {
                        ds=datasource_ptr(create_datasource(params),datasource_deleter());
                    }
                }
                else
                {
                    std::clog << "Cannot load library: " << "  "<< lt_dlerror() << std::endl;
                }
            }
#ifdef MAPNIK_DEBUG
            std::clog<<"datasource="<<ds<<" type="<<type<<std::endl;
#endif
        }
        catch (datasource_exception& ex)
        {
            std::clog << ex.what() << "\n";
        }
        catch (...)
        {
            std::clog << " exception caught\n";
        }
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
                if (!is_directory( *itr ) && itr->leaf()[0]!='.')
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
