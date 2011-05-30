/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Hermann Kraus
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
#ifndef TEXT_PLACEMENTS_HPP
#define TEXT_PLACEMENTS_HPP

//mapnik
#include <mapnik/enumeration.hpp>

//stl
#include <vector>
#include <string>

//boost
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

namespace mapnik {

typedef boost::tuple<double,double> position;

enum label_placement_enum {
    POINT_PLACEMENT,
    LINE_PLACEMENT,
    VERTEX_PLACEMENT,
    INTERIOR_PLACEMENT,
    label_placement_enum_MAX
};

DEFINE_ENUM( label_placement_e, label_placement_enum );

enum vertical_alignment
{
    V_TOP = 0,
    V_MIDDLE,
    V_BOTTOM,
    V_AUTO,
    vertical_alignment_MAX
};

DEFINE_ENUM( vertical_alignment_e, vertical_alignment );

enum horizontal_alignment
{
    H_LEFT = 0,
    H_MIDDLE,
    H_RIGHT,
    H_AUTO,
    horizontal_alignment_MAX
};

DEFINE_ENUM( horizontal_alignment_e, horizontal_alignment );

enum justify_alignment
{
    J_LEFT = 0,
    J_MIDDLE,
    J_RIGHT,
    justify_alignment_MAX
};

DEFINE_ENUM( justify_alignment_e, justify_alignment );

enum text_transform
{
    NONE = 0,
    UPPERCASE,
    LOWERCASE,
    CAPITALIZE,
    text_transform_MAX
};

DEFINE_ENUM( text_transform_e, text_transform );

class text_placements;

class text_placement_info : boost::noncopyable
{
public:
    text_placement_info(text_placements const* parent);
    /** Get next placement.
      * This function is also called before the first placement is tried. */
    virtual bool next()=0;
    /** Get next placement position.
      * This function is also called before the first position is used.
      * Each class has to return at least one position!
      * If this functions returns false the placement data should be considered invalid!
      */
    virtual bool next_position_only()=0;
    virtual ~text_placement_info() {}

    /* NOTE: Values are public and non-virtual to avoid any performance problems. */
    position displacement;
    unsigned text_size;
    horizontal_alignment_e halign;
    justify_alignment_e jalign;
    vertical_alignment_e valign;
};

typedef boost::shared_ptr<text_placement_info> text_placement_info_ptr;

class text_placements
{
public:
    text_placements() :
        text_size_(10), halign_(H_MIDDLE), jalign_(J_MIDDLE), valign_(V_MIDDLE) {}
    virtual text_placement_info_ptr get_placement_info() const =0;

    virtual void set_default_text_size(unsigned size) { text_size_ = size; }
    unsigned get_default_text_size() const { return text_size_; }

    virtual void set_default_displacement(position const& displacement) { displacement_ = displacement;}
    position const& get_default_displacement() { return displacement_; }

    virtual void set_default_halign(horizontal_alignment_e const& align) { halign_ = align;}
    horizontal_alignment_e const& get_default_halign() { return halign_; }

    virtual void set_default_jalign(justify_alignment_e const& align) { jalign_ = align;}
    justify_alignment_e const& get_default_jalign() { return jalign_; }

    virtual void set_default_valign(vertical_alignment_e const& align) { valign_ = align;}
    vertical_alignment_e const& get_default_valign() { return valign_; }

    virtual ~text_placements() {}
protected:
    unsigned text_size_;
    position displacement_;
    horizontal_alignment_e halign_;
    justify_alignment_e jalign_;
    vertical_alignment_e valign_;
    friend class text_placement_info;
};

typedef boost::shared_ptr<text_placements> text_placements_ptr;

class text_placements_info_dummy;

class text_placements_dummy: public text_placements
{
public:
    text_placement_info_ptr get_placement_info() const;
    friend class text_placement_info_dummy;
};

class text_placement_info_dummy : public text_placement_info
{
public:
    text_placement_info_dummy(text_placements_dummy const* parent) : text_placement_info(parent),
        state(0), position_state(0), parent_(parent) {}
    bool next();
    bool next_position_only();
private:
    unsigned state;
    unsigned position_state;
    text_placements_dummy const* parent_;
};



} //namespace

#endif // TEXT_PLACEMENTS_HPP
