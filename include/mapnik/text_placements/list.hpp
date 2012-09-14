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
#ifndef TEXT_PLACEMENTS_LIST_HPP
#define TEXT_PLACEMENTS_LIST_HPP
#include <mapnik/text_placements/base.hpp>

namespace mapnik {

class text_placement_info_list;


/** Tries a list of placements. */
class text_placements_list: public text_placements
{
public:
    text_placements_list();
    text_placement_info_ptr get_placement_info(double scale_factor) const;
    virtual void add_expressions(expression_set &output);
    text_symbolizer_properties & add();
    text_symbolizer_properties & get(unsigned i);
    unsigned size() const;
    static text_placements_ptr from_xml(xml_node const &xml, fontset_map const & fontsets);
private:
    std::vector<text_symbolizer_properties> list_;
    friend class text_placement_info_list;
};

/** List placement strategy.
 * See parent class for documentation of each function. */
class text_placement_info_list : public text_placement_info
{
public:
    text_placement_info_list(text_placements_list const* parent, double scale_factor) :
        text_placement_info(parent, scale_factor),
        state(0), parent_(parent) {}
    bool next();
private:
    unsigned state;
    text_placements_list const* parent_;
};


} //namespace
#endif
