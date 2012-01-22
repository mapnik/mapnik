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

#ifndef MAPNIK_TEXT_PATH_HPP
#define MAPNIK_TEXT_PATH_HPP

// mapnik
#include <mapnik/char_info.hpp>

//stl
#include <vector>

// boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

// uci
#include <unicode/unistr.h>

namespace mapnik
{
    
class string_info : private boost::noncopyable
{
protected:
    typedef std::vector<char_info> characters_t;
    characters_t characters_;
    UnicodeString text_;
    double width_;
    double height_;
    bool is_rtl;
public:
    string_info(UnicodeString const& text)
        : characters_(),
          text_(text),
          width_(0),
          height_(0),
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

    void add_text(UnicodeString text)
    {
        text_ += text;
    }

    void add_info(int c, double width, double height)
    {
        characters_.push_back(char_info(c, width, height, 0, height)); //WARNING: Do not use. Only to keep old code compilable.
    }
      
    unsigned num_characters() const
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
      
    char_info const& at(unsigned i) const
    {
        return characters_[i];
    }
      
    char_info const& operator[](unsigned i) const
    {
        return at(i);
    }
      
    void set_dimensions(double width, double height)
    {
        width_ = width;
        height_ = height;
    }
      
    std::pair<double, double> get_dimensions() const
    {
        return std::pair<double, double>(width_, height_);
    }

    UnicodeString const&  get_string() const 
    {
        return text_;
    }

    bool has_line_breaks() const
    {
       UChar break_char = '\n';
       return (text_.indexOf(break_char) >= 0);
    }

    /** Resets object to initial state. */
    void clear(void)
    {
        text_ = "";
        characters_.clear();
    }
};
    
struct text_path : boost::noncopyable
{
    struct character_node
    {
        int c;
        double x, y, angle;
        char_properties *format;
               
        character_node(int c_, double x_, double y_, double angle_, char_properties *format_)
            : c(c_), x(x_), y(y_), angle(angle_), format(format_) {}
        ~character_node() {}
               
        void vertex(int *c_, double *x_, double *y_, double *angle_, char_properties **format_)
        {
            *c_ = c;
            *x_ = x;
            *y_ = y;
            *angle_ = angle;
            *format_ = format;
        }
    };
         
    typedef std::vector<character_node> character_nodes_t;
    double starting_x;
    double starting_y;
    character_nodes_t nodes_;
    int itr_;
          
    std::pair<unsigned,unsigned> string_dimensions;
        
    text_path() 
        : starting_x(0),
          starting_y(0),
          itr_(0) {} 
         
    ~text_path() {}
          
    void add_node(int c, double x, double y, double angle, char_properties *format)
    {
        nodes_.push_back(character_node(c, x, y, angle, format));
    }
        
    void vertex(int *c, double *x, double *y, double *angle, char_properties **format)
    {
        nodes_[itr_++].vertex(c, x, y, angle, format);
    }
         
    void rewind()
    {
        itr_ = 0;
    }
         
    int num_nodes() const
    {
        return nodes_.size();
    }
         
    void clear()
    {
        nodes_.clear();
    }
};

typedef boost::shared_ptr<text_path> text_path_ptr;
}

#endif // MAPNIK_TEXT_PATH_HPP
