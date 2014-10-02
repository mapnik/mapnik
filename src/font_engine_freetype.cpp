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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/face.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/make_unique.hpp>

// boost
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

// stl
#include <algorithm>
#include <stdexcept>
#include <cstdlib>

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_MODULE_H
}

void* _Alloc_Func(FT_Memory memory, long size)
{
    return std::malloc(size);
}

void _Free_Func(FT_Memory memory, void *block)
{
    std::free(block);
}

void* _Realloc_Func(FT_Memory memory, long cur_size, long new_size, void* block)
{
    return std::realloc(block, new_size);
}

namespace mapnik
{

void init_freetype(FT_Memory memory, FT_Library & library)
{
    memory->alloc = _Alloc_Func;
    memory->free = _Free_Func;
    memory->realloc = _Realloc_Func;
    FT_Error error = FT_New_Library(memory, &library );
    if (error) throw std::runtime_error("can not initalise FreeType2 library");
    FT_Add_Default_Modules(library);
}

freetype_engine::freetype_engine()
    : library_() {}

freetype_engine::~freetype_engine() {}

bool freetype_engine::is_font_file(std::string const& file_name)
{
    // only accept files that will be matched by freetype2's `figurefiletype()`
    std::string fn = file_name;
    std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
    return boost::algorithm::ends_with(fn,std::string(".ttf")) ||
        boost::algorithm::ends_with(fn,std::string(".otf")) ||
        boost::algorithm::ends_with(fn,std::string(".ttc")) ||
        boost::algorithm::ends_with(fn,std::string(".pfa")) ||
        boost::algorithm::ends_with(fn,std::string(".pfb")) ||
        boost::algorithm::ends_with(fn,std::string(".ttc")) ||
        boost::algorithm::ends_with(fn,std::string(".woff"))||
        // Plus OSX custom ext
        boost::algorithm::ends_with(fn,std::string(".dfont"));
}

unsigned long ft_read_cb(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
    if (count <= 0) return 0;
    FILE * file = static_cast<FILE *>(stream->descriptor.pointer);
    std::fseek (file , offset , SEEK_SET);
    return std::fread ((char*)buffer, 1, count, file);
}

bool freetype_engine::register_font(std::string const& file_name)
{
#ifdef MAPNIK_THREADSAFE
    mapnik::scoped_lock lock(mutex_);
#endif
    font_library library;
    bool result = register_font_impl(file_name, library.get());
    return result;
}

bool freetype_engine::register_font_impl(std::string const& file_name, FT_LibraryRec_ * library)
{
    MAPNIK_LOG_DEBUG(font_engine_freetype) << "registering: " << file_name;
    mapnik::util::file file(file_name);
    if (!file.open()) return false;

    FT_Face face = 0;
    FT_Open_Args args;
    FT_StreamRec streamRec;
    memset(&args, 0, sizeof(args));
    memset(&streamRec, 0, sizeof(streamRec));
    streamRec.base = 0;
    streamRec.pos = 0;
    streamRec.size = file.size();
    streamRec.descriptor.pointer = file.get();
    streamRec.read  = ft_read_cb;
    streamRec.close = NULL;
    args.flags = FT_OPEN_STREAM;
    args.stream = &streamRec;
    int num_faces = 0;
    bool success = false;
    // some font files have multiple fonts in a file
    // the count is in the 'root' face library[0]
    // see the FT_FaceRec in freetype.h
    for ( int i = 0; face == 0 || i < num_faces; ++i )
    {
        // if face is null then this is the first face
        FT_Error error = FT_Open_Face(library, &args, i, &face);
        if (error) break;
        // store num_faces locally, after FT_Done_Face it can not be accessed any more
        if (num_faces == 0)
            num_faces = face->num_faces;
        // some fonts can lack names, skip them
        // http://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html#FT_FaceRec
        if (face->family_name && face->style_name)
        {
            std::string name = std::string(face->family_name) + " " + std::string(face->style_name);
            // skip fonts with leading . in the name
            if (!boost::algorithm::starts_with(name,"."))
            {
                // http://stackoverflow.com/a/24795559/2333354
                auto range = name2file_.equal_range(name);
                if (range.first == range.second) // the key was previously absent; insert a pair
                {
                    name2file_.emplace_hint(range.first,name,std::move(std::make_pair(i,file_name)));
                }
                else // the key was present, replace the associated value
                { /* some action with value range.first->second about to be overwritten here */
                    MAPNIK_LOG_WARN(font_engine_freetype) << "registering new " << name << " at '" << file_name << "'";
                    range.first->second = std::move(std::make_pair(i,file_name)); // replace value
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
        if (face) FT_Done_Face(face);
    }
    return success;
}

bool freetype_engine::register_fonts(std::string const& dir, bool recurse)
{
#ifdef MAPNIK_THREADSAFE
    mapnik::scoped_lock lock(mutex_);
#endif
    font_library library;
    bool result = register_fonts_impl(dir, library.get(), recurse);
    return result;
}

bool freetype_engine::register_fonts_impl(std::string const& dir, FT_LibraryRec_ * library, bool recurse)
{
    if (!mapnik::util::exists(dir))
    {
        return false;
    }
    if (!mapnik::util::is_directory(dir))
    {
        return mapnik::freetype_engine::register_font_impl(dir, library);
    }
    bool success = false;
    try
    {
        boost::filesystem::directory_iterator end_itr;
#ifdef _WINDOWS
        std::wstring wide_dir(mapnik::utf8_to_utf16(dir));
        for (boost::filesystem::directory_iterator itr(wide_dir); itr != end_itr; ++itr)
        {
    #if (BOOST_FILESYSTEM_VERSION == 3)
            std::string file_name = mapnik::utf16_to_utf8(itr->path().wstring());
    #else // v2
            std::string file_name = mapnik::utf16_to_utf8(itr->wstring());
    #endif
#else
        for (boost::filesystem::directory_iterator itr(dir); itr != end_itr; ++itr)
        {
#if (BOOST_FILESYSTEM_VERSION == 3)
            std::string file_name = itr->path().string();
#else // v2
            std::string file_name = itr->string();
#endif
#endif
            if (boost::filesystem::is_directory(*itr) && recurse)
            {
                if (register_fonts_impl(file_name, library, true))
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
                    mapnik::util::is_regular_file(file_name) &&
                    is_font_file(file_name))
                {
                    if (mapnik::freetype_engine::register_font_impl(file_name, library))
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


std::vector<std::string> freetype_engine::face_names ()
{
    std::vector<std::string> names;
    for (auto const& kv : name2file_)
    {
        names.push_back(kv.first);
    }
    return names;
}

std::map<std::string,std::pair<int,std::string> > const& freetype_engine::get_mapping()
{
    return name2file_;
}


face_ptr freetype_engine::create_face(std::string const& family_name)
{
    auto itr = name2file_.find(family_name);
    if (itr != name2file_.end())
    {
        FT_Face face;

        auto mem_font_itr = memory_fonts_.find(itr->second.second);

        if (mem_font_itr != memory_fonts_.end()) // memory font
        {
            FT_Error error = FT_New_Memory_Face(library_.get(),
                                                reinterpret_cast<FT_Byte const*>(mem_font_itr->second.first.get()), // data
                                                static_cast<FT_Long>(mem_font_itr->second.second), // size
                                                itr->second.first, // face index
                                                &face);

            if (!error) return std::make_shared<font_face>(face);
        }
        else
        {
            // load font into memory
#ifdef MAPNIK_THREADSAFE
            mapnik::scoped_lock lock(mutex_);
#endif

            mapnik::util::file file(itr->second.second);
            if (file.open())
            {
                auto result = memory_fonts_.emplace(itr->second.second, std::make_pair(std::move(file.data()),file.size()));
                FT_Error error = FT_New_Memory_Face (library_.get(),
                                                     reinterpret_cast<FT_Byte const*>(result.first->second.first.get()),
                                                     static_cast<FT_Long>(result.first->second.second),
                                                     itr->second.first,
                                                     &face);
                if (!error) return std::make_shared<font_face>(face);
                else
                {
                    // we can't load font, erase it.
                    memory_fonts_.erase(result.first);
                }
            }
        }
    }
    return face_ptr();
}


stroker_ptr freetype_engine::create_stroker()
{
    FT_Stroker s;
    FT_Error error = FT_Stroker_New(library_.get(), &s);
    if (!error)
    {
        return std::make_shared<stroker>(s);
    }
    return stroker_ptr();
}



template <typename T>
face_ptr face_manager<T>::get_face(std::string const& name)
{
    auto itr = face_ptr_cache_.find(name);
    if (itr != face_ptr_cache_.end())
    {
        return itr->second;
    }
    else
    {
        face_ptr face = engine_.create_face(name);
        if (face)
        {
            face_ptr_cache_.emplace(name,face);
        }
        return face;
    }
}

template <typename T>
face_set_ptr face_manager<T>::get_face_set(std::string const& name)
{
    face_set_ptr face_set = std::make_unique<font_face_set>();
    if (face_ptr face = get_face(name))
    {
        face_set->add(face);
    }
    return face_set;
}

template <typename T>
face_set_ptr face_manager<T>::get_face_set(font_set const& fset)
{
    std::vector<std::string> const& names = fset.get_face_names();
    face_set_ptr face_set = std::make_unique<font_face_set>();
    for (auto const& name  : names)
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
                << "Failed to find face '" << name
                << "' in font set '" << fset.get_name() << "'\n";
        }
#endif
    }
    return face_set;
}

template <typename T>
face_set_ptr face_manager<T>::get_face_set(const std::string &name, boost::optional<font_set> fset)
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

#ifdef MAPNIK_THREADSAFE
std::mutex freetype_engine::mutex_;
#endif
std::map<std::string,std::pair<int,std::string> > freetype_engine::name2file_;
std::map<std::string,std::pair<std::unique_ptr<char[]>,std::size_t> > freetype_engine::memory_fonts_;
template class face_manager<freetype_engine>;

}
