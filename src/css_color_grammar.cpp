/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

// NOTE: we define this here in a cpp because def is needed twice:
// once by src/color_factory.cpp and once by include/mapnik/image_filter_grammar.hpp
// otherwise it would make sense to simply do `#include <mapnik/css_color_grammar_impl.hpp>`
// in a single file
#include <mapnik/color.hpp>
#include <mapnik/css_color_grammar_impl.hpp>
#include <string>

template struct mapnik::css_color_grammar<std::string::const_iterator>;
