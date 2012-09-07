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

#ifndef MAPNIK_PLUGIN_HPP
#define MAPNIK_PLUGIN_HPP

// boost
#include <boost/utility.hpp>

// stl
#include <string>

// ltdl
#include <ltdl.h>

namespace mapnik
{
class PluginInfo : boost::noncopyable
{
private:
    std::string name_;
    lt_dlhandle module_;
public:
    PluginInfo (std::string const& name,const lt_dlhandle module);
    ~PluginInfo();
    std::string const& name() const;
    lt_dlhandle handle() const;
};
}

#endif // MAPNIK_PLUGIN_HPP
