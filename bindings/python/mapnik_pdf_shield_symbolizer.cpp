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
#include <pdf/pdf_shield_symbolizer.hpp>

void export_pdf_shield_symbolizer()
{
    using namespace boost::python;
    using mapnik::pdf_shield_symbolizer;
    using mapnik::text_symbolizer;

    class_< pdf_shield_symbolizer, bases<mapnik::shield_symbolizer> >("PDFShieldSymbolizer", init< std::string const&, std::string const&, unsigned, mapnik::Color const&, std::string const&, std::string const&, unsigned, unsigned, double, double>("TODO"))
        .def (init<mapnik::pdf_shield_symbolizer const&>("Copies a PDF Shield Symbolizer"))
        .def (init<mapnik::pdf_shield_symbolizer const&, const double, const double>("Creates a PDF Shield Symbolizer from a Shield Symbolizer"))
        .def ("set_output_size", &mapnik::pdf_shield_symbolizer::set_output_size, "Sets the output size of the shield in PDF units")
        .def ("get_output_width", &mapnik::pdf_shield_symbolizer::get_output_width, "Gets the output width of the shield in PDF units")
        .def ("get_output_height", &mapnik::pdf_shield_symbolizer::get_output_height, "Gets the output height of the shield in PDF units")
        ;
    
}
