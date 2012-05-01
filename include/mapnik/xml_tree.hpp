/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef MAPNIK_XML_TREE_H
#define MAPNIK_XML_TREE_H
//mapnik
#include <mapnik/xml_node.hpp>
#include <mapnik/expression_grammar.hpp>
#include <mapnik/path_expression_grammar.hpp>

// boost
#include <boost/format.hpp>

#if BOOST_VERSION >= 104500
#include <mapnik/css_color_grammar.hpp>
#else
#include <mapnik/css_color_grammar_deprecated.hpp>
#endif

//stl
#include <string>


namespace mapnik
{
class xml_tree
{
public:
    xml_tree(std::string const& encoding="utf8");
    void set_filename(std::string fn);
    std::string const& filename() const;
    xml_node &root();
private:
    xml_node node_;
    std::string file_;
    transcoder tr_;
public:
    mapnik::css_color_grammar<std::string::const_iterator> color_grammar;
    mapnik::expression_grammar<std::string::const_iterator> expr_grammar;
    path_expression_grammar<std::string::const_iterator> path_expr_grammar;
};

} //ns mapnik

#endif // MAPNIK_XML_TREE_H
