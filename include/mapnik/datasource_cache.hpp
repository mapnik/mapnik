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

#ifndef MAPNIK_DATASOURCE_CACHE_HPP
#define MAPNIK_DATASOURCE_CACHE_HPP

// mapnik
#include <mapnik/utils.hpp>
#include <mapnik/params.hpp>
#include <mapnik/plugin.hpp>
#include <mapnik/datasource.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

// stl
#include <map>

namespace mapnik {
class MAPNIK_DECL datasource_cache :
        public singleton <datasource_cache,CreateStatic>,
        private boost::noncopyable
{
    friend class CreateStatic<datasource_cache>;
private:
    datasource_cache();
    ~datasource_cache();
    datasource_cache(const datasource_cache&);
    datasource_cache& operator=(const datasource_cache&);
    static std::map<std::string,boost::shared_ptr<PluginInfo> > plugins_;
    static bool registered_;
    static bool insert(const std::string&  name,const lt_dlhandle module);
    static std::vector<std::string> plugin_directories_;
public:
    static std::vector<std::string> plugin_names();
    static std::string plugin_directories();
    static void register_datasources(std::string const& path);
    static bool register_datasource(std::string const& path);
    static boost::shared_ptr<datasource> create(parameters const& params, bool bind=true);
};
}

#endif // MAPNIK_DATASOURCE_CACHE_HPP
