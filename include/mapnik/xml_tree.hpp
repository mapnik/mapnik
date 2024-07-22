/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_XML_TREE_HPP
#define MAPNIK_XML_TREE_HPP

// mapnik
#include <mapnik/xml_node.hpp>
#include <mapnik/expression.hpp>

// stl
#include <string>

namespace mapnik {

class MAPNIK_DECL xml_tree
{
  public:
    xml_tree();
    void set_filename(std::string const& fn);
    std::string const& filename() const;
    xml_node& root();
    xml_node const& root() const;

  private:
    xml_node node_;
    std::string file_;

  public:
    mutable std::map<std::string, mapnik::expression_ptr> expr_cache_;
};

} // namespace mapnik

#endif // MAPNIK_XML_TREE_HPP
