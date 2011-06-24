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

// mapnik
#include <mapnik/datasource_cache.hpp>

#include <mapnik/config_error.hpp>

// boost
#include <boost/version.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

// ltdl
#include <ltdl.h>

// stl
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace mapnik
{
   
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

std::map<std::string,boost::shared_ptr<PluginInfo> > datasource_cache::plugins_;
bool datasource_cache::registered_=false;
std::vector<std::string> datasource_cache::plugin_directories_;
    
datasource_ptr datasource_cache::create(const parameters& params, bool bind) 
{
    boost::optional<std::string> type = params.get<std::string>("type");
    if ( ! type)
    {
        throw config_error(std::string("Could not create datasource. Required ") +
                           "parameter 'type' is missing");
    }

    datasource_ptr ds;
    std::map<std::string,boost::shared_ptr<PluginInfo> >::iterator itr=plugins_.find(*type);
    if ( itr == plugins_.end() )
    {
        throw config_error(std::string("Could not create datasource. No plugin ") +
                           "found for type '" + * type + "' (searched in: " + plugin_directories() + ")");
    }
    if ( ! itr->second->handle())
    {
        throw std::runtime_error(std::string("Cannot load library: ") +
                                 lt_dlerror());
    }
    // http://www.mr-edd.co.uk/blog/supressing_gcc_warnings
    #ifdef __GNUC__
    __extension__
    #endif
    create_ds* create_datasource = 
        reinterpret_cast<create_ds*>(lt_dlsym(itr->second->handle(), "create"));

    if ( ! create_datasource)
    {
        throw std::runtime_error(std::string("Cannot load symbols: ") +
                                 lt_dlerror());
    }
#ifdef MAPNIK_DEBUG
    std::clog << "size = " << params.size() << "\n";
    parameters::const_iterator i = params.begin();
    for (;i!=params.end();++i)
    {
        std::clog << i->first << "=" << i->second << "\n";
    }
#endif
    ds=datasource_ptr(create_datasource(params, bind), datasource_deleter());

#ifdef MAPNIK_DEBUG
    std::clog<<"datasource="<<ds<<" type="<<type<<std::endl;
#endif
    return ds;
}

bool datasource_cache::insert(const std::string& type,const lt_dlhandle module)
{
    return plugins_.insert(make_pair(type,boost::make_shared<PluginInfo>
                                     (type,module))).second;
}

std::string datasource_cache::plugin_directories()
{
    return boost::algorithm::join(plugin_directories_,", ");
}

std::vector<std::string> datasource_cache::plugin_names ()
{
    std::vector<std::string> names;
    std::map<std::string,boost::shared_ptr<PluginInfo> >::const_iterator itr;
    for (itr = plugins_.begin();itr!=plugins_.end();++itr)
    {
        names.push_back(itr->first);
    }
    return names;
}
   
void datasource_cache::register_datasources(const std::string& str)
{       
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mapnik::singleton<mapnik::datasource_cache,
                            mapnik::CreateStatic>::mutex_);
#endif
    boost::filesystem::path path(str);
    plugin_directories_.push_back(str);
    boost::filesystem::directory_iterator end_itr;
 
    if (exists(path) && is_directory(path))
    {
        for (boost::filesystem::directory_iterator itr(path);itr!=end_itr;++itr )
        {

#if BOOST_VERSION < 103400 
            if (!is_directory( *itr )  && is_input_plugin(itr->leaf()))
#else
#if (BOOST_FILESYSTEM_VERSION == 3)      
            if (!is_directory( *itr )  && is_input_plugin(itr->path().filename().string()))
#else // v2
            if (!is_directory( *itr )  && is_input_plugin(itr->path().leaf())) 
#endif 
#endif
            {
                try 
                {
#ifdef LIBTOOL_SUPPORTS_ADVISE
                    // with ltdl >=2.2 we can actually pass RTDL_GLOBAL to dlopen via the
                    // ltdl advise trick which is required on linux unless plugins are directly
                    // linked to libmapnik (and deps) at build time. The only other approach is to
                    // set the dlopen flags in the calling process (like in the python bindings)

                    // clear errors
                    lt_dlerror();

                    lt_dlhandle module = 0;
                    lt_dladvise advise;
                    int ret;
                
                    ret = lt_dlinit();
                    if (ret != 0) {
                        std::clog << "Datasource loader: could not intialize dynamic loading: " << lt_dlerror() << "\n";
                    }
                
                    ret = lt_dladvise_init(&advise);
                    if (ret != 0) {
                        std::clog << "Datasource loader: could not intialize dynamic loading: " << lt_dlerror() << "\n";
                    }
                
                    ret = lt_dladvise_global(&advise);
                    if (ret != 0) {
                        std::clog << "Datasource loader: could not intialize dynamic loading of global symbols: " << lt_dlerror() << "\n";
                    }
#if (BOOST_FILESYSTEM_VERSION == 3)                    
                    module = lt_dlopenadvise (itr->path().string().c_str(), advise);
#else // v2
                    module = lt_dlopenadvise (itr->string().c_str(), advise);
#endif 

                    lt_dladvise_destroy(&advise);
#else

#if (BOOST_FILESYSTEM_VERSION == 3)   
                    lt_dlhandle module = lt_dlopen(itr->path().string().c_str());
#else // v2
                    lt_dlhandle module = lt_dlopen(itr->string().c_str());
#endif

#endif
                    if (module)
                    {
                        // http://www.mr-edd.co.uk/blog/supressing_gcc_warnings
                        #ifdef __GNUC__
                        __extension__
                        #endif
                        datasource_name* ds_name = 
                            reinterpret_cast<datasource_name*>(lt_dlsym(module, "datasource_name"));
                        if (ds_name && insert(ds_name(),module))
                        {            
#ifdef MAPNIK_DEBUG
                            std::clog << "Datasource loader: registered: " << ds_name() << std::endl;
#endif 
                            registered_=true;
                        }
                    }
                    else
                    {
#if (BOOST_FILESYSTEM_VERSION == 3) 
                        std::clog << "Problem loading plugin library: " << itr->path().string() 
                                  << " (dlopen failed - plugin likely has an unsatisfied dependency or incompatible ABI)" << std::endl;
#else // v2
                        std::clog << "Problem loading plugin library: " << itr->string() 
                                  << " (dlopen failed - plugin likely has an unsatisfied dependency or incompatible ABI)" << std::endl;    
#endif
                    }
                }
                catch (...) {}
            }
        }
    }
}
}
