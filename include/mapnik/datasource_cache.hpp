/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_DATASOURCE_CACHE_HPP
#define MAPNIK_DATASOURCE_CACHE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/util/singleton.hpp>
#include <mapnik/util/noncopyable.hpp>

// stl
#include <map>
#include <set>
#include <vector>
#include <memory>
#include <mutex>

namespace mapnik {

class datasource;
class parameters;
class PluginInfo;

class MAPNIK_DECL datasource_cache
    : public singleton<datasource_cache, CreateStatic>,
      private util::noncopyable
{
    friend class CreateStatic<datasource_cache>;
public:
    std::vector<std::string> plugin_names();
    std::string plugin_directories();
    bool register_datasources(std::string const& path, bool recurse = false);
    bool register_datasource(std::string const& path);
    std::shared_ptr<datasource> create(parameters const& params);
private:
    datasource_cache();
    ~datasource_cache();
    std::map<std::string,std::shared_ptr<PluginInfo> > plugins_;
    std::set<std::string> plugin_directories_;
    // the singleton has a mutex protecting the instance pointer,
    // but the instance also needs its own mutex to protect the
    // plugins_ and plugin_directories_ members which are potentially
    // modified recusrively by register_datasources(path, true);
    std::recursive_mutex instance_mutex_;
};

extern template class MAPNIK_DECL singleton<datasource_cache, CreateStatic>;

}

#endif // MAPNIK_DATASOURCE_CACHE_HPP
