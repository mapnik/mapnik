/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/debug.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/config_error.hpp>

// boost
#include <boost/make_shared.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

// ltdl
#include <ltdl.h>

// stl
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace mapnik {

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

datasource_ptr datasource_cache::create(const parameters& params, bool bind)
{
    boost::optional<std::string> type = params.get<std::string>("type");
    if ( ! type)
    {
        throw config_error(std::string("Could not create datasource. Required ") +
                           "parameter 'type' is missing");
    }

#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif

    datasource_ptr ds;
    std::map<std::string,boost::shared_ptr<PluginInfo> >::iterator itr=plugins_.find(*type);
    if ( itr == plugins_.end() )
    {
        std::ostringstream s;
        s << "Could not create datasource for type: '" << *type << "'";
        if (plugin_directories_.empty())
        {
            s << " (no datasource plugin directories have been successfully registered)";
        }
        else
        {
            s << " (searched for datasource plugins in '" << plugin_directories() << "')";
        }
        throw config_error(s.str());
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

    if (! create_datasource)
    {
        throw std::runtime_error(std::string("Cannot load symbols: ") +
                                 lt_dlerror());
    }

#ifdef MAPNIK_LOG
    MAPNIK_LOG_DEBUG(datasource_cache) << "datasource_cache: Size=" << params.size();

    parameters::const_iterator i = params.begin();
    for (; i != params.end(); ++i)
    {
        MAPNIK_LOG_DEBUG(datasource_cache) << "datasource_cache: -- " << i->first << "=" << i->second;
    }
#endif

    ds = datasource_ptr(create_datasource(params, bind), datasource_deleter());

    MAPNIK_LOG_DEBUG(datasource_cache) << "datasource_cache: Datasource=" << ds << " type=" << type;

    return ds;
}

bool datasource_cache::insert(std::string const& type,const lt_dlhandle module)
{
    return plugins_.insert(make_pair(type,boost::make_shared<PluginInfo>
                                     (type,module))).second;
}

std::string datasource_cache::plugin_directories()
{
    return boost::algorithm::join(plugin_directories_,", ");
}

std::vector<std::string> datasource_cache::plugin_names()
{
    std::vector<std::string> names;
    std::map<std::string,boost::shared_ptr<PluginInfo> >::const_iterator itr;
    for (itr = plugins_.begin();itr!=plugins_.end();++itr)
    {
        names.push_back(itr->first);
    }
    return names;
}

void datasource_cache::register_datasources(std::string const& str)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    boost::filesystem::path path(str);
    // TODO - only push unique paths
    plugin_directories_.push_back(str);
    boost::filesystem::directory_iterator end_itr;

    if (exists(path) && is_directory(path))
    {
        for (boost::filesystem::directory_iterator itr(path);itr!=end_itr;++itr )
        {

#if (BOOST_FILESYSTEM_VERSION == 3)
            if (!is_directory( *itr )  && is_input_plugin(itr->path().filename().string()))
#else // v2
                if (!is_directory( *itr )  && is_input_plugin(itr->path().leaf()))
#endif
                {
#if (BOOST_FILESYSTEM_VERSION == 3)
                    if (register_datasource(itr->path().string().c_str()))
#else // v2
                    if (register_datasource(itr->string().c_str()))
#endif
                    {
                        registered_ = true;
                    }
                }
        }
    }
}

bool datasource_cache::register_datasource(std::string const& str)
{
    bool success = false;
    try
    {
        lt_dlhandle module = lt_dlopen(str.c_str());
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
                MAPNIK_LOG_DEBUG(datasource_cache) << "datasource_cache: Registered=" << ds_name();

                success = true;
            }
            else if (!ds_name)
            {
                MAPNIK_LOG_ERROR(datasource_cache)
                        << "Problem loading plugin library '"
                        << str << "' (plugin is lacking compatible interface)";
            }
        }
        else
        {
            MAPNIK_LOG_ERROR(datasource_cache)
                    << "Problem loading plugin library: " << str
                    << " (dlopen failed - plugin likely has an unsatisfied dependency or incompatible ABI)";
        }
    }
    catch (...) {
            MAPNIK_LOG_ERROR(datasource_cache)
                    << "Exception caught while loading plugin library: " << str;
    }
    return success;
}

}
