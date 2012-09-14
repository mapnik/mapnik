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

#ifndef MAPNIK_ATTRIBUTE_COLLECTOR_HPP
#define MAPNIK_ATTRIBUTE_COLLECTOR_HPP

// mapnik
#include <mapnik/rule.hpp>
#include <mapnik/transform_processor.hpp>
// boost
#include <boost/utility.hpp>
#include <boost/variant.hpp>
#include <boost/concept_check.hpp>
// stl
#include <set>
#include <iostream>

namespace mapnik {

template <typename Container>
struct expression_attributes : boost::static_visitor<void>
{
    explicit expression_attributes(Container& names)
        : names_(names) {}

    void operator() (value_type const& x) const
    {
        boost::ignore_unused_variable_warning(x);
    }

    void operator() (geometry_type_attribute const& type) const
    {
        // do nothing
    }

    void operator() (attribute const& attr) const
    {
        names_.insert(attr.name());
    }

    template <typename Tag>
    void operator() (binary_node<Tag> const& x) const
    {
        boost::apply_visitor(*this, x.left);
        boost::apply_visitor(*this, x.right);

    }

    template <typename Tag>
    void operator() (unary_node<Tag> const& x) const
    {
        boost::apply_visitor(*this, x.expr);
    }

    void operator() (regex_match_node const& x) const
    {
        boost::apply_visitor(*this, x.expr);
    }

    void operator() (regex_replace_node const& x) const
    {
        boost::apply_visitor(*this, x.expr);
    }

private:
    Container& names_;
};

struct symbolizer_attributes : public boost::static_visitor<>
{
    symbolizer_attributes(std::set<std::string>& names)
        : names_(names), f_attr(names) {}

    template <typename T>
    void operator () (T const&) const {}

    void operator () (text_symbolizer const& sym)
    {
        expression_set::const_iterator it;
        expression_set expressions;
        sym.get_placement_options()->add_expressions(expressions);
        for (it=expressions.begin(); it != expressions.end(); it++)
        {
            if (*it) boost::apply_visitor(f_attr, **it);
        }
        collect_transform(sym.get_transform());
    }

    void operator () (point_symbolizer const& sym)
    {
        path_expression_ptr const& filename_expr = sym.get_filename();
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }
        collect_transform(sym.get_image_transform());
        collect_transform(sym.get_transform());
    }

    void operator () (line_symbolizer const& sym)
    {
        collect_transform(sym.get_transform());
    }

    void operator () (line_pattern_symbolizer const& sym)
    {
        path_expression_ptr const& filename_expr = sym.get_filename();
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }
        collect_transform(sym.get_image_transform());
        collect_transform(sym.get_transform());
    }

    void operator () (polygon_symbolizer const& sym)
    {
        collect_transform(sym.get_transform());
    }

    void operator () (polygon_pattern_symbolizer const& sym)
    {
        path_expression_ptr const& filename_expr = sym.get_filename();
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }
        collect_transform(sym.get_image_transform());
        collect_transform(sym.get_transform());
    }

    void operator () (shield_symbolizer const& sym)
    {
        expression_set::const_iterator it;
        expression_set expressions;
        sym.get_placement_options()->add_expressions(expressions);
        for (it=expressions.begin(); it != expressions.end(); it++)
        {
            if (*it) boost::apply_visitor(f_attr, **it);
        }

        path_expression_ptr const& filename_expr = sym.get_filename();
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }
        collect_transform(sym.get_image_transform());
        collect_transform(sym.get_transform());
    }

    void operator () (markers_symbolizer const& sym)
    {
        expression_ptr const& height_expr = sym.get_height();
        if (height_expr)
        {
            boost::apply_visitor(f_attr,*height_expr);
        }
        expression_ptr const& width_expr = sym.get_width();
        if (width_expr)
        {
            boost::apply_visitor(f_attr,*width_expr);
        }
        collect_transform(sym.get_image_transform());
        collect_transform(sym.get_transform());
    }

    void operator () (building_symbolizer const& sym)
    {
        expression_ptr const& height_expr = sym.height();
        if (height_expr)
        {
            boost::apply_visitor(f_attr,*height_expr);
        }
        collect_transform(sym.get_transform());
    }
    // TODO - support remaining syms

private:
    std::set<std::string>& names_;
    expression_attributes<std::set<std::string> > f_attr;
    void collect_transform(transform_list_ptr const& trans_expr)
    {
        if (trans_expr)
        {
            transform_processor_type::collect_attributes(names_, *trans_expr);
        }
    }
};


class attribute_collector : public boost::noncopyable
{
private:
    std::set<std::string>& names_;
    expression_attributes<std::set<std::string> > f_attr;
public:

    attribute_collector(std::set<std::string>& names)
        : names_(names), f_attr(names) {}

    template <typename RuleType>
    void operator() (RuleType const& r)
    {
        typename RuleType::symbolizers const& symbols = r.get_symbolizers();
        typename RuleType::symbolizers::const_iterator symIter=symbols.begin();
        symbolizer_attributes s_attr(names_);
        while (symIter != symbols.end())
        {
            boost::apply_visitor(s_attr,*symIter++);
        }

        expression_ptr const& expr = r.get_filter();
        boost::apply_visitor(f_attr,*expr);
    }
};

struct directive_collector : public boost::static_visitor<>
{
    directive_collector(double & filter_factor)
        : filter_factor_(filter_factor) {}

    template <typename T>
    void operator () (T const&) const {}

    void operator () (raster_symbolizer const& sym)
    {
        filter_factor_ = sym.calculate_filter_factor();
    }
private:
    double & filter_factor_;
};

} // namespace mapnik

#endif // MAPNIK_ATTRIBUTE_COLLECTOR_HPP
