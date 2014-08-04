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

#include <mapnik/plugin.hpp>
#include <stdexcept>

#ifdef _WINDOWS
  #define NOMINMAX
  #include <windows.h>
  #define handle HMODULE
  #define dlsym GetProcAddress
  #define dlclose FreeLibrary
  #define dlerror GetLastError
#else
  #include <dlfcn.h>
  #define handle void *
#endif

// TODO - handle/report dlerror

namespace mapnik
{

struct _mapnik_lib_t {
    handle dl;
};

PluginInfo::PluginInfo(std::string const& filename,
                       std::string const& library_name)
    : filename_(filename),
      name_(),
      module_(new mapnik_lib_t)
      {
#ifdef _WINDOWS
          if (module_) module_->dl = LoadLibraryA(filename.c_str());
#else
          if (module_) module_->dl = dlopen(filename.c_str(),RTLD_LAZY);
#endif
          if (module_ && module_->dl)
          {
                name_func name = reinterpret_cast<name_func>(dlsym(module_->dl, library_name.c_str()));
                if (name) name_ = name();
          }
      }

PluginInfo::~PluginInfo()
{
    if (module_)
    {
        if (module_->dl) dlclose(module_->dl),module_->dl=0;
        delete module_;
    }
}


void * PluginInfo::get_symbol(std::string const& sym_name) const
{
    return static_cast<void *>(dlsym(module_->dl, sym_name.c_str()));
}

std::string const& PluginInfo::name() const
{
    return name_;
}

bool PluginInfo::valid() const
{
    if (module_ && module_->dl && !name_.empty()) return true;
    return false;
}

std::string PluginInfo::get_error() const
{
    return std::string("could not open: '") + name_ + "'";
}

void PluginInfo::init()
{
    // do any initialization needed
}

void PluginInfo::exit()
{
    // do any shutdown needed
}


}
