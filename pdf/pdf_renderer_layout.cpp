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

//! \file pdf_renderer_layout.cpp
//! \brief Implementation of pdf_renderer_layout.hpp
//!

#if ENABLE_PDF

// mapnik
#include <mapnik/map.hpp>

// pdf
#include <pdf/pdf_renderer.hpp>
#include <pdf/pdf_renderer_layout.hpp>
#include <pdf/font_engine_pdf.hpp>

namespace mapnik 
{

  //---------------------------------------------------------------------------
  pdf_renderer_layout::pdf_renderer_layout(const std::string &units, const double width, const double height, const bool portrait)
    : page_area(Envelope<double>(0,0, width, height)),
      page_units(units)
  {
    page_background_colour = Color(255, 255, 255);
    page_portrait = portrait;

    map_area = page_area;
    map_area_border = false;
    map_area_border_width = 0.5;

    font_name = "Arial";

    border_scales = true;
    map_grid = false;
    map_grid_approx_spacing = 35; 
    border_scale_width = 4.5;
    border_scale_linewidth = 0.1; 
    map_grid_colour = Color(0,0,0,64);
    map_grid_linewidth = 0.25;

    scale_bar = true;
    scale_bar_area = Envelope<double>(20,20,100,27.5);
    scale_bar_factor = 0.001;
    scale_bar_unit = "Kilometers";
  }

  //---------------------------------------------------------------------------
  pdf_renderer_layout::~pdf_renderer_layout()
  {
    //remove all data in overlay images vector
    OIDitr itr;
    for(itr = overlay_images.begin(); itr != overlay_images.end(); itr++) {
      delete *itr;
      *itr = NULL;
    }

  }

  //---------------------------------------------------------------------------
  void pdf_renderer_layout::set_map_area(const double x, const double y, const double width, const double height)
  {
    map_area = Envelope<double>(x, y, x+width, y+height);

    if(!page_area.contains(map_area))
    {
      map_area = page_area;
      std::clog << "WARNING: specified map area is not within the page area. Setting map area to page area.";
    }
  }

  void pdf_renderer_layout::add_overlay_image(const std::string &image, const double x, const double y, const double width, const double height, const double angle)
  {
    overlay_image_data *data = new overlay_image_data;
    data->file = image;
    data->x = x;
    data->y = y;
    data->width = width;
    data->height = height;
    data->angle = angle;

    overlay_images.push_back(data);
  }



} //namespace mapnik


#endif // #if ENABLE_PDF

