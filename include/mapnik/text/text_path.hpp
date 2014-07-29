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

#ifndef MAPNIK_TEXT_PATH_HPP
#define MAPNIK_TEXT_PATH_HPP

// mapnik
#include <mapnik/text/char_info.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/value_types.hpp>

//stl
#include <vector>

// boost
#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>

namespace mapnik
{

class string_info : private mapnik::noncopyable
{
protected:
    using characters_t = std::vector<char_info>;
    characters_t characters_;
    mapnik::value_unicode_string text_;
    bool is_rtl;
public:
    string_info(mapnik::value_unicode_string const& text)
        : characters_(),
          text_(text),
          is_rtl(false)
    {

    }

    string_info()
        : characters_(),
          text_(),
          is_rtl(false)
    {

    }

    void add_info(char_info const& info)
    {
        characters_.push_back(info);
    }

    void add_text(mapnik::value_unicode_string const& text)
    {
        text_ += text;
    }

    std::size_t num_characters() const
    {
        return characters_.size();
    }

    void set_rtl(bool value)
    {
        is_rtl = value;
    }

    bool get_rtl() const
    {
        return is_rtl;
    }

    char_info const& at(std::size_t i) const
    {
        return characters_[i];
    }

    char_info const& operator[](std::size_t i) const
    {
        return at(i);
    }

    mapnik::value_unicode_string const& get_string() const
    {
        return text_;
    }

    bool has_line_breaks() const
    {
        // uint16_t
        UChar break_char = '\n';
        return (text_.indexOf(break_char) >= 0);
    }

    // Resets object to initial state.
    void clear()
    {
        text_.remove();
        characters_.clear();
    }
};

using char_info_ptr = char_info const *;


// List of all characters and their positions and formats for a placement.
class text_path : mapnik::noncopyable
{
    struct character_node
    {
        char_info_ptr c;
        pixel_position pos;
        double angle;

        character_node(char_info_ptr c_, double x_, double y_, double angle_)
            : c(c_),
              pos(x_, y_),
              angle(angle_)
        {

        }

        ~character_node() {}

        void vertex(char_info_ptr & c_, double & x_, double  & y_, double & angle_) const
        {
            c_ = c;
            x_ = pos.x;
            y_ = pos.y;
            angle_ = angle;
        }
    };

    mutable int itr_;
public:
    using character_nodes_t = std::vector<character_node>;
    pixel_position center;
    character_nodes_t nodes_;

    text_path(double x, double y)
        : itr_(0),
          center(x,y),
          nodes_()
    {

    }

    ~text_path() {}

    /** Adds a new char to the list. */
    void add_node(char_info_ptr c, double x, double y, double angle)
    {
        nodes_.push_back(character_node(c, x, y, angle));
    }

    /** Return node. Always returns a new node. Has no way to report that there are no more nodes. */
    void vertex(char_info_ptr & c, double & x, double & y, double & angle) const
    {
        nodes_[itr_++].vertex(c, x, y, angle);
    }

    /** Start again at first node. */
    void rewind() const
    {
        itr_ = 0;
    }

    /** Number of nodes. */
    std::size_t num_nodes() const
    {
        return nodes_.size();
    }

    /** Delete all nodes. */
    void clear()
    {
        nodes_.clear();
    }
};

using text_path_ptr = std::shared_ptr<text_path>;
using placements_type = boost::ptr_vector<text_path>;
}

#endif // MAPNIK_TEXT_PATH_HPP
