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

// mapnik
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
#include <mapnik/debug_symbolizer.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/config.hpp> // MAPNIK_DECL

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

inline bool operator==(debug_symbolizer const& lhs,
                       debug_symbolizer const& rhs)
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
                       markers_symbolizer,
                       debug_symbolizer> symbolizer;

class MAPNIK_DECL rule
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

public:
    rule();
    rule(std::string const& name,
         double min_scale_denominator = 0,
         double max_scale_denominator = std::numeric_limits<double>::infinity());
    rule(const rule& rhs, bool deep_copy = false);

    rule& operator=(rule const& rhs);
    bool operator==(rule const& other);
    void set_max_scale(double scale);
    double get_max_scale() const;
    void set_min_scale(double scale);
    double get_min_scale() const;
    void set_name(std::string const& name);
    std::string const& get_name() const;
    void append(symbolizer const& sym);
    void remove_at(size_t index);
    const symbolizers& get_symbolizers() const;
    symbolizers::const_iterator begin() const;
    symbolizers::const_iterator end() const;
    symbolizers::iterator begin();
    symbolizers::iterator end();
    void set_filter(expression_ptr const& filter);
    expression_ptr const& get_filter() const;
    void set_else(bool else_filter);
    bool has_else_filter() const;
    void set_also(bool also_filter);
    bool has_also_filter() const;
    bool active(double scale) const;

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
