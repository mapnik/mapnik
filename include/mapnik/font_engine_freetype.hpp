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

struct FT_LibraryRec_;
struct FT_MemoryRec_;
namespace boost { template <class T> class optional; }

namespace mapnik
{

class stroker;
using stroker_ptr = std::shared_ptr<stroker>;
class font_face_set;
using face_set_ptr = std::shared_ptr<font_face_set>;
class font_face;
using face_ptr = std::shared_ptr<font_face>;


class MAPNIK_DECL freetype_engine
{
public:
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
    static std::map<std::string, std::pair<int,std::string> > const& get_mapping();
    face_ptr create_face(std::string const& family_name);
    stroker_ptr create_stroker();
    virtual ~freetype_engine();
    freetype_engine();
private:
    static bool register_font_impl(std::string const& file_name, FT_LibraryRec_ * library);
    static bool register_fonts_impl(std::string const& dir, FT_LibraryRec_ * library, bool recurse = false);
    font_library library_;
#ifdef MAPNIK_THREADSAFE
    static std::mutex mutex_;
#endif
    static std::map<std::string, std::pair<int,std::string> > name2file_;
    static std::map<std::string, std::pair<std::unique_ptr<char[]>, std::size_t> > memory_fonts_;
};

template <typename T>
class MAPNIK_DECL face_manager : private mapnik::noncopyable
{
    using font_engine_type = T;
    using face_ptr_cache_type = std::map<std::string, face_ptr>;

public:
    face_manager(T & engine)
        : engine_(engine),
        stroker_(engine_.create_stroker()),
        face_ptr_cache_()  {}

    face_ptr get_face(std::string const& name);
    face_set_ptr get_face_set(std::string const& name);
    face_set_ptr get_face_set(font_set const& fset);
    face_set_ptr get_face_set(std::string const& name, boost::optional<font_set> fset);


    inline stroker_ptr get_stroker() { return stroker_; }

private:
    font_engine_type & engine_;
    stroker_ptr stroker_;
    face_ptr_cache_type face_ptr_cache_;
};

using face_manager_freetype = face_manager<freetype_engine>;

}

#endif // MAPNIK_FONT_ENGINE_FREETYPE_HPP
