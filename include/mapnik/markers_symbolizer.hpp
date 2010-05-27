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

#ifndef MARKERS_SYMBOLIZER_HPP
#define MARKERS_SYMBOLIZER_HPP

//mapnik
#include <mapnik/path_expression_grammar.hpp>

namespace mapnik {
   
struct MAPNIK_DECL markers_symbolizer
{
public:
    markers_symbolizer(path_expression_ptr filename, bool allow_overlap=false) 
	: filename_(filename),
	allow_overlap_(allow_overlap) {}
    
    path_expression_ptr get_filename() const
    {
	return filename_;
    }
    
    void set_filename(path_expression_ptr filename)
    {
	filename_ = filename;
    }
    
    void set_allow_overlap(bool overlap)
    {
        allow_overlap_ = overlap;
    }
    
    float get_allow_overlap() const
    {
        return allow_overlap_;
    }
private:
    path_expression_ptr filename_;
    bool allow_overlap_;
};
}

#endif // MARKERS_SYMBOLIZER_HPP
