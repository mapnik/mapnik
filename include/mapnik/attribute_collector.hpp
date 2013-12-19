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
#include <mapnik/transform_processor.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/rule.hpp> // for rule::symbolizers
#include <mapnik/expression.hpp>  // for expression_ptr, etc
#include <mapnik/expression_node_types.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/parse_path.hpp>  // for path_processor_type
#include <mapnik/path_expression.hpp>  // for path_expression_ptr
#include <mapnik/text/placements/base.hpp>  // for text_placements
#include <mapnik/image_scaling.hpp>
#include <mapnik/group/group_symbolizer_properties.hpp>
#include <mapnik/group/group_rule.hpp>

// boost
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

// stl
#include <set>

namespace mapnik {

template <typename Container>
struct expression_attributes : boost::static_visitor<void>
{
    explicit expression_attributes(Container& names)
        : names_(names) {}

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

    template <typename T>
    void operator() (T const& val) const {}

private:
    Container& names_;
};

struct symbolizer_attributes : public boost::static_visitor<>
{
    symbolizer_attributes(std::set<std::string>& names,
                          double & filter_factor)
        : names_(names),
          filter_factor_(filter_factor),
          f_attr(names) {}

    template <typename T>
    void operator () (T const&) const {}

    void operator () (text_symbolizer const& sym)
    {
        expression_set::const_iterator it;
        expression_set expressions;
        get<mapnik::text_placements_ptr>(sym, keys::text_placements_)->add_expressions(expressions);
        for (it=expressions.begin(); it != expressions.end(); ++it)
        {
            if (*it) boost::apply_visitor(f_attr, **it);
        }
        collect_transform(get<mapnik::transform_type>(sym, keys::geometry_transform));
    }

    void operator () (point_symbolizer const& sym)
    {
        path_expression_ptr const& filename_expr = get<mapnik::path_expression_ptr>(sym, keys::file);
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }
        collect_transform(get<mapnik::transform_type>(sym, keys::geometry_transform));
        collect_transform(get<mapnik::transform_type>(sym, keys::image_transform));
    }

    void operator () (line_symbolizer const& sym)
    {
        collect_transform(get<mapnik::transform_type>(sym, keys::geometry_transform));
    }

    void operator () (line_pattern_symbolizer const& sym)
    {
        path_expression_ptr const& filename_expr = get<mapnik::path_expression_ptr>(sym, keys::file);
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }
        collect_transform(get<mapnik::transform_type>(sym, keys::geometry_transform));
        collect_transform(get<mapnik::transform_type>(sym, keys::image_transform));
    }

    void operator () (polygon_symbolizer const& sym)
    {
        collect_transform(get<mapnik::transform_type>(sym, keys::geometry_transform));
    }

    void operator () (polygon_pattern_symbolizer const& sym)
    {
        path_expression_ptr const& filename_expr = get<mapnik::path_expression_ptr>(sym, keys::file);
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }
        collect_transform(get<mapnik::transform_type>(sym, keys::geometry_transform));
        collect_transform(get<mapnik::transform_type>(sym, keys::image_transform));
    }

    void operator () (shield_symbolizer const& sym)
    {
        expression_set::const_iterator it;
        expression_set expressions;
        get<mapnik::text_placements_ptr>(sym, keys::text_placements_)->add_expressions(expressions);
        for (it=expressions.begin(); it != expressions.end(); ++it)
        {
            if (*it) boost::apply_visitor(f_attr, **it);
        }

        path_expression_ptr const& filename_expr = get<mapnik::path_expression_ptr>(sym, keys::file);
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }

        collect_transform(get<mapnik::transform_type>(sym, keys::geometry_transform));
        collect_transform(get<mapnik::transform_type>(sym, keys::image_transform));
    }

    void operator () (markers_symbolizer const& sym)
    {
        expression_ptr const& height_expr = get<mapnik::expression_ptr>(sym,keys::height);
        if (height_expr)
        {
            boost::apply_visitor(f_attr,*height_expr);
        }
        expression_ptr const& width_expr = get<mapnik::expression_ptr>(sym,keys::width);
        if (width_expr)
        {
            boost::apply_visitor(f_attr,*width_expr);
        }
        path_expression_ptr const& filename_expr = get<mapnik::path_expression_ptr>(sym, keys::file);
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }
        collect_transform(get<mapnik::transform_type>(sym, keys::geometry_transform));
        collect_transform(get<mapnik::transform_type>(sym, keys::image_transform));
    }

    void operator () (building_symbolizer const& sym)
    {
        expression_ptr const& height_expr = get<mapnik::expression_ptr>(sym,keys::height);
        if (height_expr)
        {
            boost::apply_visitor(f_attr,*height_expr);
        }
        collect_transform(get<mapnik::transform_type>(sym, keys::geometry_transform));
    }

    void operator () (raster_symbolizer const& sym)
    {
        boost::optional<double> filter_factor = get_optional<double>(sym, keys::filter_factor);
        if (filter_factor)
        {
            filter_factor_ = *filter_factor;
        }
        else
        {
            boost::optional<scaling_method_e> scaling_method = get_optional<scaling_method_e>(sym, keys::scaling);
            if (scaling_method && *scaling_method != SCALING_NEAR)
            {
                filter_factor_ = 2;
            }
        }
    }

    void operator () (group_symbolizer const& sym);

private:
    std::set<std::string>& names_;
    double & filter_factor_;
    expression_attributes<std::set<std::string> > f_attr;
    void collect_transform(transform_list_ptr const& trans_expr)
    {
        if (trans_expr)
        {
            transform_processor_type::collect_attributes(names_, *trans_expr);
        }
    }
};


class attribute_collector : public mapnik::noncopyable
{
private:
    std::set<std::string>& names_;
    double filter_factor_;
    expression_attributes<std::set<std::string> > f_attr;
public:

    attribute_collector(std::set<std::string>& names)
        : names_(names),
          filter_factor_(1.0),
          f_attr(names) {}
    template <typename RuleType>
    void operator() (RuleType const& r)
    {
        typename RuleType::symbolizers const& symbols = r.get_symbolizers();
        typename RuleType::symbolizers::const_iterator symIter=symbols.begin();
        symbolizer_attributes s_attr(names_,filter_factor_);
        while (symIter != symbols.end())
        {
            boost::apply_visitor(s_attr,*symIter++);
        }

        expression_ptr const& expr = r.get_filter();
        boost::apply_visitor(f_attr,*expr);
    }

    double get_filter_factor() const
    {
        return filter_factor_;
    }
};

inline void symbolizer_attributes::operator () (group_symbolizer const& sym)
{
    // find all column names referenced in the group symbolizer
    std::set<std::string> group_columns;
    attribute_collector column_collector(group_columns);
    expression_attributes<std::set<std::string> > rk_attr(group_columns);

    // get columns from symbolizer repeat key
    expression_ptr repeat_key = get<mapnik::expression_ptr>(sym, keys::repeat_key);
    if (repeat_key)
    {
        boost::apply_visitor(rk_attr, *repeat_key);
    }

    // get columns from child rules and symbolizers
    group_symbolizer_properties_ptr props = get<group_symbolizer_properties_ptr>(sym, keys::group_properties);
    if (props) {
        for (auto const& rule : props->get_rules())
        {
            column_collector(*rule);
            if (rule->get_repeat_key())
            {
                boost::apply_visitor(rk_attr, *(rule->get_repeat_key()));
            }
        }
    }

    // get indexed column names
    for (auto const& col_name : group_columns)
    {
        if (col_name.find('%') != std::string::npos)
        {
            // Note: ignore column name if it is '%' by itself.
            // '%' is a special case to access the index value itself,
            // rather than acessing indexed columns from data source.
            if (col_name.size() > 1)
            {
                // indexed column name. add column name for each index value.
                int start = get<value_integer>(sym, keys::start_column);
                int end = start + get<value_integer>(sym, keys::num_columns);
                for (int col_idx = start; col_idx < end; ++col_idx)
                {
                    std::string col_idx_name = col_name;
                    boost::replace_all(col_idx_name, "%", boost::lexical_cast<std::string>(col_idx));
                    names_.insert(col_idx_name);
                }
            }
        }
        else
        {
            // non indexed column name. insert as is.
            names_.insert(col_name);
        }
    }
}

} // namespace mapnik

#endif // MAPNIK_ATTRIBUTE_COLLECTOR_HPP
