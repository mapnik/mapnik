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

#ifndef MAPNIK_RULE_HPP
#define MAPNIK_RULE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/symbolizer_base.hpp>
#include <mapnik/expression.hpp>

// stl
#include <string>
#include <vector>
#include <limits>

namespace mapnik {
class MAPNIK_DECL rule
{
  public:
    using symbolizers = std::vector<symbolizer>;

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
    rule(const rule& rhs);
    rule(rule&& rhs);
    rule& operator=(rule rhs);
    bool operator==(rule const& rhs) const;
    void set_max_scale(double scale);
    double get_max_scale() const;
    void set_min_scale(double scale);
    double get_min_scale() const;
    void set_name(std::string const& name);
    std::string const& get_name() const;
    void append(symbolizer&& sym);
    void remove_at(size_t index);
    symbolizers const& get_symbolizers() const;
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
    inline bool active(double scale) const
    {
        return (scale >= min_scale_ - 1e-6 && scale < max_scale_ + 1e-6 && !syms_.empty());
    }
    inline void reserve(std::size_t size) { syms_.reserve(size); }
};

} // namespace mapnik

#endif // MAPNIK_RULE_HPP
