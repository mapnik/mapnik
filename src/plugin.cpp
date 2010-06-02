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

//$Id: plugin.cpp 17 2005-03-08 23:58:43Z pavlenko $

#include <mapnik/plugin.hpp>
#include <ltdl.h>

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
