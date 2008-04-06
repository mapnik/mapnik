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
#include <pdf/pdf_point_symbolizer.hpp>

void export_pdf_point_symbolizer()
{
    using namespace boost::python;
    using mapnik::pdf_point_symbolizer;
    
    class_<pdf_point_symbolizer, bases<mapnik::point_symbolizer>>("PDFPointSymbolizer", init<>("Default PDF Point Symbolizer - 1x1 black square"))
        .def (init<std::string const&, std::string const&, unsigned, unsigned, double, double>("Create a new PDF Point Symbolizer"))
        .def (init<mapnik::pdf_point_symbolizer &>("Copies a PDF Point Symbolizer"))
        .def (init<mapnik::point_symbolizer &, double, double>("Creates a PDF point symbolizer from a point symbolizer"))
        .def ("set_output_size", &mapnik::pdf_point_symbolizer::set_output_size, "Sets the output size of the symbol in PDF units")
        .def ("get_output_width", &mapnik::pdf_point_symbolizer::get_output_width, "Gets the output width of the symbol in PDF units")
        .def ("get_output_height", &mapnik::pdf_point_symbolizer::get_output_height, "Gets the output height of the symbol in PDF units")
        ;
} 
