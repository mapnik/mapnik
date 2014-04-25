/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#ifndef MAPNIK_EVALUATE_GLOBAL_ATTRIBUTES_HPP
#define MAPNIK_EVALUATE_GLOBAL_ATTRIBUTES_HPP

#include <mapnik/map.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/symbolizer_hash.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/color_factory.hpp>

// boost
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/regex.hpp>
#if defined(BOOST_REGEX_HAS_ICU)
#include <boost/regex/icu.hpp>
#endif

namespace mapnik {

namespace {

template <typename T, typename Attributes>
struct evaluate_expression : boost::static_visitor<T>
{
    typedef T value_type;

    explicit evaluate_expression(Attributes const& attributes)
        : attributes_(attributes) {}

    value_type operator() (attribute const& attr) const
    {
        throw std::runtime_error("can't evaluate feature attributes in this context");
    }

    value_type operator() (global_attribute const& attr) const
    {
        auto itr = attributes_.find(attr.name);
        if (itr != attributes_.end())
        {
            return itr->second;
        }
        return value_type();// throw?
    }

    value_type operator() (geometry_type_attribute const& geom) const
    {
        throw std::runtime_error("can't evaluate geometry_type attributes in this context");
    }

    value_type operator() (binary_node<tags::logical_and> const & x) const
    {
        return (boost::apply_visitor(*this, x.left).to_bool())
            && (boost::apply_visitor(*this, x.right).to_bool());
    }

    value_type operator() (binary_node<tags::logical_or> const & x) const
    {
        return (boost::apply_visitor(*this,x.left).to_bool())
            || (boost::apply_visitor(*this,x.right).to_bool());
    }

    template <typename Tag>
    value_type operator() (binary_node<Tag> const& x) const
    {
        typename make_op<Tag>::type operation;
        return operation(boost::apply_visitor(*this, x.left),
                         boost::apply_visitor(*this, x.right));
    }

    template <typename Tag>
    value_type operator() (unary_node<Tag> const& x) const
    {
        typename make_op<Tag>::type func;
        return func(boost::apply_visitor(*this, x.expr));
    }

    value_type operator() (unary_node<tags::logical_not> const& x) const
    {
        return ! (boost::apply_visitor(*this,x.expr).to_bool());
    }

    value_type operator() (regex_match_node const& x) const
    {
        value_type v = boost::apply_visitor(*this, x.expr);
#if defined(BOOST_REGEX_HAS_ICU)
        return boost::u32regex_match(v.to_unicode(),x.pattern);
#else
        return boost::regex_match(v.to_string(),x.pattern);
#endif

    }

    value_type operator() (regex_replace_node const& x) const
    {
        value_type v = boost::apply_visitor(*this, x.expr);
#if defined(BOOST_REGEX_HAS_ICU)
        return boost::u32regex_replace(v.to_unicode(),x.pattern,x.format);
#else
        std::string repl = boost::regex_replace(v.to_string(),x.pattern,x.format);
        mapnik::transcoder tr_("utf8");
        return tr_.transcode(repl.c_str());
#endif
    }

    template <typename ValueType>
    value_type operator() (ValueType const& val) const
    {
        return value_type(val);
    }

    Attributes const& attributes_;
};

template <typename T, typename Attributes>
struct assign_value : boost::static_visitor<> {};

template <typename Attributes>
struct assign_value<expression_ptr,Attributes> : boost::static_visitor<>
{
    assign_value(symbolizer_base::value_type & val, expression_ptr const& expr, Attributes const& attributes)
        : val_(val),
          expr_(expr),
          attributes_(attributes) {}

    void operator() (color const& default_val) const
    {
        // evaluate expression as a string then parse as css color
        std::string str = boost::apply_visitor(mapnik::evaluate_expression<mapnik::value,
                                               Attributes>(attributes_),*expr_).to_string();
        try { val_ = parse_color(str); }
        catch (...) { val_ = default_val;}
    }

    void operator() (value_double default_val) const
    {
        val_ = boost::apply_visitor(mapnik::evaluate_expression<mapnik::value, Attributes>(attributes_),*expr_).to_double();
    }

    void operator() (value_integer default_val) const
    {
        val_ = boost::apply_visitor(mapnik::evaluate_expression<mapnik::value, Attributes>(attributes_),*expr_).to_int();
    }

    void operator() (value_bool default_val) const
    {
        val_ = boost::apply_visitor(mapnik::evaluate_expression<mapnik::value, Attributes>(attributes_),*expr_).to_bool();
    }

    template <typename T>
    void operator() (T const& default_val) const
    {
        // no-op
    }
    symbolizer_base::value_type & val_;
    expression_ptr const& expr_;
    Attributes const& attributes_;
};

}
struct evaluate_global_attributes : mapnik::noncopyable
{
    template <typename Attributes>
    struct evaluator : boost::static_visitor<>
    {
        evaluator(symbolizer_base::cont_type::value_type & prop, Attributes const& attributes)
            : prop_(prop),
              attributes_(attributes) {}

        void operator() (expression_ptr const& expr) const
        {
            auto const& meta = get_meta(prop_.first);
            boost::apply_visitor(assign_value<expression_ptr,Attributes>(prop_.second, expr, attributes_), std::get<1>(meta));
        }

        template <typename T>
        void operator() (T const& val) const
        {
            // no-op
        }
        symbolizer_base::cont_type::value_type & prop_;
        Attributes const& attributes_;
    };

    template <typename Attributes>
    struct extract_symbolizer : boost::static_visitor<>
    {
        extract_symbolizer(Attributes const& attributes)
            : attributes_(attributes) {}

        template <typename Symbolizer>
        void operator() (Symbolizer & sym) const
        {
            for (auto & prop : sym.properties)
            {
                boost::apply_visitor(evaluator<Attributes>(prop, attributes_), prop.second);
            }
        }
        Attributes const& attributes_;
    };

    template <typename Attributes>
    static void apply(Map & m, Attributes const& attributes)
    {
        for ( auto & val :  m.styles() )
        {
            for (auto & rule : val.second.get_rules_nonconst())
            {
                for (auto & sym : rule)
                {
                    boost::apply_visitor(extract_symbolizer<Attributes>(attributes), sym);
                }
            }
        }
    }
};

}

#endif // MAPNIK_EVALUATE_GLOBAL_ATTRIBUTES_HPP
