/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

// mapnik
#include <mapnik/cairo_io.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/map.hpp>

#ifdef HAVE_CAIRO
#include <mapnik/cairo/cairo_renderer.hpp>
#include <cairo.h>
#ifdef CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif
#ifdef CAIRO_HAS_PS_SURFACE
#include <cairo-ps.h>
#endif
#ifdef CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>
#endif
#endif

// stl
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace mapnik {

#if defined(HAVE_CAIRO)
void save_to_cairo_file(mapnik::Map const& map,
                        std::string const& filename,
                        double scale_factor,
                        double scale_denominator)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        save_to_cairo_file(map, filename, *type, scale_factor, scale_denominator);
    }
    else
        throw image_writer_exception("Could not write file to " + filename);
}

void save_to_cairo_file(mapnik::Map const& map,
                        std::string const& filename,
                        std::string const& type,
                        double scale_factor,
                        double scale_denominator)
{
    std::ofstream file(filename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    if (file)
    {
        cairo_surface_ptr surface;
        unsigned width = map.width();
        unsigned height = map.height();
        if (type == "pdf")
        {
#ifdef CAIRO_HAS_PDF_SURFACE
            surface =
              cairo_surface_ptr(cairo_pdf_surface_create(filename.c_str(), width, height), cairo_surface_closer());
#else
            throw image_writer_exception("PDFSurface not supported in the cairo backend");
#endif
        }
#ifdef CAIRO_HAS_SVG_SURFACE
        else if (type == "svg")
        {
            surface =
              cairo_surface_ptr(cairo_svg_surface_create(filename.c_str(), width, height), cairo_surface_closer());
            cairo_svg_surface_restrict_to_version(surface.get(), CAIRO_SVG_VERSION_1_2);
        }
#endif
#ifdef CAIRO_HAS_PS_SURFACE
        else if (type == "ps")
        {
            surface =
              cairo_surface_ptr(cairo_ps_surface_create(filename.c_str(), width, height), cairo_surface_closer());
        }
#endif
#ifdef CAIRO_HAS_IMAGE_SURFACE
        else if (type == "ARGB32")
        {
            surface =
              cairo_surface_ptr(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height), cairo_surface_closer());
        }
        else if (type == "RGB24")
        {
            surface =
              cairo_surface_ptr(cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height), cairo_surface_closer());
        }
#endif
        else
        {
            throw image_writer_exception("unknown file type: " + type);
        }

        // cairo_t * ctx = cairo_create(surface);

        // TODO - expose as user option
        /*
          if (type == "ARGB32" || type == "RGB24")
          {
          context->set_antialias(Cairo::ANTIALIAS_NONE);
          }
        */

        mapnik::cairo_renderer<cairo_ptr> ren(map, create_context(surface), scale_factor);
        ren.apply(scale_denominator);

        if (type == "ARGB32" || type == "RGB24")
        {
            cairo_surface_write_to_png(&*surface, filename.c_str());
        }
        cairo_surface_finish(&*surface);
    }
}

#endif

} // namespace mapnik
