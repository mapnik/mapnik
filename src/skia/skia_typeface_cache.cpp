/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#include <mapnik/debug.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
#endif

#include <SkTypeface.h>
//#include <SkFontHost.h>
#include <SkAdvancedTypefaceMetrics.h>

#include <mapnik/skia/skia_typeface_cache.hpp>
#include <mapnik/util/fs.hpp>

namespace mapnik
{

skia_typeface_cache::skia_typeface_cache()
{
}

skia_typeface_cache::~skia_typeface_cache()
{
}


SkTypeface * skia_typeface_cache::create(std::string const& family_name)
{
    cache_type::const_iterator itr = typefaces_.find(family_name);
    if (itr != typefaces_.end())
    {
        return itr->second;
    }
    return 0;
}

bool skia_typeface_cache::register_font(std::string const& file_name)
{
#ifdef MAPNIK_THREADSAFE
    boost::mutex::scoped_lock lock(mutex_);
#endif
    SkTypeface * typeface = SkTypeface::CreateFromFile(file_name.c_str());
    if (typeface)
    {
        SkAdvancedTypefaceMetrics * metrics = typeface->getAdvancedTypefaceMetrics(SkAdvancedTypefaceMetrics::kNo_PerGlyphInfo);
        std::cerr << metrics->fFontName.c_str() << std::endl;
        typefaces_.insert(std::make_pair(std::string(metrics->fFontName.c_str()),typeface));
        return true;
    }
    return false;
}

bool skia_typeface_cache::register_fonts(std::string const& dir, bool recurse)
{
    if (!mapnik::util::exists(dir))
    {
        return false;
    }
    if (!mapnik::util::is_directory(dir))
    {
        return register_font(dir);
    }
    bool success = false;
    try
    {
        boost::filesystem::directory_iterator end_itr;
        for (boost::filesystem::directory_iterator itr(dir); itr != end_itr; ++itr)
        {
#if (BOOST_FILESYSTEM_VERSION == 3)
            std::string file_name = itr->path().string();
#else // v2
            std::string file_name = itr->string();
#endif
            if (boost::filesystem::is_directory(*itr) && recurse)
            {
                if (register_fonts(file_name, true))
                {
                    success = true;
                }
            }
            else
            {
#if (BOOST_FILESYSTEM_VERSION == 3)
                std::string base_name = itr->path().filename().string();
#else // v2
                std::string base_name = itr->filename();
#endif
                if (!boost::algorithm::starts_with(base_name,".") &&
                    boost::filesystem::is_regular_file(file_name))
                {
                    if (register_font(file_name))
                    {
                        success = true;
                    }
                }
            }
        }
    }
    catch (std::exception const& ex)
    {
        MAPNIK_LOG_ERROR(skia_typeface_cache) << "register_fonts: " << ex.what();
    }
    return success;
}

#ifdef MAPNIK_THREADSAFE
boost::mutex skia_typeface_cache::mutex_;
#endif
skia_typeface_cache::cache_type skia_typeface_cache::typefaces_;

}
