/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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
#ifndef PROCESSED_TEXT_HPP
#define PROCESSED_TEXT_HPP

// mapnik
#include <mapnik/text_properties.hpp>
#include <mapnik/text_path.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/font_engine_freetype.hpp> //!!
#include <mapnik/skia/skia_font_manager.hpp>
// stl
#include <list>
// icu (temp)
#include <unicode/schriter.h>

namespace mapnik {

// fwd declares
//class freetype_engine;
//template <typename T> class face_manager;

class MAPNIK_DECL processed_text : mapnik::noncopyable
{
public:
    struct processed_expression
    {
        processed_expression(char_properties const& properties, UnicodeString const& text)
            : p(properties), str(text) {}
        char_properties p;
        UnicodeString str;
    };
public:
    processed_text(double scale_factor);
    void push_back(char_properties const& properties, UnicodeString const& text);
    unsigned size() const { return expr_list_.size(); }
    unsigned empty() const { return expr_list_.empty(); }
    void clear();
    typedef std::list<processed_expression> expression_list;
    expression_list::const_iterator begin() const;
    expression_list::const_iterator end() const;

    template <typename T>
    string_info const& get_string_info(T & font_manager)
    {
        info_.clear(); //if this function is called twice invalid results are returned, so clear string_info first
        expression_list::iterator itr = expr_list_.begin();
        expression_list::iterator end = expr_list_.end();
        for (; itr != end; ++itr)
        {
            char_properties const& p = itr->p;
            std::cerr << p.face_name << " : " << p.fontset << std::endl;
            face_set_ptr faces = font_manager.get_face_set(p.face_name, p.fontset);
            if (faces->size() > 0)
            {
                faces->set_character_sizes(p.text_size * scale_factor_); // ???
                faces->get_string_info(info_, itr->str, &(itr->p));
                info_.add_text(itr->str);
            }
        }
        return info_;
    }
#if defined(HAVE_SKIA)
    string_info const& get_string_info(skia_font_manager & manager)
    {
        info_.clear(); //if this function is called twice invalid results are returned, so clear string_info first
        expression_list::iterator itr = expr_list_.begin();
        expression_list::iterator end = expr_list_.end();
        for (; itr != end; ++itr)
        {
            char_properties const& p = itr->p;
            if (p.fontset)
            {
                for (auto const& face_name : p.fontset->get_face_names())
                {
                    std::cerr << face_name << std::endl;
                }
            }

            StringCharacterIterator iter(itr->str);
            for (iter.setToStart(); iter.hasNext();)
            {
                UChar ch = iter.nextPostInc();
                char_info char_dim(ch, 10, 12, 0, 12);
                char_dim.format = &(itr->p);
                char_dim.avg_height = 12;//avg_height;
                info_.add_info(char_dim);
            }
            // char sizes ---> p.text_size * scale_factor_
            info_.add_text(itr->str);
        }

        return info_;
    }
#endif
    //string_info const& get_string_info();
private:

    expression_list expr_list_;
    double scale_factor_;
    string_info info_;
};

} // ns mapnik
#endif // PROCESSED_TEXT_HPP
