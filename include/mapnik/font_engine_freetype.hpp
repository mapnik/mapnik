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
#include <mapnik/debug.hpp>
#include <mapnik/color.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/text/face.hpp>

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_STROKER_H
}

// boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
#endif

// stl
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>

// uci
#include <unicode/unistr.h>

namespace mapnik
{

// FT_Stroker wrapper
class stroker : boost::noncopyable
{
public:
    explicit stroker(FT_Stroker s)
        : s_(s) {}
    ~stroker();

    void init(double radius);

    FT_Stroker const& get() const { return s_; }
private:
    FT_Stroker s_;
};
typedef boost::shared_ptr<stroker> stroker_ptr;


class MAPNIK_DECL freetype_engine
{
public:
    static bool is_font_file(std::string const& file_name);
    static bool register_font(std::string const& file_name);
    static bool register_fonts(std::string const& dir, bool recurse = false);
    static std::vector<std::string> face_names();
    static std::map<std::string,std::pair<int,std::string> > const& get_mapping();
    face_ptr create_face(std::string const& family_name);
    stroker_ptr create_stroker();
    virtual ~freetype_engine();
    freetype_engine();
private:
    FT_Library library_;
#ifdef MAPNIK_THREADSAFE
    static boost::mutex mutex_;
#endif
    static std::map<std::string, std::pair<int,std::string> > name2file_;
};

template <typename T>
class MAPNIK_DECL face_manager : private boost::noncopyable
{
    typedef T font_engine_type;
    typedef std::map<std::string,face_ptr> face_ptr_cache_type;

public:
    face_manager(T & engine)
        : engine_(engine),
        stroker_(engine_.create_stroker()),
        face_ptr_cache_()  {}

    face_ptr get_face(std::string const& name)
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

    face_set_ptr get_face_set(std::string const& name)
    {
        face_set_ptr face_set = boost::make_shared<font_face_set>();
        if (face_ptr face = get_face(name))
        {
            face_set->add(face);
        }
        return face_set;
    }

    face_set_ptr get_face_set(font_set const& fset)
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

    face_set_ptr get_face_set(std::string const& name, font_set const& fset)
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

    stroker_ptr get_stroker()
    {
        return stroker_;
    }

private:
    font_engine_type & engine_;
    stroker_ptr stroker_;
    face_ptr_cache_type face_ptr_cache_;
};

template <typename T>
struct text_renderer : private boost::noncopyable
{
    struct glyph_t : boost::noncopyable
    {
        FT_Glyph image;
        char_properties *properties;
        glyph_t(FT_Glyph image_, char_properties *properties_)
            : image(image_), properties(properties_) {}
        ~glyph_t () { FT_Done_Glyph(image);}
    };

    typedef boost::ptr_vector<glyph_t> glyphs_t;
    typedef T pixmap_type;

    text_renderer (pixmap_type & pixmap, face_manager<freetype_engine> &font_manager_, stroker & s, composite_mode_e comp_op = src_over);
    void render(pixel_position pos);
    void render_id(int feature_id, pixel_position pos, double min_radius=1.0);

private:
    void render_bitmap(FT_Bitmap *bitmap, unsigned rgba, int x, int y, double opacity);
    void render_bitmap_id(FT_Bitmap *bitmap,int feature_id,int x,int y);

    pixmap_type & pixmap_;
    face_manager<freetype_engine> &font_manager_;
    stroker & stroker_;
    glyphs_t glyphs_;
    composite_mode_e comp_op_;
};

typedef face_manager<freetype_engine> face_manager_freetype;

}

#endif // MAPNIK_FONT_ENGINE_FREETYPE_HPP
