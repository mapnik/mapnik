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

//! \file pdf_renderer_utility.cpp
//! \brief Implementation of pdf_renderer_utility.hpp
//!

#if ENABLE_PDF

// mapnik
#include <mapnik/map.hpp>
#include <pdf/pdf_renderer_layout.hpp>
#include <pdf/pdf_renderer.hpp>

// WX
#include <wx/pdfdoc.h>


namespace mapnik {

MAPNIK_DECL void render_to_pdf(const mapnik::Map& map, const mapnik::pdf_renderer_layout &layout, const std::string& filename, const bool compress) {
  using namespace mapnik;

  //create the pdf
  wxPdfDocument pdf(layout.get_page_width(), layout.get_page_height(), layout.get_page_portrait()?wxPORTRAIT:wxLANDSCAPE, pdf_renderer::convert_string_to_wstring(layout.get_page_units()));

  //set pdf options
  pdf.SetMargins(0,0,0);
  pdf.SetCompression(compress);

  //create the renderer
  pdf_renderer pr(map, pdf, layout, 0, 0);

  //render
  pr.apply();

  //save to file
  pdf.SaveAsFile(pdf_renderer::convert_string_to_wstring(filename));
}


} //namespace mapnik


#endif //#if ENABLE_PDF