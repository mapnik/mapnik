/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_RULE_CACHE_HPP
#define MAPNIK_RULE_CACHE_HPP

// mapnik
#include <mapnik/expression_node.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/util/noncopyable.hpp>

// stl
#include <string>
#include <span>
#include <unordered_map>
#include <vector>

namespace mapnik {

class rule_cache : private util::noncopyable
{
  private:
    struct precondition
    {
        std::string name;
        value expected;
    };

    static bool extract_precondition(expr_node const& node, precondition& result)
    {
        if (node.is<binary_node<tags::equal_to>>())
        {
            auto const& equality = node.get_unchecked<binary_node<tags::equal_to>>();
            if (equality.left.is<attribute>() && equality.right.is<value_unicode_string>())
            {
                result.name = equality.left.get_unchecked<attribute>().name();
                result.expected = value(equality.right.get_unchecked<value_unicode_string>());
                return true;
            }
            if (equality.right.is<attribute>() && equality.left.is<value_unicode_string>())
            {
                result.name = equality.right.get_unchecked<attribute>().name();
                result.expected = value(equality.left.get_unchecked<value_unicode_string>());
                return true;
            }
            return false;
        }
        if (node.is<binary_node<tags::logical_and>>())
        {
            auto const& conjunction = node.get_unchecked<binary_node<tags::logical_and>>();
            return extract_precondition(conjunction.left, result) || extract_precondition(conjunction.right, result);
        }
        return false;
    }

  public:
    using rule_ptrs = std::vector<rule const*>;
    using rule_indices = std::vector<std::size_t>;
    using precondition_values = std::unordered_map<value, rule_indices>;

    struct precondition_group
    {
        std::string name;
        precondition_values rules;
    };

    rule_cache()
        : if_rules_(),
          else_rules_(),
          also_rules_(),
          rules_without_precondition_(),
          precondition_groups_()
    {}

    rule_cache(rule_cache&& rhs) // move ctor
        : if_rules_(std::move(rhs.if_rules_)),
          else_rules_(std::move(rhs.else_rules_)),
          also_rules_(std::move(rhs.also_rules_)),
          rules_without_precondition_(std::move(rhs.rules_without_precondition_)),
          precondition_groups_(std::move(rhs.precondition_groups_))
    {}

    rule_cache& operator=(rule_cache&& rhs) // move assign
    {
        std::swap(if_rules_, rhs.if_rules_);
        std::swap(else_rules_, rhs.else_rules_);
        std::swap(also_rules_, rhs.also_rules_);
        std::swap(rules_without_precondition_, rhs.rules_without_precondition_);
        std::swap(precondition_groups_, rhs.precondition_groups_);
        return *this;
    }

    void add_rule(rule const& r)
    {
        if (r.has_else_filter())
        {
            else_rules_.push_back(&r);
        }
        else if (r.has_also_filter())
        {
            also_rules_.push_back(&r);
        }
        else
        {
            std::size_t const index = if_rules_.size();
            if_rules_.push_back(&r);
            precondition condition;
            expression_ptr const& filter = r.get_filter();
            if (!filter || !extract_precondition(*filter, condition))
            {
                rules_without_precondition_.push_back(index);
                return;
            }

            for (precondition_group& group : precondition_groups_)
            {
                if (group.name == condition.name)
                {
                    group.rules[condition.expected].push_back(index);
                    return;
                }
            }
            precondition_groups_.push_back({condition.name, {}});
            precondition_groups_.back().rules[condition.expected].push_back(index);
        }
    }

    rule_ptrs const& get_if_rules() const { return if_rules_; }

    rule_ptrs const& get_else_rules() const { return else_rules_; }

    rule_ptrs const& get_also_rules() const { return also_rules_; }

    std::span<std::size_t const> get_rules_without_precondition() const { return rules_without_precondition_; }

    std::span<precondition_group const> get_precondition_groups() const { return precondition_groups_; }

  private:
    rule_ptrs if_rules_;
    rule_ptrs else_rules_;
    rule_ptrs also_rules_;
    rule_indices rules_without_precondition_;
    std::vector<precondition_group> precondition_groups_;
};

} // namespace mapnik

#endif // MAPNIK_RULE_CACHE_HPP
