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
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/path_expression_grammar.hpp>
#include <mapnik/parse_path.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/variant.hpp>
#include <boost/concept_check.hpp>

// stl
#include <set>
#include <iostream>

namespace mapnik {

struct expression_attributes : boost::static_visitor<void>
{
    explicit expression_attributes(std::set<std::string> & names)
        : names_(names) {}
    
    void operator() (value_type const& x) const 
    {
        boost::ignore_unused_variable_warning(x);
    }
    
    void operator() (attribute const& attr) const
    {
        names_.insert(attr.name());
    }
    
    template <typename Tag> 
    void operator() (binary_node<Tag> const& x) const
    {
        boost::apply_visitor(expression_attributes(names_),x.left);
        boost::apply_visitor(expression_attributes(names_),x.right);
        
    }

    template <typename Tag>
    void operator() (unary_node<Tag> const& x) const
    {
        boost::apply_visitor(expression_attributes(names_),x.expr);     
    }
    
    void operator() (regex_match_node const& x) const
    {
        boost::apply_visitor(expression_attributes(names_),x.expr);
    }
    
    void operator() (regex_replace_node const& x) const
    {
        boost::apply_visitor(expression_attributes(names_),x.expr);
    }
    
private:
    std::set<std::string>& names_;
};

struct symbolizer_attributes : public boost::static_visitor<>
{
    symbolizer_attributes(std::set<std::string>& names)
        : names_(names) {}
        
    template <typename T>
    void operator () (T const&) const {}
    
    void operator () (text_symbolizer const& sym)
    {
        expression_ptr const& name_expr = sym.get_name();
        if (name_expr)
        {
            expression_attributes f_attr(names_);
            boost::apply_visitor(f_attr,*name_expr);
        }

        expression_ptr const& orientation_expr = sym.get_orientation();
        if (orientation_expr)
        {
            expression_attributes f_attr(names_);
            boost::apply_visitor(f_attr,*orientation_expr);
        }
        collect_metawriter(sym);
    }
    
    void operator () (point_symbolizer const& sym)
    {   
        path_expression_ptr const& filename_expr = sym.get_filename();
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }
        collect_metawriter(sym);

    }

    void operator () (line_symbolizer const& sym)
    {
        collect_metawriter(sym);
    }

    void operator () (line_pattern_symbolizer const& sym)
    {   
        path_expression_ptr const& filename_expr = sym.get_filename();
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }
        collect_metawriter(sym);
    }

    void operator () (polygon_symbolizer const& sym)
    {
        collect_metawriter(sym);
    }

    void operator () (polygon_pattern_symbolizer const& sym)
    {   
        path_expression_ptr const& filename_expr = sym.get_filename();
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }
        collect_metawriter(sym);
    }
    
    void operator () (shield_symbolizer const& sym)
    {
        expression_ptr const& name_expr = sym.get_name();
        if (name_expr)
        {
            expression_attributes name_attr(names_);
            boost::apply_visitor(name_attr,*name_expr);
        }
        
        path_expression_ptr const& filename_expr = sym.get_filename();
        if (filename_expr)
        {
            path_processor_type::collect_attributes(*filename_expr,names_);
        }
        collect_metawriter(sym);
    }

    void operator () (glyph_symbolizer const& sym)
    {
        expression_ptr const& char_expr = sym.get_char();
        if (char_expr)
        {
            expression_attributes f_attr(names_);
            boost::apply_visitor(f_attr,*char_expr);
        }

        expression_ptr const& angle_expr = sym.get_angle();
        if (angle_expr)
        {
            expression_attributes f_attr(names_);
            boost::apply_visitor(f_attr,*angle_expr);
        }

        expression_ptr const& value_expr = sym.get_value();
        if (value_expr)
        {
            expression_attributes f_attr(names_);
            boost::apply_visitor(f_attr,*value_expr);
        }

        expression_ptr const& size_expr = sym.get_size();
        if (size_expr)
        {
            expression_attributes f_attr(names_);
            boost::apply_visitor(f_attr,*size_expr);
        }

        expression_ptr const& color_expr = sym.get_color();
        if (color_expr)
        {
            expression_attributes f_attr(names_);
            boost::apply_visitor(f_attr,*color_expr);
        }
        collect_metawriter(sym);
    }

    void operator () (markers_symbolizer const& sym)
    {
        collect_metawriter(sym);
    }

    void operator () (building_symbolizer const& sym)
    {
    	expression_ptr const& height_expr = sym.height();
		if (height_expr)
		{
			expression_attributes f_attr(names_);
			boost::apply_visitor(f_attr,*height_expr);
		}
        collect_metawriter(sym);
    }
    // TODO - support remaining syms
    
private:
    std::set<std::string>& names_;
    void collect_metawriter(symbolizer_base const& sym)
    {
        metawriter_properties const& properties = sym.get_metawriter_properties();
        names_.insert(properties.begin(), properties.end());
    }
};


class attribute_collector : public boost::noncopyable
{
private:
    std::set<std::string>& names_;
public:
    
    attribute_collector(std::set<std::string>& names)
        : names_(names) {}
        
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
        expression_attributes f_attr(names_);
        boost::apply_visitor(f_attr,*expr);
    }   
};

struct directive_collector : public boost::static_visitor<>
{
    directive_collector(double * filter_factor)
        : filter_factor_(filter_factor) {}
        
    template <typename T>
    void operator () (T const&) const {}
    
    void operator () (raster_symbolizer const& sym)
    {
        *filter_factor_ = sym.calculate_filter_factor();
    }
private:
    double * filter_factor_;
};

} // namespace mapnik

#endif // MAPNIK_ATTRIBUTE_COLLECTOR_HPP
