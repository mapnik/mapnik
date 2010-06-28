/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
 * Copyright (C) 2006 10East Corp.
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

#ifndef MAPNIK_SHIELD_SYMBOLIZER_HPP
#define MAPNIK_SHIELD_SYMBOLIZER_HPP

// mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/text_symbolizer.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

namespace mapnik
{
struct MAPNIK_DECL shield_symbolizer : public text_symbolizer,
                                       public symbolizer_with_image
{
    shield_symbolizer(expression_ptr name,
                      std::string const& face_name,
                      unsigned size,
                      color const& fill,
                      path_expression_ptr file);
    shield_symbolizer(expression_ptr name,
                      unsigned size,
                      color const& fill,
                      path_expression_ptr file);
    
    bool get_unlock_image() const;              // image is not locked to the text placement
    void set_unlock_image(bool unlock_image);
    bool get_no_text() const;                   // do no render text
    void set_no_text(bool unlock_image);
    void set_shield_displacement(double shield_dx,double shield_dy);
    boost::tuple<double,double> const& get_shield_displacement() const;
    
private:
    bool unlock_image_;
    bool no_text_;
    boost::tuple<double,double> shield_displacement_;
};
}

#endif // SHIELD_SYMBOLIZER_HPP
