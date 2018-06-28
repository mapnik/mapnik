/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#include <mapnik/expression_node.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/safe_cast.hpp>

namespace mapnik
{

regex_pattern_node::regex_pattern_node(value_unicode_string const& pat)
{
    UErrorCode status = U_ZERO_ERROR;
    pattern_.reset(RegexPattern::compile(pat, 0, status));
    if (U_FAILURE(status))
    {
        throw std::runtime_error("Failed to parse regex: " + to_utf8(pat));
    }
}

regex_match_node::regex_match_node(expr_node && arg,
                                   value_unicode_string const& pat)
    : regex_pattern_node(pat),
      expr(std::move(arg)) {}

value regex_match_node::apply(value const& v) const
{
    auto const& subject = v.to_unicode();
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<RegexMatcher> m;
    m.reset(pattern_->matcher(subject, status));
    return m && m->find(0, status);
}

std::string regex_match_node::to_string() const
{
    std::string str_;
    str_ += ".match('";
    pattern_->pattern().toUTF8String(str_); // FIXME escape ['\\]
    str_ += "')";
    return str_;
}

regex_replace_node::regex_replace_node(expr_node && arg,
                                       value_unicode_string const& pat,
                                       value_unicode_string const& fmt)
    : regex_pattern_node(pat),
      expr(std::move(arg)),
      format(fmt) {}

value regex_replace_node::apply(value const& v) const
{
    auto const& subject = v.to_unicode();
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<RegexMatcher> m;
    m.reset(pattern_->matcher(subject, status));
    return m ? m->replaceAll(format, status) : value{};
}

std::string regex_replace_node::to_string() const
{
    std::string str_;
    str_ += ".replace('";
    pattern_->pattern().toUTF8String(str_); // FIXME escape ['\\]
    str_ += "','";
    format.toUTF8String(str_);
    str_ += "')";
    return str_;
}

}
