/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_SVG_PARSER_HPP
#define MAPNIK_SVG_PARSER_HPP

// mapnik
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/gradient.hpp>

// boost
#include <boost/utility.hpp>
#include <libxml/xmlreader.h>

// stl
#include <map>

namespace  mapnik { namespace svg {

    class svg_parser : private boost::noncopyable
    {
    public:
        explicit svg_parser(svg_converter_type & path);
        ~svg_parser();
        void parse(std::string const& filename);
        void parse_from_string(std::string const& svg);
    private:
        bool parse_reader(xmlTextReaderPtr reader);
        void process_node(xmlTextReaderPtr reader);
        void start_element(xmlTextReaderPtr reader);
        void end_element(xmlTextReaderPtr reader);
        void parse_path(xmlTextReaderPtr reader);
        void parse_polygon(xmlTextReaderPtr reader);
        void parse_polyline(xmlTextReaderPtr reader);
        void parse_line(xmlTextReaderPtr reader);
        void parse_rect(xmlTextReaderPtr reader);
        void parse_circle(xmlTextReaderPtr reader);
        void parse_ellipse(xmlTextReaderPtr reader);
        void parse_linear_gradient(xmlTextReaderPtr reader);
        void parse_radial_gradient(xmlTextReaderPtr reader);
        bool parse_common_gradient(xmlTextReaderPtr reader);
        void parse_gradient_stop(xmlTextReaderPtr reader);
        void parse_pattern(xmlTextReaderPtr reader);
        void parse_attr(xmlTextReaderPtr reader);
        void parse_attr(const xmlChar * name, const xmlChar * value );
    private:
        svg_converter_type & path_;
        bool is_defs_;
        std::map<std::string, gradient> gradient_map_;
        std::pair<std::string, gradient> temporary_gradient_;
    };

    }}


#endif // MAPNIK_SVG_PARSER_HPP
