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
//$Id: symbolizer.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef MAPNIK_SYMBOLIZER_HPP
#define MAPNIK_SYMBOLIZER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/path_expression_grammar.hpp>

// boost
#include <boost/array.hpp>

namespace mapnik 
{

typedef boost::array<double,6> transform_type;

class MAPNIK_DECL symbolizer_with_image {
public:
    path_expression_ptr get_filename() const;
    void set_filename(path_expression_ptr filename);
    void set_transform(transform_type const& );
    transform_type const& get_transform() const;
    void set_opacity(float opacity);
    float get_opacity() const;
protected:
    symbolizer_with_image(path_expression_ptr filename);
    symbolizer_with_image(symbolizer_with_image const& rhs);
    path_expression_ptr image_filename_;   
    float opacity_;
    transform_type matrix_;
};
}

#endif //MAPNIK_SYMBOLIZER_HPP
