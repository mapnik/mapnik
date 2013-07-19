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

#include <mapnik/processed_text.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/font_engine_freetype.hpp>

namespace mapnik
{

void processed_text::push_back(char_properties const& properties, UnicodeString const& text)
{
    expr_list_.push_back(processed_expression(properties, text));
}

processed_text::expression_list::const_iterator processed_text::begin() const
{
    return expr_list_.begin();
}

processed_text::expression_list::const_iterator processed_text::end() const
{
    return expr_list_.end();
}

processed_text::processed_text( double scale_factor)
    : scale_factor_(scale_factor) {}

void processed_text::clear()
{
    info_.clear();
    expr_list_.clear();
}

} //ns mapnik
