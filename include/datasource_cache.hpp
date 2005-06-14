/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: datasource_cache.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef DSFACTORY_HPP
#define DSFACTORY_HPP

#include "utils.hpp"
#include "ptr.hpp"
#include "params.hpp"
#include "plugin.hpp"
#include "datasource.hpp"
#include <map>

namespace mapnik
{
    class datasource_cache : public singleton <datasource_cache,CreateStatic>
    {
        friend class CreateStatic<datasource_cache>;
    private:
	datasource_cache();
	~datasource_cache();
	datasource_cache(const datasource_cache&);
	datasource_cache& operator=(const datasource_cache&);
	static std::map<std::string,ref_ptr<PluginInfo> > plugins_;
	static bool registered_;
	static bool insert(const std::string&  name,const lt_dlhandle module);
    public:
	static void register_datasources(const std::string& path);
	static ref_ptr<datasource,datasource_delete> create(const Parameters& params);
    };
}
#endif                                            //DSFACTORY_HPP
