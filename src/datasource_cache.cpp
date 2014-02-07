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
#include <mapnik/params.hpp>
#include <mapnik/plugin.hpp>
#include <mapnik/util/fs.hpp>

// boost
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

// stl
#include <algorithm>
#include <map>
#include <stdexcept>

namespace mapnik {

extern datasource_ptr create_static_datasource(parameters const& params);
extern std::vector<std::string> get_static_datasource_names();

bool is_input_plugin(std::string const& filename)
{
    return boost::algorithm::ends_with(filename,std::string(".input"));
}

datasource_cache::datasource_cache()
{
    PluginInfo::init();
}

datasource_cache::~datasource_cache()
{
    PluginInfo::exit();
}

datasource_ptr datasource_cache::create(parameters const& params)
{
    boost::optional<std::string> type = params.get<std::string>("type");
    if ( ! type)
    {
        throw config_error(std::string("Could not create datasource. Required ") +
                           "parameter 'type' is missing");
    }

    datasource_ptr ds;

#ifdef MAPNIK_STATIC_PLUGINS
    // return if it's created, raise otherwise
    ds = create_static_datasource(params);
    if (ds)
    {
        return ds;
    }
#endif

#ifdef MAPNIK_THREADSAFE
    mapnik::scoped_lock lock(mutex_);
#endif

    std::map<std::string,std::shared_ptr<PluginInfo> >::iterator itr=plugins_.find(*type);
    if (itr == plugins_.end())
    {
        std::string s("Could not create datasource for type: '");
        s += *type + "'";
        if (plugin_directories_.empty())
        {
            s += " (no datasource plugin directories have been successfully registered)";
        }
        else
        {
            s += " (searched for datasource plugins in '" + plugin_directories() + "')";
        }
        throw config_error(s);
    }

    if (! itr->second->valid())
    {
        throw std::runtime_error(std::string("Cannot load library: ") +
                                 itr->second->get_error());
    }

    // http://www.mr-edd.co.uk/blog/supressing_gcc_warnings
#ifdef __GNUC__
    __extension__
#endif
        create_ds* create_datasource = reinterpret_cast<create_ds*>(itr->second->get_symbol("create"));

    if (! create_datasource)
    {
        throw std::runtime_error(std::string("Cannot load symbols: ") +
                                 itr->second->get_error());
    }

    ds = datasource_ptr(create_datasource(params), datasource_deleter());

#ifdef MAPNIK_LOG
    MAPNIK_LOG_DEBUG(datasource_cache)
        << "datasource_cache: Datasource="
        << ds << " type=" << type;

    MAPNIK_LOG_DEBUG(datasource_cache)
        << "datasource_cache: Size="
        << params.size();

    parameters::const_iterator i = params.begin();
    for (; i != params.end(); ++i)
    {
        MAPNIK_LOG_DEBUG(datasource_cache)
            << "datasource_cache: -- "
            << i->first << "=" << i->second;
    }
#endif

    return ds;
}

std::string datasource_cache::plugin_directories()
{
    return boost::algorithm::join(plugin_directories_,", ");
}

std::vector<std::string> datasource_cache::plugin_names()
{
    std::vector<std::string> names;

#ifdef MAPNIK_STATIC_PLUGINS
    names = get_static_datasource_names();
#endif

    std::map<std::string,std::shared_ptr<PluginInfo> >::const_iterator itr;
    for (itr = plugins_.begin(); itr != plugins_.end(); ++itr)
    {
        names.push_back(itr->first);
    }

    return names;
}

void datasource_cache::register_datasources(std::string const& str)
{
#ifdef MAPNIK_THREADSAFE
    mapnik::scoped_lock lock(mutex_);
#endif
    plugin_directories_.insert(str);
    if (mapnik::util::exists(str) && mapnik::util::is_directory(str))
    {
        boost::filesystem::directory_iterator end_itr;
        for (boost::filesystem::directory_iterator itr(str); itr != end_itr; ++itr )
        {

#if (BOOST_FILESYSTEM_VERSION == 3)
            if (!boost::filesystem::is_directory(*itr) && is_input_plugin(itr->path().filename().string()))
#else // v2
            if (!boost::filesystem::is_directory(*itr) && is_input_plugin(itr->path().leaf()))
#endif
            {
#if (BOOST_FILESYSTEM_VERSION == 3)
                if (register_datasource(itr->path().string()))
#else // v2
                if (register_datasource(itr->string()))
#endif
                {
                    registered_ = true;
                }
            }
        }
    }
}

bool datasource_cache::register_datasource(std::string const& filename)
{
    try
    {
        if (!mapnik::util::exists(filename))
        {
            MAPNIK_LOG_ERROR(datasource_cache)
                    << "Cannot load '"
                    << filename << "' (plugin does not exist)";
            return false;
        }
        std::shared_ptr<PluginInfo> plugin = std::make_shared<PluginInfo>(filename,"datasource_name");
        if (plugin->valid())
        {
            if (plugin->name().empty())
            {
                MAPNIK_LOG_ERROR(datasource_cache)
                        << "Problem loading plugin library '"
                        << filename << "' (plugin is lacking compatible interface)";
            }
            else
            {
                if (plugins_.insert(std::make_pair(plugin->name(),plugin)).second)
                {
                    MAPNIK_LOG_ERROR(datasource_cache)
                            << "datasource_cache: Registered="
                            << plugin->name();
                    return true;
                }
            }
        }
        else
        {
            MAPNIK_LOG_ERROR(datasource_cache)
                    << "Problem loading plugin library: "
                    << filename << " (dlopen failed - plugin likely has an unsatisfied dependency or incompatible ABI)";
        }
    }
    catch (std::exception const& ex)
    {
        MAPNIK_LOG_ERROR(datasource_cache)
                << "Exception caught while loading plugin library: "
                << filename << " (" << ex.what() << ")";
    }
    return false;
}

}
