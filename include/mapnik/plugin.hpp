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

#ifndef MAPNIK_PLUGIN_HPP
#define MAPNIK_PLUGIN_HPP

// mapnik
#include <mapnik/util/noncopyable.hpp>

// stl
#include <string>

namespace mapnik {

//  Opaque structure for handle
using mapnik_lib_t = struct _mapnik_lib_t;

class PluginInfo : util::noncopyable
{
  public:
    using callable_returning_string = const char* (*)();
    using callable_returning_void = void (*)();
    PluginInfo(std::string const& filename, std::string const& library_name);
    ~PluginInfo();
    std::string const& name() const;
    bool valid() const;
    std::string get_error() const;
    void* get_symbol(std::string const& sym_name) const;
    static void init();
    static void exit();

  private:
    std::string filename_;
    std::string name_;
    mapnik_lib_t* module_;
};
} // namespace mapnik

#endif // MAPNIK_PLUGIN_HPP
