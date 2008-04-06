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
#include <pdf/pdf_line_pattern_symbolizer.hpp>
#include <mapnik/line_symbolizer.hpp>

using mapnik::pdf_line_pattern_symbolizer;
using mapnik::stroke;

void export_pdf_line_pattern_symbolizer()
{
    using namespace boost::python;
    
    class_<pdf_line_pattern_symbolizer, bases<mapnik::line_pattern_symbolizer>>("PDFLinePatternSymbolizer", init<std::string const&, std::string const&, const double, const unsigned, const unsigned, const double, const double>("Create a new PDF Line Pattern Symbolizer"))
        .def (init<mapnik::pdf_line_pattern_symbolizer const&>("Copies a PDF Line Pattern Symbolizer"))
        .def (init<mapnik::line_pattern_symbolizer const&, const double, const double, const double>("Creates a PDF Line Pattern Symbolizer from a Line Pattern Symbolizer"))
        .def ("set_output_size", &mapnik::pdf_line_pattern_symbolizer::set_output_size, "Sets the output size of the tile in PDF units")
        .def ("get_output_width", &mapnik::pdf_line_pattern_symbolizer::get_output_width, "Gets the output width of the tile in PDF units")
        .def ("get_output_height", &mapnik::pdf_line_pattern_symbolizer::get_output_height, "Gets the output height of the tile in PDF units")
        .add_property("stroke",make_function( &pdf_line_pattern_symbolizer::get_stroke, return_value_policy<copy_const_reference>() ), &pdf_line_pattern_symbolizer::set_stroke )
        ;
}
