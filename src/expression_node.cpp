/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/value_types.hpp>
#include <mapnik/util/noncopyable.hpp>

#if defined(BOOST_REGEX_HAS_ICU)
#include <boost/regex/icu.hpp>
#else
#include <boost/regex.hpp>
#endif

namespace mapnik
{

struct _regex_match_impl : util::noncopyable {
#if defined(BOOST_REGEX_HAS_ICU)
    _regex_match_impl(value_unicode_string const& ustr) :
        pattern_(boost::make_u32regex(ustr)) {}
    boost::u32regex pattern_;
#else
    _regex_match_impl(std::string const& ustr) :
        pattern_(ustr) {}
    boost::regex pattern_;
#endif
};

struct _regex_replace_impl : util::noncopyable {
#if defined(BOOST_REGEX_HAS_ICU)
    _regex_replace_impl(value_unicode_string const& ustr, value_unicode_string const& f) :
        pattern_(boost::make_u32regex(ustr)),
        format_(f) {}
    boost::u32regex pattern_;
    value_unicode_string format_;
#else
    _regex_replace_impl(std::string const& ustr,std::string const& f) :
        pattern_(ustr),
        format_(f) {}
    boost::regex pattern_;
    std::string format_;
#endif
};


regex_match_node::regex_match_node(transcoder const& tr,
                                   expr_node const& a,
                                   std::string const& ustr)
    : expr(a),
      impl_(new _regex_match_impl(
#if defined(BOOST_REGEX_HAS_ICU)
        tr.transcode(ustr.c_str())
#else
        ustr
#endif
        )) {}

value regex_match_node::apply(value const& v) const
{
    auto const& pattern = impl_.get()->pattern_;
#if defined(BOOST_REGEX_HAS_ICU)
    return boost::u32regex_match(v.to_unicode(),pattern);
#else
    return boost::regex_match(v.to_string(),pattern);
#endif
}

std::string regex_match_node::to_string() const
{
    std::string str_;
    str_ +=".match('";
    auto const& pattern = impl_.get()->pattern_;
#if defined(BOOST_REGEX_HAS_ICU)
    std::string utf8;
    value_unicode_string ustr = value_unicode_string::fromUTF32( &pattern.str()[0], pattern.str().length());
    to_utf8(ustr,utf8);
    str_ += utf8;
#else
    str_ += pattern.str();
#endif
    str_ +="')";
    return str_;
}

regex_replace_node::regex_replace_node(transcoder const& tr,
                                       expr_node const& a,
                                       std::string const& ustr,
                                       std::string const& f)
    : expr(a),
      impl_(new _regex_replace_impl(
#if defined(BOOST_REGEX_HAS_ICU)
                     tr.transcode(ustr.c_str()),
                     tr.transcode(f.c_str())
#else
                     ustr,
                     f
#endif
                     )) {}

value regex_replace_node::apply(value const& v) const
{
    auto const& pattern = impl_.get()->pattern_;
    auto const& format = impl_.get()->format_;
#if defined(BOOST_REGEX_HAS_ICU)
    return boost::u32regex_replace(v.to_unicode(),pattern,format);
#else
    std::string repl = boost::regex_replace(v.to_string(),pattern,format);
    transcoder tr_("utf8");
    return tr_.transcode(repl.c_str());
#endif
}

std::string regex_replace_node::to_string() const
{
    std::string str_;
    str_ +=".replace(";
    str_ += "'";
    auto const& pattern = impl_.get()->pattern_;
    auto const& format = impl_.get()->format_;
#if defined(BOOST_REGEX_HAS_ICU)
    std::string utf8;
    value_unicode_string ustr = value_unicode_string::fromUTF32( &pattern.str()[0], pattern.str().length());
    to_utf8(ustr,utf8);
    str_ += utf8;
    str_ +="','";
    to_utf8(format ,utf8);
    str_ += utf8;
#else
    str_ += pattern.str();
    str_ +="','";
    str_ += format;
#endif
    str_ +="')";
    return str_;
}

}
