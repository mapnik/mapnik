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
#ifndef TEXT_PROPERTIES_HPP
#define TEXT_PROPERTIES_HPP

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/font_set.hpp>


// stl
#include <map>

// boost
#include <boost/property_tree/ptree.hpp>

namespace mapnik
{

enum text_transform
{
    NONE = 0,
    UPPERCASE,
    LOWERCASE,
    CAPITALIZE,
    text_transform_MAX
};
DEFINE_ENUM( text_transform_e, text_transform );

struct char_properties
{
    char_properties();
    /** Construct object from XML. */
    void from_xml(boost::property_tree::ptree const &sym, std::map<std::string,font_set> const & fontsets);
    /** Write object to XML ptree. */
    void to_xml(boost::property_tree::ptree &node, bool explicit_defaults, char_properties const &dfl=char_properties()) const;
    std::string face_name;
    font_set fontset;
    float text_size;
    double character_spacing;
    double line_spacing; //Largest total height (fontsize+line_spacing) per line is chosen
    double text_opacity;
    bool wrap_before;
    unsigned wrap_char;
    text_transform_e text_transform; //Per expression
    color fill;
    color halo_fill;
    double halo_radius;
};

} //ns mapnik

#endif // TEXT_PROPERTIES_HPP
