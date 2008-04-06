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
#include <pdf/pdf_polygon_pattern_symbolizer.hpp>

void export_pdf_polygon_pattern_symbolizer()
{
    using namespace boost::python;
    using mapnik::pdf_polygon_pattern_symbolizer;
    
    class_<pdf_polygon_pattern_symbolizer, bases<mapnik::polygon_pattern_symbolizer>>("PDFPolygonPatternSymbolizer", init<std::string const&, std::string const&, const unsigned, const unsigned, const double, const double>("Create a new PDF Polygon Pattern Symbolizer"))
        .def (init<mapnik::pdf_polygon_pattern_symbolizer const&>("Copies a PDF Polygon Pattern Symbolizer"))
        .def (init<mapnik::polygon_pattern_symbolizer const&, const double, const double>("Creates a PDF Polygon Pattern Symbolizer from a Polygon Pattern Symbolizer"))
        .def ("set_output_size", &mapnik::pdf_polygon_pattern_symbolizer::set_output_size, "Sets the output size of the tile in PDF units")
        .def ("get_output_width", &mapnik::pdf_polygon_pattern_symbolizer::get_output_width, "Gets the output width of the tile in PDF units")
        .def ("get_output_height", &mapnik::pdf_polygon_pattern_symbolizer::get_output_height, "Gets the output height of the tile in PDF units")
        ;
}
