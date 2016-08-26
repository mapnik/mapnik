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
#include <mapnik/box2d.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/char_info.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/pixel_position.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/foreach.hpp>
#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
#endif

// stl
#include <string>
#include <vector>
#include <map>

struct FT_LibraryRec_;

namespace mapnik
{
class font_face;
class text_path;
class string_info;
struct char_properties;
class stroker;
struct glyph_t;

typedef boost::shared_ptr<font_face> face_ptr;

class MAPNIK_DECL font_glyph : private mapnik::noncopyable
{
public:
    font_glyph(face_ptr face, unsigned index)
        : face_(face), index_(index) {}

    face_ptr get_face() const
    {
        return face_;
    }

    unsigned get_index() const
    {
        return index_;
    }
private:
    face_ptr face_;
    unsigned index_;
};

typedef boost::shared_ptr<font_glyph> glyph_ptr;



class MAPNIK_DECL font_face_set : private mapnik::noncopyable
{
public:
    typedef std::vector<face_ptr> container_type;
    typedef container_type::size_type size_type;

    font_face_set(void)
        : faces_(),
        dimension_cache_() {}

    void add(face_ptr face);
    size_type size() const;
    glyph_ptr get_glyph(unsigned c) const;
    char_info character_dimensions(unsigned c);
    void get_string_info(string_info & info, mapnik::value_unicode_string const& ustr, char_properties *format);
    void set_pixel_sizes(unsigned size);
    void set_character_sizes(double size);
private:
    container_type faces_;
    std::map<unsigned, char_info> dimension_cache_;
};

typedef boost::shared_ptr<font_face_set> face_set_ptr;
typedef boost::shared_ptr<stroker> stroker_ptr;

class MAPNIK_DECL freetype_engine
{
public:
    typedef std::map<std::string,std::pair<int,std::string> > font_file_mapping_type;
    static bool is_font_file(std::string const& file_name);

    /*! \brief register a font file
     *  @param file_name path to a font file.
     *  @return bool - true if at least one face was successfully registered in the file.
     */
    static bool register_font(std::string const& file_name);

    /*! \brief register a font file using a local mapping
     *  @param file_name path to a font file.
     *  @param font_mapping a std::map holding association between a family name and a font file.
     *  @return bool - true if at least one face was successfully registered in the file.
     */
    static bool register_font(std::string const& file_name,
                                    font_file_mapping_type & font_mapping);

    /*! \brief register a font file
     *  @param dir - path to a directory containing fonts or subdirectories.
     *  @param recurse - default false, whether to search for fonts in sub directories.
     *  @return bool - true if at least one face was successfully registered.
     */
    static bool register_fonts(std::string const& dir, bool recurse = false);
    static bool register_fonts(std::string const& dir, font_file_mapping_type & font_mapping, bool recurse = false);
    static std::vector<std::string> face_names();
    static font_file_mapping_type const& get_mapping();
    face_ptr create_face(std::string const& family_name,
                         font_file_mapping_type const& font_mapping,
                         std::map<std::string, std::string> & memory_fonts);
    stroker_ptr create_stroker();
    virtual ~freetype_engine();
    freetype_engine();
private:
    FT_LibraryRec_ * library_;
#ifdef MAPNIK_THREADSAFE
    static boost::mutex mutex_;
#endif
    static font_file_mapping_type name2file_;
};

template <typename T>
class MAPNIK_DECL face_manager : private mapnik::noncopyable
{
    typedef T font_engine_type;
    typedef std::map<std::string,face_ptr> face_ptr_cache_type;

public:
    face_manager(T & engine,
                 freetype_engine::font_file_mapping_type const& font_mapping)
        : engine_(engine),
        memory_fonts_(),
        font_file_mapping_(font_mapping),
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
            face_ptr face = engine_.create_face(name,font_file_mapping_,memory_fonts_);
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
        BOOST_FOREACH( std::string const& name, names)
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

    face_set_ptr get_face_set(std::string const& name, boost::optional<font_set> fset)
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

    inline stroker_ptr get_stroker()
    {
        return stroker_;
    }

private:
    font_engine_type & engine_;
    std::map<std::string, std::string> memory_fonts_;
    freetype_engine::font_file_mapping_type const& font_file_mapping_;
    stroker_ptr stroker_;
    face_ptr_cache_type face_ptr_cache_;
};

template <typename T>
struct text_renderer : private mapnik::noncopyable
{

    typedef boost::ptr_vector<glyph_t> glyphs_t;
    typedef T pixmap_type;

    text_renderer (pixmap_type & pixmap,
                   face_manager<freetype_engine> & font_manager,
                   halo_rasterizer_e rasterizer,
                   composite_mode_e comp_op = src_over,
                   double scale_factor=1.0);
    box2d<double> prepare_glyphs(text_path const& path);
    void render(pixel_position const& pos);
    void render_id(mapnik::value_integer feature_id,
                   pixel_position const& pos);
private:
    pixmap_type & pixmap_;
    face_manager<freetype_engine> & font_manager_;
    halo_rasterizer_e rasterizer_;
    glyphs_t glyphs_;
    composite_mode_e comp_op_;
    double scale_factor_;
};

typedef face_manager<freetype_engine> face_manager_freetype;

}

#endif // MAPNIK_FONT_ENGINE_FREETYPE_HPP
