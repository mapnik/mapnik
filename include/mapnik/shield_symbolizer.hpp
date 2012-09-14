/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
    // Note - we do not use boost::make_shared below as VC2008 and VC2010 are
    // not able to compile make_shared used within a constructor
    shield_symbolizer(text_placements_ptr placements = text_placements_ptr(new text_placements_dummy));
    shield_symbolizer(expression_ptr name,
                      std::string const& face_name,
                      float size,
                      color const& fill,
                      path_expression_ptr file);
    shield_symbolizer(expression_ptr name,
                      float size,
                      color const& fill,
                      path_expression_ptr file);

    bool get_unlock_image() const;              // image is not locked to the text placement
    void set_unlock_image(bool unlock_image);
    void set_shield_displacement(double shield_dx, double shield_dy);
    position const& get_shield_displacement() const;

private:
    bool unlock_image_;
    position shield_displacement_;
};
}

#endif // MAPNIK_SHIELD_SYMBOLIZER_HPP
