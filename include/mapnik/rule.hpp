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

#ifndef MAPNIK_RULE_HPP
#define MAPNIK_RULE_HPP

// mapni
#include <mapnik/building_symbolizer.hpp>
#include <mapnik/line_symbolizer.hpp>
#include <mapnik/line_pattern_symbolizer.hpp>
#include <mapnik/polygon_symbolizer.hpp>
#include <mapnik/polygon_pattern_symbolizer.hpp>
#include <mapnik/point_symbolizer.hpp>
#include <mapnik/raster_symbolizer.hpp>
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/markers_symbolizer.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_string.hpp>

// boost
#include <boost/concept_check.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/variant.hpp>

// stl
#include <string>
#include <vector>

namespace mapnik
{
inline bool operator==(point_symbolizer const& lhs,
                       point_symbolizer const& rhs)
{
    return (&lhs == &rhs);
}
inline bool operator==(line_symbolizer const& lhs,
                       line_symbolizer const& rhs)
{
    return (&lhs == &rhs);
}
inline bool operator==(line_pattern_symbolizer const& lhs,
                       line_pattern_symbolizer const& rhs)
{
    return (&lhs == &rhs);
}

inline bool operator==(polygon_symbolizer const& lhs,
                       polygon_symbolizer const& rhs)
{
    return (&lhs == &rhs);
}

inline bool operator==(polygon_pattern_symbolizer const& lhs,
                       polygon_pattern_symbolizer const& rhs)
{
    return (&lhs == &rhs);
}

inline bool operator==(raster_symbolizer const& lhs,
                       raster_symbolizer const& rhs)
{
    return (&lhs == &rhs);
}

inline bool operator==(text_symbolizer const& lhs,
                       text_symbolizer const& rhs)
{
    return (&lhs == &rhs);
}

inline bool operator==(shield_symbolizer const& lhs,
                       shield_symbolizer const& rhs)
{
    return (&lhs == &rhs);
}

inline bool operator==(building_symbolizer const& lhs,
                       building_symbolizer const& rhs)
{
    return (&lhs == &rhs);
}

inline bool operator==(markers_symbolizer const& lhs,
                       markers_symbolizer const& rhs)
{
    return (&lhs == &rhs);
}

typedef boost::variant<point_symbolizer,
                       line_symbolizer,
                       line_pattern_symbolizer,
                       polygon_symbolizer,
                       polygon_pattern_symbolizer,
                       raster_symbolizer,
                       shield_symbolizer,
                       text_symbolizer,
                       building_symbolizer,
                       markers_symbolizer> symbolizer;

class rule
{
public:
    typedef std::vector<symbolizer> symbolizers;
private:

    std::string name_;
    double min_scale_;
    double max_scale_;
    symbolizers syms_;
    expression_ptr filter_;
    bool else_filter_;
    bool also_filter_;

    struct deepcopy_symbolizer : public boost::static_visitor<>
    {

        void operator () (markers_symbolizer & sym) const
        {
            copy_path_ptr(sym);
        }

        void operator () (point_symbolizer & sym) const
        {
            copy_path_ptr(sym);
        }

        void operator () (polygon_pattern_symbolizer & sym) const
        {
            copy_path_ptr(sym);
        }

        void operator () (line_pattern_symbolizer & sym) const
        {
            copy_path_ptr(sym);
        }

        void operator () (raster_symbolizer & sym) const
        {
            raster_colorizer_ptr old_colorizer = sym.get_colorizer();
            raster_colorizer_ptr new_colorizer = raster_colorizer_ptr();
            new_colorizer->set_stops(old_colorizer->get_stops());
            new_colorizer->set_default_mode(old_colorizer->get_default_mode());
            new_colorizer->set_default_color(old_colorizer->get_default_color());
            new_colorizer->set_epsilon(old_colorizer->get_epsilon());
            sym.set_colorizer(new_colorizer);
        }

        void operator () (text_symbolizer & sym) const
        {
            copy_text_ptr(sym);
        }

        void operator () (shield_symbolizer & sym) const
        {
            copy_path_ptr(sym);
            copy_text_ptr(sym);
        }

        void operator () (building_symbolizer & sym) const
        {
            copy_height_ptr(sym);
        }


        template <typename T> void operator () (T &sym) const
        {
            boost::ignore_unused_variable_warning(sym);
        }

    private:
        template <class T>
        void copy_path_ptr(T & sym) const
        {
            std::string path = path_processor_type::to_string(*sym.get_filename());
            sym.set_filename( parse_path(path) );
        }

        template <class T>
        void copy_text_ptr(T & sym) const
        {
            MAPNIK_LOG_WARN(rule) << "rule: deep copying TextSymbolizers is broken!";
        }

        template <class T>
        void copy_height_ptr(T & sym) const
        {
            std::string height_expr = to_expression_string(sym.height());
            sym.set_height(parse_expression(height_expr,"utf8"));
        }
    };

public:
    rule()
        : name_(),
          min_scale_(0),
          max_scale_(std::numeric_limits<double>::infinity()),
          syms_(),
          filter_(boost::make_shared<mapnik::expr_node>(true)),
          else_filter_(false),
          also_filter_(false) {}

    rule(const std::string& name,
         double min_scale_denominator=0,
         double max_scale_denominator=std::numeric_limits<double>::infinity())
        : name_(name),
          min_scale_(min_scale_denominator),
          max_scale_(max_scale_denominator),
          syms_(),
          filter_(boost::make_shared<mapnik::expr_node>(true)),
          else_filter_(false),
          also_filter_(false)  {}

    rule(const rule& rhs, bool deep_copy = false)
        : name_(rhs.name_),
          min_scale_(rhs.min_scale_),
          max_scale_(rhs.max_scale_),
          syms_(rhs.syms_),
          filter_(rhs.filter_),
          else_filter_(rhs.else_filter_),
          also_filter_(rhs.also_filter_)
    {
        if (deep_copy) {

            std::string expr = to_expression_string(*filter_);
            filter_ = parse_expression(expr,"utf8");
            symbolizers::const_iterator it  = syms_.begin();
            symbolizers::const_iterator end = syms_.end();

            // FIXME - metawriter_ptr?

            for(; it != end; ++it)
            {
                boost::apply_visitor(deepcopy_symbolizer(),*it);
            }
        }
    }

    rule& operator=(rule const& rhs)
    {
        rule tmp(rhs);
        swap(tmp);
        return *this;
    }
    bool operator==(rule const& other)
    {
        return  (this == &other);
    }

    void set_max_scale(double scale)
    {
        max_scale_=scale;
    }

    double get_max_scale() const
    {
        return max_scale_;
    }

    void set_min_scale(double scale)
    {
        min_scale_=scale;
    }

    double get_min_scale() const
    {
        return min_scale_;
    }

    void set_name(std::string const& name)
    {
        name_=name;
    }

    std::string const& get_name() const
    {
        return name_;
    }

    void append(const symbolizer& sym)
    {
        syms_.push_back(sym);
    }

    void remove_at(size_t index)
    {
        if (index < syms_.size())
        {
            syms_.erase(syms_.begin()+index);
        }
    }

    const symbolizers& get_symbolizers() const
    {
        return syms_;
    }

    symbolizers::const_iterator begin() const
    {
        return syms_.begin();
    }

    symbolizers::const_iterator end() const
    {
        return syms_.end();
    }

    symbolizers::iterator begin()
    {
        return syms_.begin();
    }

    symbolizers::iterator end()
    {
        return syms_.end();
    }

    void set_filter(const expression_ptr& filter)
    {
        filter_=filter;
    }

    expression_ptr const& get_filter() const
    {
        return filter_;
    }

    void set_else(bool else_filter)
    {
        else_filter_=else_filter;
    }

    bool has_else_filter() const
    {
        return else_filter_;
    }

    void set_also(bool also_filter)
    {
        also_filter_=also_filter;
    }

    bool has_also_filter() const
    {
        return also_filter_;
    }

    bool active(double scale) const
    {
        return ( scale >= min_scale_ - 1e-6 && scale < max_scale_ + 1e-6);
    }

private:

    void swap(rule& rhs) throw()
    {
        name_=rhs.name_;
        min_scale_=rhs.min_scale_;
        max_scale_=rhs.max_scale_;
        syms_=rhs.syms_;
        filter_=rhs.filter_;
        else_filter_=rhs.else_filter_;
        also_filter_=rhs.also_filter_;
    }
};

}

#endif // MAPNIK_RULE_HPP
