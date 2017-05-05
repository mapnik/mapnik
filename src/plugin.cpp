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

#include <mapnik/plugin.hpp>
#include <stdexcept>

#ifdef _WINDOWS
  #define NOMINMAX
  #include <windows.h>
  #define handle HMODULE
  #define dlsym GetProcAddress
  #define dlclose FreeLibrary
  #define dlerror GetLastError
  #define MAPNIK_SUPPORTS_DLOPEN
#else
  #ifdef MAPNIK_HAS_DLCFN
    #include <dlfcn.h>
    #define MAPNIK_SUPPORTS_DLOPEN
  #endif
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
          if (module_ && module_->dl)
          {
              callable_returning_string name_call = reinterpret_cast<callable_returning_string>(dlsym(module_->dl, library_name.c_str()));
                if (name_call) name_ = name_call();
                callable_returning_void init_once = reinterpret_cast<callable_returning_void>(dlsym(module_->dl, "on_plugin_load"));
                if (init_once) {
                    init_once();
                }
          }
#else
  #ifdef MAPNIK_HAS_DLCFN
          if (module_) module_->dl = dlopen(filename.c_str(),RTLD_LAZY);
          if (module_ && module_->dl)
          {
                callable_returning_string name_call = reinterpret_cast<callable_returning_string>(dlsym(module_->dl, library_name.c_str()));
                if (name_call) name_ = name_call();
                callable_returning_void init_once = reinterpret_cast<callable_returning_void>(dlsym(module_->dl, "on_plugin_load"));
                if (init_once) {
                    init_once();
                }
          }
  #else
          throw std::runtime_error("no support for loading dynamic objects (Mapnik not compiled with -DMAPNIK_HAS_DLCFN)");
  #endif
#endif
      }

PluginInfo::~PluginInfo()
{
    if (module_)
    {
#ifdef MAPNIK_SUPPORTS_DLOPEN
        /*
          We do not call dlclose for plugins that link libgdal.
          This is a terrible hack, but necessary to prevent crashes
          at exit when gdal attempts to shutdown. The problem arises
          when Mapnik is used with another library that uses thread-local
          storage (like libuv). In this case GDAL also tries to cleanup thread
          local storage and leaves things in a state that causes libuv to crash.
          This is partially fixed by http://trac.osgeo.org/gdal/ticket/5509 but only
          in the case that gdal is linked as a shared library. This workaround therefore
          prevents crashes with gdal 1.11.x and gdal 2.x when using a static libgdal.
        */
        if (module_->dl && name_ != "gdal" && name_ != "ogr")
        {
#ifndef MAPNIK_NO_DLCLOSE
            dlclose(module_->dl),module_->dl=0;
#endif
        }
#endif
        delete module_;
    }
}


void * PluginInfo::get_symbol(std::string const& sym_name) const
{
#ifdef MAPNIK_SUPPORTS_DLOPEN
    return static_cast<void *>(dlsym(module_->dl, sym_name.c_str()));
#else
    return nullptr;
#endif
}

std::string const& PluginInfo::name() const
{
    return name_;
}

bool PluginInfo::valid() const
{
#ifdef MAPNIK_SUPPORTS_DLOPEN
    if (module_ && module_->dl && !name_.empty()) return true;
#endif
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
