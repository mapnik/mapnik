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
#ifndef MAPNIK_PLACEMENTS_LIST_HPP
#define MAPNIK_PLACEMENTS_LIST_HPP
//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/glyph_info.hpp>

//stl
#include <vector>
#include <list>

namespace mapnik
{

struct glyph_info;

struct glyph_position
{
    glyph_position(glyph_info const& glyph, pixel_position const& pos, double angle)
        : glyph(&glyph), pos(pos), angle(angle) { }
    glyph_info const* glyph;
    pixel_position pos;
    double angle;
};

/** Stores positions of glphys.
 *
 * The actual glyphs and their format is stored in text_layout.
 */
class glyph_positions
{
public:
    typedef std::vector<glyph_position>::const_iterator const_iterator;
    glyph_positions();

    const_iterator begin() const;
    const_iterator end() const;

    void push_back(glyph_info const& glyph, pixel_position offset, double angle);

    /** Is each character rotated by the same angle?
     * This function is used to avoid costly trigonometric function calls when not necessary. */
    bool is_constant_angle() const;
    double get_angle() const;

    pixel_position const& get_base_point() const;
    void set_base_point(pixel_position base_point);
private:
    std::vector<glyph_position> data_;
    pixel_position base_point_;
    double angle_;
    bool const_angle_;
};
typedef boost::shared_ptr<glyph_positions> glyph_positions_ptr;

#if 0
class placements_list
{
public:
    placements_list();
    void push_back(glyph_positions_ptr glyphs);
    typedef std::list<glyph_positions_ptr> list_type;
    typedef list_type::const_iterator const_iterator;
    const_iterator begin() const;
    const_iterator end() const;
private:
    list_type placements_;
};
#endif

typedef std::list<glyph_positions_ptr> placements_list;
}
#endif // PLACEMENTS_LIST_HPP
