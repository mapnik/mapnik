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

#ifndef MAPNIK_FONT_ENGINE_FREETYPE_HPP
#define MAPNIK_FONT_ENGINE_FREETYPE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/text/font_library.hpp>
#include <mapnik/noncopyable.hpp>

// stl
#include <memory>
#include <map>
#include <utility> // pair
#include <vector>

#ifdef MAPNIK_THREADSAFE
#include <mutex>
#endif

namespace boost { template <class T> class optional; }

namespace mapnik
{

class stroker;
using stroker_ptr = std::shared_ptr<stroker>;
class font_face_set;
using face_set_ptr = std::unique_ptr<font_face_set>;
class font_face;
using face_ptr = std::shared_ptr<font_face>;

class MAPNIK_DECL freetype_engine
{
public:
    using font_file_mapping_type = std::map<std::string,std::pair<int,std::string>>;
    using font_memory_cache_type = std::map<std::string, std::pair<std::unique_ptr<char[]>, std::size_t>>;
    static bool is_font_file(std::string const& file_name);
    /*! \brief register a font file
     *  @param file_name path to a font file.
     *  @return bool - true if at least one face was successfully registered in the file.
     */
    static bool register_font(std::string const& file_name);
    /*! \brief register a font files
     *  @param dir - path to a directory containing fonts or subdirectories.
     *  @param recurse - default false, whether to search for fonts in sub directories.
     *  @return bool - true if at least one face was successfully registered.
     */
    static bool register_fonts(std::string const& dir, bool recurse = false);
    static std::vector<std::string> face_names();
    static font_file_mapping_type const& get_mapping();
    static font_memory_cache_type & get_cache();
    static bool can_open(std::string const& face_name,
                         font_library & library,
                         font_file_mapping_type const& font_file_mapping,
                         font_file_mapping_type const& global_font_file_mapping);
    static face_ptr create_face(std::string const& face_name,
                         font_library & library,
                         font_file_mapping_type const& font_file_mapping,
                         freetype_engine::font_memory_cache_type const& font_cache,
                         font_file_mapping_type const& global_font_file_mapping,
                         freetype_engine::font_memory_cache_type & global_memory_fonts);
    static bool register_font_impl(std::string const& file_name,
                                   font_library & libary,
                                   font_file_mapping_type & font_file_mapping);
    static bool register_fonts_impl(std::string const& dir,
                                    font_library & libary,
                                    font_file_mapping_type & font_file_mapping,
                                    bool recurse = false);
    virtual ~freetype_engine();
    freetype_engine();
private:
    static bool register_font_impl(std::string const& file_name, FT_LibraryRec_ * library);
    static bool register_fonts_impl(std::string const& dir, FT_LibraryRec_ * library, bool recurse = false);
#ifdef MAPNIK_THREADSAFE
    static std::mutex mutex_;
#endif
    static font_file_mapping_type global_font_file_mapping_;
    static font_memory_cache_type global_memory_fonts_;
};

class MAPNIK_DECL face_manager : private mapnik::noncopyable
{
    using face_ptr_cache_type = std::map<std::string, face_ptr>;

public:
    face_manager(font_library & library,
                 freetype_engine::font_file_mapping_type const& font_file_mapping,
                 freetype_engine::font_memory_cache_type const& font_cache);
    face_ptr get_face(std::string const& name);
    face_set_ptr get_face_set(std::string const& name);
    face_set_ptr get_face_set(font_set const& fset);
    face_set_ptr get_face_set(std::string const& name, boost::optional<font_set> fset);
    inline stroker_ptr get_stroker() { return stroker_; }
private:
    face_ptr_cache_type face_ptr_cache_;
    font_library & library_;
    freetype_engine::font_file_mapping_type const& font_file_mapping_;
    freetype_engine::font_memory_cache_type const& font_memory_cache_;
    stroker_ptr stroker_;
};

using face_manager_freetype = face_manager;

}

#endif // MAPNIK_FONT_ENGINE_FREETYPE_HPP
