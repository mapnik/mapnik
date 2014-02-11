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

// boost
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>

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

template <typename Container>
struct extract_attribute_names : boost::static_visitor<void>
{
    expression_attributes<std::set<std::string> > f_attr;

    explicit extract_attribute_names(Container& names)
        : names_(names),
          f_attr(names) {}

    void operator() (mapnik::expression_ptr const& expr) const
    {
        if (expr)
        {
            boost::apply_visitor(f_attr, *expr);
        }
    }
    void operator() (mapnik::transform_type const& expr) const
    {
        if (expr)
        {
            transform_processor_type::collect_attributes(names_, *expr);
        }
    }

    void operator() (mapnik::text_placements_ptr const& expr) const
    {
        if (expr)
        {
            expression_set::const_iterator it;
            expression_set expressions;
            // TODO - optimize (dane)
            expr->add_expressions(expressions);
            for (it=expressions.begin(); it != expressions.end(); ++it)
            {
                if (*it) boost::apply_visitor(f_attr, **it);
            }
        }
    }

    void operator() (mapnik::path_expression_ptr const& expr) const
    {
        if (expr)
        {
            path_processor_type::collect_attributes(*expr,names_);
        }
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
          f_attrs(names) {}

    template <typename T>
    void operator () (T const& sym)
    {
        for (auto const& prop : sym.properties)
        {
            boost::apply_visitor(f_attrs, prop.second);
        }
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
        for (auto const& prop : sym.properties)
        {
            boost::apply_visitor(f_attrs, prop.second);
        }
    }

private:
    std::set<std::string>& names_;
    double & filter_factor_;
    extract_attribute_names<std::set<std::string> > f_attrs;
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

} // namespace mapnik

#endif // MAPNIK_ATTRIBUTE_COLLECTOR_HPP
