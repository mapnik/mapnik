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
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text_path.hpp>

//boost
#include <boost/shared_ptr.hpp>

namespace mapnik
{

typedef boost::shared_ptr<processed_text> processed_text_ptr;
typedef std::list<processed_text_ptr> processed_text_list;

class processed_text : boost::noncopyable
{
public:
    class processed_expression
    {
    public:
        processed_expression(char_properties const& properties, UnicodeString const& text) :
            p(properties), str(text) {}
        char_properties p;
        UnicodeString str;
    };
public:
    processed_text(face_manager<freetype_engine> & font_manager, double scale_factor, position displacement = position(0.0, 0.0));
    void push_back(char_properties const& properties, UnicodeString const& text);
    void add_child(processed_text_ptr child_text);
    unsigned size() const { return expr_list_.size(); }
    unsigned empty() const { return expr_list_.empty(); }
    void clear();
    typedef std::list<processed_expression> expression_list;
    expression_list::const_iterator begin() const;
    expression_list::const_iterator end() const;
    processed_text_list::const_iterator child_begin() const;
    processed_text_list::const_iterator child_end() const;
    string_info_ptr get_string_info();
    void get_offset_info(string_info_list &);
    
    face_manager<freetype_engine> &get_font_manager()
    {
        return font_manager_;
    }
    double get_scale_factor()
    {
        return scale_factor_;
    }
    position get_displacement()
    {
        return displacement_;
    }
    
private:
    expression_list expr_list_;
    processed_text_list child_text_list_;
    face_manager<freetype_engine> &font_manager_;
    double scale_factor_;
    position displacement_;
    string_info info_;
};

} // ns mapnik
#endif // PROCESSED_TEXT_HPP
