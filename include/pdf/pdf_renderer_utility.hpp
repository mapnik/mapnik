/*****************************************************************************
 * 
 * This file is part of the wxPdfDoc modifications for mapnik
 *
 * Copyright (C) 2007 Ben Moores
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

//! \file pdf_renderer_utility.hpp
//! \brief PDF renderer utilities
//! 

#ifndef PDF_RENDERER_UTILITY_H
#define PDF_RENDERER_UTILITY_H

namespace mapnik {

#include <mapnik/config.hpp>

//class prototypes
class Map;
class pdf_renderer_layout;


//! \brief Render a map to pdf using a layout
//!
//! \param[in] map The map to render
//! \param[in] layout The page layouit
//! \param[in] filename The output file name
//! \param[in] compress Enable/Disable pdf compression
MAPNIK_DECL void render_to_pdf(const mapnik::Map& map, const mapnik::pdf_renderer_layout &layout, const std::string& filename, const bool compress = true);


} //namespace mapnik

#endif //PDF_RENDERER_UTILITY_H
