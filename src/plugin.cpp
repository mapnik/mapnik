/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#include <mapnik/datasource_plugin.hpp>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#define handle  HMODULE
#define dlsym   GetProcAddress
#define dlclose FreeLibrary
#define dlerror GetLastError
#define MAPNIK_SUPPORTS_DLOPEN
#else
#ifdef MAPNIK_HAS_DLCFN
#include <dlfcn.h>
#define MAPNIK_SUPPORTS_DLOPEN
#endif
#define handle void*
#endif

// TODO - handle/report dlerror

namespace mapnik {

struct _mapnik_lib_t
{
    std::string name;
    std::string error_str;
    handle dl;
    _mapnik_lib_t()
        : name{"unknown"}
        , error_str{""}
        , dl{nullptr}
    {}
    ~_mapnik_lib_t()
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
        if (dl /*&& name_ != "gdal" && name_ != "ogr"*/) // is the gdal issue sill present? We are now
                                                         // unregister all drivers for ogal and gdal (todo:
                                                         // before_unload call)
        {
#ifndef MAPNIK_NO_DLCLOSE
            dlclose(dl);
            dl = nullptr;
#endif
        }
#endif
    }
};

PluginInfo::PluginInfo(std::string const& filename, std::string const& library_name)
    : filename_(filename)
    , module_{std::make_unique<mapnik_lib_t>()}
{
    assert(module_ != nullptr);
#if defined(_WIN32)
    module_->dl = LoadLibraryA(filename.c_str());
#elif defined(MAPNIK_HAS_DLCFN)
    module_->dl = dlopen(filename.c_str(), RTLD_LAZY);
#else
    throw std::runtime_error("no support for loading dynamic objects (Mapnik not compiled with -DMAPNIK_HAS_DLCFN)");
#endif
#if defined(MAPNIK_HAS_DLCFN) || defined(_WIN32)
    if (module_->dl)
    {
        datasource_plugin* plugin{reinterpret_cast<datasource_plugin*>(get_symbol("plugin"))};
        if (!plugin)
            throw std::runtime_error("plugin has a false interface"); //! todo: better error text
        module_->name = plugin->name();
        module_->error_str = "";
        plugin->after_load();
    }
    else
    {
        const auto errcode = dlerror();
#ifdef _WIN32
        module_->error_str = std::system_category().message(errcode);
#else
        module_->error_str = errcode ? errcode : "";
#endif
    }
#endif // defined(MAPNIK_HAS_DLCFN) || defined(_WIN32)
}

PluginInfo::~PluginInfo() {}

void* PluginInfo::get_symbol(std::string const& sym_name) const
{
#ifdef MAPNIK_SUPPORTS_DLOPEN
    return static_cast<void*>(dlsym(module_->dl, sym_name.c_str()));
#else
    return nullptr;
#endif
}

std::string const& PluginInfo::name() const
{
    return module_->name;
}

bool PluginInfo::valid() const
{
#ifdef MAPNIK_SUPPORTS_DLOPEN
    if (module_->dl && !module_->name.empty())
        return true;
#endif
    return false;
}

std::string PluginInfo::get_error() const
{
    return std::string{"could not open: '"} + module_->name + "'. Error: " + module_->error_str;
}

} // namespace mapnik
