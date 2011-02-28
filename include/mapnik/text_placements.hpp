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

//stl
#include <vector>
#include <string>

//boost
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>

namespace mapnik {

typedef boost::tuple<double,double> position;

class text_placement_info
{
public:
    /** Get next placement.
      * This function is also called before the first placement is tried. */
    virtual bool next()=0;
    /** Get next placement position.
      * This function is also called before the first position is used.
      * Each class has to return at least one position!
      * If this functions returns false the placement data should be considered invalid!
      */
    virtual bool next_position_only()=0;

    /* NOTE: Values are public and non-virtual to avoid any performance problems. */
    position displacement;
    unsigned text_size;
};

typedef boost::shared_ptr<text_placement_info> text_placement_info_ptr;

class text_placements
{
public:
    text_placements() : text_size_(10) {}
    virtual text_placement_info_ptr get_placement_info() const =0;
    virtual void set_default_text_size(unsigned size) { text_size_ = size; }
    unsigned get_default_text_size() const { return text_size_; }
    virtual void set_default_displacement(position const& displacement) { displacement_ = displacement;}
    position const& get_default_displacement() { return displacement_; }
protected:
    unsigned text_size_;
    position displacement_;
};

typedef boost::shared_ptr<text_placements> text_placements_ptr;

class text_placements_dummy;
class text_placement_info_dummy : public text_placement_info
{
public:
    text_placement_info_dummy(text_placements_dummy const* parent) : state(0), position_state(0), parent_(parent) {}
    bool next();
    bool next_position_only();
private:
    unsigned state;
    unsigned position_state;
    text_placements_dummy const* parent_;
};

class text_placements_dummy: public text_placements
{
public:
    text_placement_info_ptr get_placement_info() const;
    friend class text_placement_info_dummy;
};

} //namespace

#endif // TEXT_PLACEMENTS_HPP
