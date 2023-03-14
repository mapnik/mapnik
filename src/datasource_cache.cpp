/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/datasource_plugin.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/params.hpp>
#include <mapnik/plugin.hpp>
#include <mapnik/util/fs.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <algorithm>
#include <map>
#include <stdexcept>

namespace mapnik {

template class singleton<datasource_cache, CreateStatic>;

extern datasource_ptr create_static_datasource(parameters const& params);
extern std::vector<std::string> get_static_datasource_names();

bool is_input_plugin(std::string const& filename)
{
    return boost::algorithm::ends_with(filename, std::string(".input"));
}

datasource_cache::datasource_cache() {}
datasource_cache::~datasource_cache() {}

datasource_ptr datasource_cache::create(parameters const& params)
{
    boost::optional<std::string> type = params.get<std::string>("type");
    if (!type)
    {
        throw config_error(std::string("Could not create datasource. Required ") + "parameter 'type' is missing");
    }

#ifdef MAPNIK_STATIC_PLUGINS
    // return if it's created, raise otherwise
    datasource_ptr ds = create_static_datasource(params);
    if (ds)
    {
        return ds;
    }
#endif

    std::map<std::string, std::shared_ptr<PluginInfo>>::iterator itr;
    // add scope to ensure lock is released asap
    {
#ifdef MAPNIK_THREADSAFE
        std::lock_guard<std::recursive_mutex> lock(instance_mutex_);
#endif
        itr = plugins_.find(*type);
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
    }

    if (!itr->second->valid())
    {
        throw std::runtime_error(std::string("Cannot load library: ") + itr->second->get_error());
    }

    // http://www.mr-edd.co.uk/blog/supressing_gcc_warnings
#ifdef __GNUC__
    __extension__
#endif
      datasource_plugin* create_datasource = reinterpret_cast<datasource_plugin*>(itr->second->get_symbol("plugin"));

    if (!create_datasource)
    {
        throw std::runtime_error(std::string("Cannot load symbols: ") + itr->second->get_error());
    }

    return create_datasource->create(params);
}

std::string datasource_cache::plugin_directories()
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::recursive_mutex> lock(instance_mutex_);
#endif
    return boost::algorithm::join(plugin_directories_, ", ");
}

bool datasource_cache::plugin_registered(const std::string& plugin_name) const
{
#ifdef MAPNIK_STATIC_PLUGINS
    const auto static_names = get_static_datasource_names();
    const auto static_it = std::find(static_names.begin(), static_names.end(), plugin_name);
    if (static_it != static_names.end())
        return true;
#endif

#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::recursive_mutex> lock(instance_mutex_);
#endif

    std::map<std::string, std::shared_ptr<PluginInfo>>::const_iterator itr;
    for (itr = plugins_.begin(); itr != plugins_.end(); ++itr)
    {
        if (itr->second->name() == plugin_name)
            return true;
    }
    return false;
}

std::vector<std::string> datasource_cache::plugin_names() const
{
    std::vector<std::string> names;

#ifdef MAPNIK_STATIC_PLUGINS
    names = get_static_datasource_names();
#endif

#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::recursive_mutex> lock(instance_mutex_);
#endif

    std::map<std::string, std::shared_ptr<PluginInfo>>::const_iterator itr;
    for (itr = plugins_.begin(); itr != plugins_.end(); ++itr)
    {
        names.push_back(itr->first);
    }

    return names;
}

bool datasource_cache::register_datasources(std::string const& dir, bool recurse)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::recursive_mutex> lock(instance_mutex_);
#endif
    if (!mapnik::util::exists(dir))
    {
        return false;
    }
    plugin_directories_.insert(dir);
    if (!mapnik::util::is_directory(dir))
    {
        return register_datasource(dir);
    }
    bool success = false;
    try
    {
        for (std::string const& file_name : mapnik::util::list_directory(dir))
        {
            if (mapnik::util::is_directory(file_name) && recurse)
            {
                if (register_datasources(file_name, true))
                {
                    success = true;
                }
            }
            else
            {
                std::string base_name = mapnik::util::basename(file_name);
                if (!boost::algorithm::starts_with(base_name, ".") && mapnik::util::is_regular_file(file_name) &&
                    is_input_plugin(file_name))
                {
                    if (register_datasource(file_name))
                    {
                        success = true;
                    }
                }
            }
        }
    }
    catch (std::exception const& ex)
    {
        MAPNIK_LOG_ERROR(datasource_cache) << "register_datasources: " << ex.what();
    }
    return success;
}

bool datasource_cache::register_datasource(std::string const& filename)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::recursive_mutex> lock(instance_mutex_);
#endif
    try
    {
        if (!mapnik::util::exists(filename))
        {
            MAPNIK_LOG_ERROR(datasource_cache) << "Cannot load '" << filename << "' (plugin does not exist)";
            return false;
        }
        std::shared_ptr<PluginInfo> plugin = std::make_shared<PluginInfo>(filename, "datasource_name");
        if (plugin->valid())
        {
            if (plugin->name().empty())
            {
                MAPNIK_LOG_ERROR(datasource_cache)
                  << "Problem loading plugin library '" << filename << "' (plugin is lacking compatible interface)";
            }
            else
            {
                if (plugins_.emplace(plugin->name(), plugin).second)
                {
                    MAPNIK_LOG_DEBUG(datasource_cache) << "datasource_cache: Registered=" << plugin->name();
                    return true;
                }
            }
        }
        else
        {
            MAPNIK_LOG_ERROR(datasource_cache)
              << "Problem loading plugin library: " << filename
              << " (dlopen failed - plugin likely has an unsatisfied dependency or incompatible ABI)";
        }
    }
    catch (std::exception const& ex)
    {
        MAPNIK_LOG_ERROR(datasource_cache)
          << "Exception caught while loading plugin library: " << filename << " (" << ex.what() << ")";
    }
    return false;
}

} // namespace mapnik
