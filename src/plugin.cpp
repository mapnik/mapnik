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

//$Id: plugin.cpp 17 2005-03-08 23:58:43Z pavlenko $

#include "plugin.hpp"

namespace mapnik
{

    PluginInfo::PluginInfo (const std::string& name,const lt_dlhandle module)
        :name_(name),module_(module) {}

    PluginInfo::~PluginInfo()
    {
        if (module_)
        {
            lt_dlclose(module_),module_=0;
        }
    }

    const std::string& PluginInfo::name() const
    {
        return name_;
    }

    lt_dlhandle PluginInfo::handle() const
    {
        return module_;
    }

}
