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
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/debug.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>

namespace mapnik
{
freetype_engine::freetype_engine()
{
    FT_Error error = FT_Init_FreeType( &library_ );
    if (error)
    {
        throw std::runtime_error("can not load FreeType2 library");
    }
}

freetype_engine::~freetype_engine()
{
    FT_Done_FreeType(library_);
}

bool freetype_engine::is_font_file(std::string const& file_name)
{
    /** only accept files that will be matched by freetype2's `figurefiletype()` */
    std::string const& fn = boost::algorithm::to_lower_copy(file_name);
    return boost::algorithm::ends_with(fn,std::string(".ttf")) ||
        boost::algorithm::ends_with(fn,std::string(".otf")) ||
        boost::algorithm::ends_with(fn,std::string(".ttc")) ||
        boost::algorithm::ends_with(fn,std::string(".pfa")) ||
        boost::algorithm::ends_with(fn,std::string(".pfb")) ||
        boost::algorithm::ends_with(fn,std::string(".ttc")) ||
        /** Plus OSX custom ext */
        boost::algorithm::ends_with(fn,std::string(".dfont"));
}

bool freetype_engine::register_font(std::string const& file_name)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    FT_Library library = 0;
    FT_Error error = FT_Init_FreeType(&library);
    if (error)
    {
        throw std::runtime_error("Failed to initialize FreeType2 library");
    }

    FT_Face face = 0;
    int num_faces = 0;
    bool success = false;
    // some font files have multiple fonts in a file
    // the count is in the 'root' face library[0]
    // see the FT_FaceRec in freetype.h
    for ( int i = 0; face == 0 || i < num_faces; i++ ) {
        // if face is null then this is the first face
        error = FT_New_Face (library,file_name.c_str(),i,&face);
        if (error)
        {
            break;
        }
        // store num_faces locally, after FT_Done_Face it can not be accessed any more
        if (!num_faces)
            num_faces = face->num_faces;
        // some fonts can lack names, skip them
        // http://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html#FT_FaceRec
        if (face->family_name && face->style_name)
        {
            std::string name = std::string(face->family_name) + " " + std::string(face->style_name);
            // skip fonts with leading . in name
            if (!boost::algorithm::starts_with(name,"."))
            {
                success = true;
                name2file_.insert(std::make_pair(name, std::make_pair(i,file_name)));
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

            MAPNIK_LOG_DEBUG(font_engine_freetype) << "freetype_engine: " << s.str();
        }
    }
    if (face)
        FT_Done_Face(face);
    if (library)
        FT_Done_FreeType(library);
    return success;
}

bool freetype_engine::register_fonts(std::string const& dir, bool recurse)
{
    boost::filesystem::path path(dir);

    if (!boost::filesystem::exists(path))
        return false;

    if (!boost::filesystem::is_directory(path))
        return mapnik::freetype_engine::register_font(dir);

    boost::filesystem::directory_iterator end_itr;
    bool success = false;
    for (boost::filesystem::directory_iterator itr(dir); itr != end_itr; ++itr)
    {
#if (BOOST_FILESYSTEM_VERSION == 3)
        std::string file_name = itr->path().string();
#else // v2
        std::string file_name = itr->string();
#endif
        if (boost::filesystem::is_directory(*itr) && recurse)
        {
            success = register_fonts(file_name, true);
        }
        else
        {
#if (BOOST_FILESYSTEM_VERSION == 3)
            std::string base_name = itr->path().filename().string();
#else // v2
            std::string base_name = itr->filename();
#endif
            if (!boost::algorithm::starts_with(base_name,".") &&
                     boost::filesystem::is_regular_file(file_name) &&
                     is_font_file(file_name))
            {
                success = mapnik::freetype_engine::register_font(file_name);
            }
        }
    }
    return success;
}


std::vector<std::string> freetype_engine::face_names ()
{
    std::vector<std::string> names;
    std::map<std::string,std::pair<int,std::string> >::const_iterator itr;
    for (itr = name2file_.begin();itr!=name2file_.end();++itr)
    {
        names.push_back(itr->first);
    }
    return names;
}

std::map<std::string,std::pair<int,std::string> > const& freetype_engine::get_mapping()
{
    return name2file_;
}


face_ptr freetype_engine::create_face(std::string const& family_name)
{
    std::map<std::string, std::pair<int,std::string> >::iterator itr;
    itr = name2file_.find(family_name);
    if (itr != name2file_.end())
    {
        FT_Face face;
        FT_Error error = FT_New_Face (library_,
                                      itr->second.second.c_str(),
                                      itr->second.first,
                                      &face);
        if (!error)
        {
            return boost::make_shared<font_face>(face);
        }
    }
    return face_ptr();
}

stroker_ptr freetype_engine::create_stroker()
{
    FT_Stroker s;
    FT_Error error = FT_Stroker_New(library_, &s);
    if (!error)
    {
        return boost::make_shared<stroker>(s);
    }
    return stroker_ptr();
}

template <typename T>
face_ptr face_manager<T>::get_face(const std::string &name)
{
    face_ptr_cache_type::iterator itr;
    itr = face_ptr_cache_.find(name);
    if (itr != face_ptr_cache_.end())
    {
        return itr->second;
    }
    else
    {
        face_ptr face = engine_.create_face(name);
        if (face)
        {
            face_ptr_cache_.insert(make_pair(name,face));
        }
        return face;
    }
}

template <typename T>
face_set_ptr face_manager<T>::get_face_set(const std::string &name)
{
    face_set_ptr face_set = boost::make_shared<font_face_set>();
    if (face_ptr face = get_face(name))
    {
        face_set->add(face);
    }
    return face_set;
}

template <typename T>
face_set_ptr face_manager<T>::get_face_set(const font_set &fset)
{
    std::vector<std::string> const& names = fset.get_face_names();
    face_set_ptr face_set = boost::make_shared<font_face_set>();
    for (std::vector<std::string>::const_iterator name = names.begin(); name != names.end(); ++name)
    {
        face_ptr face = get_face(*name);
        if (face)
        {
            face_set->add(face);
        }
#ifdef MAPNIK_LOG
        else
        {
            MAPNIK_LOG_DEBUG(font_engine_freetype)
                    << "Failed to find face '" << *name
                    << "' in font set '" << fset.get_name() << "'\n";
        }
#endif
    }
    return face_set;
}

template <typename T>
face_set_ptr face_manager<T>::get_face_set(const std::string &name, const font_set &fset)
{
    if (fset.size() > 0)
    {
        return get_face_set(fset);
    }
    else
    {
        return get_face_set(name);
    }
}

#ifdef MAPNIK_THREADSAFE
boost::mutex freetype_engine::mutex_;
#endif
std::map<std::string,std::pair<int,std::string> > freetype_engine::name2file_;

void stroker::init(double radius)
{
    FT_Stroker_Set(s_, (FT_Fixed) (radius * (1<<6)),
                   FT_STROKER_LINECAP_ROUND,
                   FT_STROKER_LINEJOIN_ROUND,
                   0);
}

stroker::~stroker()
{
    MAPNIK_LOG_DEBUG(font_engine_freetype) << "stroker: Destroy stroker=" << s_;

    FT_Stroker_Done(s_);
}

template class face_manager<freetype_engine>;

}
