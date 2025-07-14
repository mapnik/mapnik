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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/face.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/util/file_io.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string/predicate.hpp>

// freetype2
extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_MODULE_H
}
MAPNIK_DISABLE_WARNING_POP

// stl
#include <algorithm>

namespace mapnik {

MAPNIK_DISABLE_WARNING_PUSH
MAPNIK_DISABLE_WARNING_ATTRIBUTES
template class MAPNIK_DECL singleton<freetype_engine, CreateUsingNew>;
MAPNIK_DISABLE_WARNING_POP

bool freetype_engine::is_font_file(std::string const& file_name)
{
    // only accept files that will be matched by freetype2's `figurefiletype()`
    std::string fn = file_name;
    std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
    return boost::algorithm::ends_with(fn, std::string(".ttf")) ||
           boost::algorithm::ends_with(fn, std::string(".otf")) ||
           boost::algorithm::ends_with(fn, std::string(".woff")) ||
           boost::algorithm::ends_with(fn, std::string(".ttc")) ||
           boost::algorithm::ends_with(fn, std::string(".pfa")) ||
           boost::algorithm::ends_with(fn, std::string(".pfb")) ||
           // Plus OSX custom ext
           boost::algorithm::ends_with(fn, std::string(".dfont"));
}

unsigned long ft_read_cb(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count)
{
    if (count <= 0)
        return 0;
    FILE* file = static_cast<FILE*>(stream->descriptor.pointer);
    std::fseek(file, offset, SEEK_SET);
    return std::fread(reinterpret_cast<unsigned char*>(buffer), 1, count, file);
}

bool freetype_engine::register_font(std::string const& file_name)
{
    return instance().register_font_impl(file_name);
}

bool freetype_engine::register_font_impl(std::string const& file_name)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(mutex_);
#endif
    font_library library;
    return register_font_impl(file_name, library, global_font_file_mapping_);
}

bool freetype_engine::register_font_impl(std::string const& file_name,
                                         font_library& library,
                                         freetype_engine::font_file_mapping_type& font_file_mapping)
{
    MAPNIK_LOG_DEBUG(font_engine_freetype) << "registering: " << file_name;
    mapnik::util::file file(file_name);
    if (!file)
        return false;

    FT_Face face = 0;
    FT_Open_Args args;
    FT_StreamRec streamRec;
    memset(&args, 0, sizeof(args));
    memset(&streamRec, 0, sizeof(streamRec));
    streamRec.base = 0;
    streamRec.pos = 0;
    streamRec.size = file.size();
    streamRec.descriptor.pointer = file.get();
    streamRec.read = ft_read_cb;
    streamRec.close = nullptr;
    args.flags = FT_OPEN_STREAM;
    args.stream = &streamRec;
    int num_faces = 0;
    bool success = false;
    // some font files have multiple fonts in a file
    // the count is in the 'root' face library[0]
    // see the FT_FaceRec in freetype.h
    for (int i = 0; face == 0 || i < num_faces; ++i)
    {
        // if face is null then this is the first face
        FT_Error error = FT_Open_Face(library.get(), &args, i, &face);
        if (error)
            break;
        // store num_faces locally, after FT_Done_Face it can not be accessed any more
        if (num_faces == 0)
            num_faces = face->num_faces;
        // some fonts can lack names, skip them
        // http://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html#FT_FaceRec
        if (face->family_name && face->style_name)
        {
            std::string name = std::string(face->family_name) + " " + std::string(face->style_name);
            // skip fonts with leading . in the name
            if (!boost::algorithm::starts_with(name, "."))
            {
                // http://stackoverflow.com/a/24795559/2333354
                auto range = font_file_mapping.equal_range(name);
                if (range.first == range.second) // the key was previously absent; insert a pair
                {
                    font_file_mapping.emplace_hint(range.first, name, std::make_pair(i, file_name));
                }
                else // the key was present, replace the associated value
                {    /* some action with value range.first->second about to be overwritten here */
                    MAPNIK_LOG_WARN(font_engine_freetype) << "registering new " << name << " at '" << file_name << "'";
                    range.first->second = std::make_pair(i, file_name); // replace value
                }
                success = true;
            }
        }
        else
        {
            std::ostringstream s;
            s << "Warning: unable to load font file '" << file_name << "' ";
            if (!face->family_name && !face->style_name)
                s << "which lacks both a family name and style name";
            else if (face->family_name)
                s << "which reports a family name of '" << std::string(face->family_name) << "' and lacks a style name";
            else if (face->style_name)
                s << "which reports a style name of '" << std::string(face->style_name) << "' and lacks a family name";
            MAPNIK_LOG_ERROR(font_engine_freetype) << "register_font: " << s.str();
        }
        if (face)
            FT_Done_Face(face);
    }
    return success;
}

bool freetype_engine::register_fonts(std::string const& dir, bool recurse)
{
    return instance().register_fonts_impl(dir, recurse);
}

bool freetype_engine::register_fonts_impl(std::string const& dir, bool recurse)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(mutex_);
#endif
    font_library library;
    return register_fonts_impl(dir, library, global_font_file_mapping_, recurse);
}

bool freetype_engine::register_fonts_impl(std::string const& dir,
                                          font_library& library,
                                          freetype_engine::font_file_mapping_type& font_file_mapping,
                                          bool recurse)
{
    if (!mapnik::util::exists(dir))
    {
        return false;
    }
    if (!mapnik::util::is_directory(dir))
    {
        return register_font_impl(dir, library, font_file_mapping);
    }
    bool success = false;
    try
    {
        for (std::string const& file_name : mapnik::util::list_directory(dir))
        {
            if (mapnik::util::is_directory(file_name) && recurse)
            {
                if (register_fonts_impl(file_name, library, font_file_mapping, true))
                {
                    success = true;
                }
            }
            else
            {
                std::string base_name = mapnik::util::basename(file_name);
                if (!boost::algorithm::starts_with(base_name, ".") && mapnik::util::is_regular_file(file_name) &&
                    is_font_file(file_name))
                {
                    if (register_font_impl(file_name, library, font_file_mapping))
                    {
                        success = true;
                    }
                }
            }
        }
    }
    catch (std::exception const& ex)
    {
        MAPNIK_LOG_ERROR(font_engine_freetype) << "register_fonts: " << ex.what();
    }
    return success;
}

std::vector<std::string> freetype_engine::face_names()
{
    return instance().face_names_impl();
}

std::vector<std::string> freetype_engine::face_names_impl()
{
    std::vector<std::string> names;
    for (auto const& kv : global_font_file_mapping_)
    {
        names.push_back(kv.first);
    }
    return names;
}

freetype_engine::font_file_mapping_type const& freetype_engine::get_mapping()
{
    return instance().get_mapping_impl();
}

freetype_engine::font_file_mapping_type const& freetype_engine::get_mapping_impl()
{
    return global_font_file_mapping_;
}

freetype_engine::font_memory_cache_type& freetype_engine::get_cache()
{
    return instance().get_cache_impl();
}

freetype_engine::font_memory_cache_type& freetype_engine::get_cache_impl()
{
    return global_memory_fonts_;
}

bool freetype_engine::can_open(std::string const& face_name,
                               font_library& library,
                               font_file_mapping_type const& font_file_mapping,
                               font_file_mapping_type const& global_font_file_mapping)
{
    return instance().can_open_impl(face_name, library, font_file_mapping, global_font_file_mapping);
}

bool freetype_engine::can_open_impl(std::string const& face_name,
                                    font_library& library,
                                    font_file_mapping_type const& font_file_mapping,
                                    font_file_mapping_type const& global_font_file_mapping)
{
    bool found_font_file = false;
    font_file_mapping_type::const_iterator itr = font_file_mapping.find(face_name);
    if (itr != font_file_mapping.end())
    {
        found_font_file = true;
    }
    else
    {
        itr = global_font_file_mapping.find(face_name);
        if (itr != global_font_file_mapping.end())
        {
            found_font_file = true;
        }
    }
    if (!found_font_file)
        return false;
    mapnik::util::file file(itr->second.second);
    if (!file)
        return false;
    FT_Face face = 0;
    FT_Open_Args args;
    FT_StreamRec streamRec;
    memset(&args, 0, sizeof(args));
    memset(&streamRec, 0, sizeof(streamRec));
    streamRec.base = 0;
    streamRec.pos = 0;
    streamRec.size = file.size();
    streamRec.descriptor.pointer = file.get();
    streamRec.read = ft_read_cb;
    streamRec.close = nullptr;
    args.flags = FT_OPEN_STREAM;
    args.stream = &streamRec;
    // -1 is used to quickly check if the font file appears valid without iterating each face
    FT_Error error = FT_Open_Face(library.get(), &args, -1, &face);
    if (face)
        FT_Done_Face(face);
    if (error)
        return false;
    return true;
}

face_ptr freetype_engine::create_face_impl(std::string const& family_name,
                                           font_library& library,
                                           freetype_engine::font_file_mapping_type const& font_file_mapping,
                                           freetype_engine::font_memory_cache_type const& font_cache,
                                           freetype_engine::font_file_mapping_type const& global_font_file_mapping,
                                           freetype_engine::font_memory_cache_type& global_memory_fonts)
{
    bool found_font_file = false;
    font_file_mapping_type::const_iterator itr = font_file_mapping.find(family_name);
    // look for font registered on specific map
    if (itr != font_file_mapping.end())
    {
        auto mem_font_itr = font_cache.find(itr->second.second);
        // if map has font already in memory, use it
        if (mem_font_itr != font_cache.end())
        {
            FT_Face face;
            FT_Error error =
              FT_New_Memory_Face(library.get(),
                                 reinterpret_cast<FT_Byte const*>(mem_font_itr->second.first.get()), // data
                                 static_cast<FT_Long>(mem_font_itr->second.second),                  // size
                                 itr->second.first,                                                  // face index
                                 &face);
            if (!error)
                return std::make_shared<font_face>(face);
        }
        // we don't add to cache here because the map and its font_cache
        // must be immutable during rendering for predictable thread safety
        found_font_file = true;
    }
    else
    {
        // otherwise search global registry
        itr = global_font_file_mapping.find(family_name);
        if (itr != global_font_file_mapping.end())
        {
            auto mem_font_itr = global_memory_fonts.find(itr->second.second);
            // if font already in memory, use it
            if (mem_font_itr != global_memory_fonts.end())
            {
                FT_Face face;
                FT_Error error =
                  FT_New_Memory_Face(library.get(),
                                     reinterpret_cast<FT_Byte const*>(mem_font_itr->second.first.get()), // data
                                     static_cast<FT_Long>(mem_font_itr->second.second),                  // size
                                     itr->second.first,                                                  // face index
                                     &face);
                if (!error)
                    return std::make_shared<font_face>(face);
            }
            found_font_file = true;
        }
    }
    // if we found file file but it is not yet in memory
    if (found_font_file)
    {
        mapnik::util::file file(itr->second.second);
        if (file)
        {
#ifdef MAPNIK_THREADSAFE
            std::lock_guard<std::mutex> lock(mutex_);
#endif
            auto result = global_memory_fonts.emplace(itr->second.second, std::make_pair(file.data(), file.size()));
            FT_Face face;
            FT_Error error =
              FT_New_Memory_Face(library.get(),
                                 reinterpret_cast<FT_Byte const*>(result.first->second.first.get()), // data
                                 static_cast<FT_Long>(result.first->second.second),                  // size
                                 itr->second.first,                                                  // face index
                                 &face);
            if (error)
            {
                // we can't load font, erase it.
                global_memory_fonts.erase(result.first);
                return face_ptr();
            }
            return std::make_shared<font_face>(face);
        }
    }
    return face_ptr();
}

face_ptr freetype_engine::create_face(std::string const& family_name,
                                      font_library& library,
                                      freetype_engine::font_file_mapping_type const& font_file_mapping,
                                      freetype_engine::font_memory_cache_type const& font_cache,
                                      freetype_engine::font_file_mapping_type const& global_font_file_mapping,
                                      freetype_engine::font_memory_cache_type& global_memory_fonts)
{
    return instance().create_face_impl(family_name,
                                       library,
                                       font_file_mapping,
                                       font_cache,
                                       global_font_file_mapping,
                                       global_memory_fonts);
}

face_manager::face_manager(font_library& library,
                           freetype_engine::font_file_mapping_type const& font_file_mapping,
                           freetype_engine::font_memory_cache_type const& font_cache)
    : face_cache_(new face_cache()),
      library_(library),
      font_file_mapping_(font_file_mapping),
      font_memory_cache_(font_cache)
{
    FT_Stroker s;
    FT_Error error = FT_Stroker_New(library_.get(), &s);
    if (!error)
    {
        stroker_ = std::make_shared<stroker>(s);
    }
}

face_ptr face_manager::get_face(std::string const& name)
{
    auto itr = face_cache_->find(name);
    if (itr != face_cache_->end())
    {
        return itr->second;
    }
    else
    {
        face_ptr face = freetype_engine::create_face(name,
                                                     library_,
                                                     font_file_mapping_,
                                                     font_memory_cache_,
                                                     freetype_engine::instance().get_mapping(),
                                                     freetype_engine::instance().get_cache());
        if (face)
        {
            face_cache_->emplace(name, face);
        }
        return face;
    }
}

face_set_ptr face_manager::get_face_set(std::string const& name)
{
    face_set_ptr face_set = std::make_unique<font_face_set>();
    if (face_ptr face = get_face(name))
    {
        face_set->add(face);
    }
    return face_set;
}

face_set_ptr face_manager::get_face_set(font_set const& fset)
{
    std::vector<std::string> const& names = fset.get_face_names();
    face_set_ptr face_set = std::make_unique<font_face_set>();
    for (auto const& name : names)
    {
        face_ptr face = get_face(name);
        if (face)
        {
            face_set->add(face);
        }
#ifdef MAPNIK_LOG
        else
        {
            MAPNIK_LOG_DEBUG(font_engine_freetype)
              << "Failed to find face '" << name << "' in font set '" << fset.get_name() << "'\n";
        }
#endif
    }
    return face_set;
}

face_set_ptr face_manager::get_face_set(std::string const& name, std::optional<font_set> fset)
{
    if (fset && fset->size() > 0)
    {
        return get_face_set(*fset);
    }
    else
    {
        return get_face_set(name);
    }
}

} // namespace mapnik
