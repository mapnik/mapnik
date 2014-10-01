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

// mapnik
#include <mapnik/rule.hpp>
#include <mapnik/expression_node.hpp>

// stl
#include <limits>
#include <memory>

namespace mapnik
{

rule::rule()
    : name_(),
      min_scale_(0),
      max_scale_(std::numeric_limits<double>::infinity()),
      syms_(),
      filter_(std::make_shared<expr_node>(true)),
      else_filter_(false),
      also_filter_(false) {}

rule::rule(std::string const& name,
     double min_scale_denominator,
     double max_scale_denominator)
    : name_(name),
      min_scale_(min_scale_denominator),
      max_scale_(max_scale_denominator),
      syms_(),
      filter_(std::make_shared<mapnik::expr_node>(true)),
      else_filter_(false),
      also_filter_(false)  {}

rule::rule(rule const& rhs)
    : name_(rhs.name_),
      min_scale_(rhs.min_scale_),
      max_scale_(rhs.max_scale_),
      syms_(rhs.syms_),
      filter_(std::make_shared<expr_node>(*rhs.filter_)),
      else_filter_(rhs.else_filter_),
      also_filter_(rhs.also_filter_) {}

rule::rule(rule && rhs)
    : name_(std::move(rhs.name_)),
      min_scale_(std::move(rhs.min_scale_)),
      max_scale_(std::move(rhs.max_scale_)),
      syms_(std::move(rhs.syms_)),
      filter_(std::move(rhs.filter_)),
      else_filter_(std::move(rhs.else_filter_)),
      also_filter_(std::move(rhs.also_filter_)) {}

rule& rule::operator=(rule rhs)
{
    using std::swap;
    swap(this->name_, rhs.name_);
    swap(this->min_scale_, rhs.min_scale_);
    swap(this->max_scale_, rhs.max_scale_);
    swap(this->syms_, rhs.syms_);
    swap(this->filter_, rhs.filter_);
    swap(this->else_filter_, rhs.else_filter_);
    swap(this->also_filter_, rhs.also_filter_);
    return *this;
}

bool rule::operator==(rule const& rhs) const
{
    return  (name_ == rhs.name_) &&
        (min_scale_ == rhs.min_scale_) &&
        (max_scale_ == rhs.max_scale_) &&
        (syms_ == rhs.syms_) &&
        (filter_ == rhs.filter_) &&
        (else_filter_ == rhs.else_filter_) &&
        (also_filter_ == rhs.also_filter_);
}


void rule::set_max_scale(double scale)
{
    max_scale_=scale;
}

double rule::get_max_scale() const
{
    return max_scale_;
}

void rule::set_min_scale(double scale)
{
    min_scale_=scale;
}

double rule::get_min_scale() const
{
    return min_scale_;
}

void rule::set_name(std::string const& name)
{
    name_=name;
}

std::string const& rule::get_name() const
{
    return name_;
}

void rule::append(symbolizer && sym)
{
    syms_.push_back(std::move(sym));
}

void rule::remove_at(size_t index)
{
    if (index < syms_.size())
    {
        syms_.erase(syms_.begin()+index);
    }
}

rule::symbolizers const& rule::get_symbolizers() const
{
    return syms_;
}

rule::symbolizers::const_iterator rule::begin() const
{
    return syms_.begin();
}

rule::symbolizers::const_iterator rule::end() const
{
    return syms_.end();
}

rule::symbolizers::iterator rule::begin()
{
    return syms_.begin();
}

rule::symbolizers::iterator rule::end()
{
    return syms_.end();
}

void rule::set_filter(expression_ptr const& filter)
{
    filter_=filter;
}

expression_ptr const& rule::get_filter() const
{
    return filter_;
}

void rule::set_else(bool else_filter)
{
    else_filter_=else_filter;
}

bool rule::has_else_filter() const
{
    return else_filter_;
}

void rule::set_also(bool also_filter)
{
    also_filter_=also_filter;
}

bool rule::has_also_filter() const
{
    return also_filter_;
}

}
