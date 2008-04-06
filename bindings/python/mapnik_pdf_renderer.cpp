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

#include <boost/python.hpp>
#include <pdf/pdf_renderer.hpp>
#include <pdf/pdf_renderer_layout.hpp>
#include <pdf/pdf_renderer_utility.hpp>


void export_pdf_renderer_layout()
{
    using namespace boost::python;
    using mapnik::pdf_renderer_layout;
    
    class_<pdf_renderer_layout>("PDFRendererLayout", init<const std::string, const double, const double, const bool>("Default PDF Renderer Layout"))
        .def ("set_font", &mapnik::pdf_renderer_layout::set_font, "Set the font to use for overlays")
        .def ("set_background_colour", &mapnik::pdf_renderer_layout::set_background_colour, "set the page background colour")
        .def ("set_map_area", &mapnik::pdf_renderer_layout::set_map_area, "Set the area of the page to render the map")
        .def ("set_map_area_border", &mapnik::pdf_renderer_layout::set_map_area_border, "Set whether to draw a map border")
        .def ("set_map_area_border_width", &mapnik::pdf_renderer_layout::set_map_area_border_width, "Set the map border width")
        .def ("add_overlay_image", &mapnik::pdf_renderer_layout::add_overlay_image, "Add an overlay image")
        .def ("set_map_grid", &mapnik::pdf_renderer_layout::set_map_grid, "Enable/Disable drawing map grid")
        .def ("set_border_scales", &mapnik::pdf_renderer_layout::set_border_scales, "Enable/Disable drawing scales around edge of map")
        .def ("set_map_grid_approx_spacing", &mapnik::pdf_renderer_layout::set_map_grid_approx_spacing, "Sets the approximate spacing of the grid lines")
        .def ("set_border_scale_width", &mapnik::pdf_renderer_layout::set_border_scale_width, "Set width of scale bars around edge of map")
        .def ("set_border_scale_linewidth", &mapnik::pdf_renderer_layout::set_border_scale_linewidth, "Set thickness of lines around the grid edge")
        .def ("set_map_grid_colour", &mapnik::pdf_renderer_layout::set_map_grid_colour, "Set the colour of the grid across the map")
        .def ("set_map_grid_linewidth", &mapnik::pdf_renderer_layout::set_map_grid_linewidth, "Set the map grid line width")
        .def ("set_scale_bar", &mapnik::pdf_renderer_layout::set_scale_bar, "Enable/Disable the scale bar")
        .def ("set_scale_bar_area", &mapnik::pdf_renderer_layout::set_scale_bar_area, "Set the scale bar size and location")
        .def ("set_scale_bar_factor", &mapnik::pdf_renderer_layout::set_scale_bar_factor, "Sets the scale bar scale factor and label")
        ;
} 



void render_to_pdf_1(const mapnik::Map& map, const mapnik::pdf_renderer_layout &layout, const std::string& filename) {
  mapnik::render_to_pdf(map, layout, filename);
}

void render_to_pdf_2(const mapnik::Map& map, const mapnik::pdf_renderer_layout &layout, const std::string& filename, const bool compress) {
  mapnik::render_to_pdf(map, layout, filename, compress);
}




void export_pdf_renderer()
{

    using namespace boost::python;
    using mapnik::pdf_renderer;

    def("render_to_pdf",&render_to_pdf_1);
    def("render_to_pdf",&render_to_pdf_2);

}
