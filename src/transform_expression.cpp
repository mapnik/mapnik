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

// mapnik
#include <mapnik/expression_string.hpp>
#include <mapnik/transform/transform_expression.hpp>

// stl
#include <sstream>

namespace mapnik {

struct transform_node_to_expression_string

{
    std::ostringstream& os_;

    transform_node_to_expression_string(std::ostringstream& os)
        : os_(os)
    {}

    void operator()(identity_node const&) const {}

    void operator()(matrix_node const& node)
    {
        os_ << "matrix(" << to_expression_string(node.a_) << ", " << to_expression_string(node.b_) << ", "
            << to_expression_string(node.c_) << ", " << to_expression_string(node.d_) << ", "
            << to_expression_string(node.e_) << ", " << to_expression_string(node.f_) << ")";
    }

    void operator()(translate_node const& node)
    {
        if (detail::is_null_node(node.ty_))
        {
            os_ << "translate(" << to_expression_string(node.tx_) << ")";
        }
        else
        {
            os_ << "translate(" << to_expression_string(node.tx_) << ", " << to_expression_string(node.ty_) << ")";
        }
    }

    void operator()(scale_node const& node)
    {
        if (detail::is_null_node(node.sy_))
        {
            os_ << "scale(" << to_expression_string(node.sx_) << ")";
        }
        else
        {
            os_ << "scale(" << to_expression_string(node.sx_) << ", " << to_expression_string(node.sy_) << ")";
        }
    }

    void operator()(rotate_node const& node)
    {
        if (detail::is_null_node(node.cy_) || detail::is_null_node(node.cy_))
        {
            os_ << "rotate(" << to_expression_string(node.angle_) << ")";
        }
        else
        {
            os_ << "rotate(" << to_expression_string(node.angle_) << ", " << to_expression_string(node.cx_) << ", "
                << to_expression_string(node.cy_) << ")";
        }
    }

    void operator()(skewX_node const& node) { os_ << "skewX(" << to_expression_string(node.angle_) << ")"; }

    void operator()(skewY_node const& node) { os_ << "skewY(" << to_expression_string(node.angle_) << ")"; }
};

std::string to_expression_string(transform_node const& node)
{
    std::ostringstream os; // FIXME set precision?
    transform_node_to_expression_string to_string(os);
    util::apply_visitor(to_string, *node);
    return os.str();
}

std::string to_expression_string(transform_list const& list)
{
    std::ostringstream os; // FIXME set precision?
    std::streamsize first = 1;
    transform_node_to_expression_string to_string(os);

    for (transform_node const& node : list)
    {
        os.write(" ", first ? (first = 0) : 1);
        util::apply_visitor(to_string, *node);
    }
    return os.str();
}

} // namespace mapnik
