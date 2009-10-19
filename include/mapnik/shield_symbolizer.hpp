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

#ifndef SHIELD_SYMBOLIZER_HPP
#define SHIELD_SYMBOLIZER_HPP

#include <mapnik/graphics.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/symbolizer.hpp>

#include <boost/shared_ptr.hpp>

namespace mapnik
{
  struct MAPNIK_DECL shield_symbolizer : public text_symbolizer,
                                         public symbolizer_with_image
  {
    shield_symbolizer(std::string const& name,
                      std::string const& face_name,
                      unsigned size,
                      color const& fill,
                      std::string const& file,
                      std::string const& type,
                      unsigned width,unsigned height);
    shield_symbolizer(std::string const& name,
                      unsigned size,
                      color const& fill,
                      std::string const& file,
                      std::string const& type,
                      unsigned width,unsigned height);

                      bool get_unlock_image() const;              // image is not locked to the text placement
                      void set_unlock_image(bool unlock_image);
                      bool get_no_text() const;                   // do no render text
                      void set_no_text(bool unlock_image);

    private:

        bool unlock_image_;
        bool no_text_;
  };
}

#endif // SHIELD_SYMBOLIZER_HPP
