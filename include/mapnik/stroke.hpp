/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
//$Id$

#ifndef STROKE_HPP
#define STROKE_HPP
// mapnik
#include <mapnik/color.hpp>
#include <mapnik/enumeration.hpp>
// stl
#include <vector>
 
namespace mapnik
{
    using std::pair;
    using std::vector;
    typedef vector<pair<float,float> > dash_array;
    
    // if you add new tokens, don't forget to add them to the corresponding
    // string array in the cpp file.
    enum line_cap_enum
        {
            BUTT_CAP,
            SQUARE_CAP,
            ROUND_CAP,
            line_cap_enum_MAX
        }; 

    DEFINE_ENUM( line_cap_e, line_cap_enum );
    
    // if you add new tokens, don't forget to add them to the corresponding
    // string array in the cpp file.
    enum line_join_enum
        {
            MITER_JOIN,
            MITER_REVERT_JOIN,
            ROUND_JOIN,
            BEVEL_JOIN,
            line_join_enum_MAX
        };

    DEFINE_ENUM( line_join_e, line_join_enum );
    
  class MAPNIK_DECL stroke
  {	
        color c_;
        float width_;
        float opacity_; // 0.0 - 1.0
        line_cap_e  line_cap_;
        line_join_e line_join_;
        dash_array dash_;	
    public:
        explicit stroke();
        stroke(color const& c, float width=1.0);
        stroke(stroke const& other);
        stroke& operator=(const stroke& rhs);

        void set_color(const color& c);
	
        color const& get_color() const;
	
        float get_width() const;
        void set_width(float w);
        void set_opacity(float opacity);

        float get_opacity() const;
	
        void set_line_cap(line_cap_e line_cap);
        line_cap_e get_line_cap() const;
	
        void set_line_join(line_join_e line_join);
        line_join_e get_line_join() const;
	
        void add_dash(float dash,float gap);
        bool has_dash() const;
	
        dash_array const& get_dash_array() const;
	
    private:
        void swap(const stroke& other) throw();
    };
}

#endif //STROKE_HPP
