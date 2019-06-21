/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2018 Artem Pavlenko
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

#ifndef MAPNIK_UTIL_SPIRIT_RULE_HPP
#define MAPNIK_UTIL_SPIRIT_RULE_HPP

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace util { namespace spirit {

namespace x3 = boost::spirit::x3;

using x3::operator |    ; // alternative
using x3::operator &    ; // and predicate
using x3::operator !    ; // not predicate
using x3::operator *    ; // kleene star
using x3::operator +    ; // one or more
using x3::operator -    ; // difference, optional
using x3::operator %    ; // list
using x3::operator >    ; // expectation
using x3::operator >>   ; // sequence

template <typename T>
struct actual_attribute_type
{
    using type = T;
};

template <>
struct actual_attribute_type<x3::unused_type>
{
    using type = x3::unused_type const;
};

// We need ADL to find the `parse_rule` function template defined here.
// Had we used `x3::rule<TagName, Attribute>`, this namespace would not
// be considered. One method to get the attention of ADL is wrapping
// the TagName in a class template declared here (need not be defined).
// Hence we use `x3::rule<id<TagName>, Attribute>`.
template <typename TagName>
struct id;

template <typename TagName>
struct rule_proxy
{
    using base_type = rule_proxy<TagName>;
    using type = TagName;

    template <typename T = x3::unused_type>
    using actual_attribute_t = typename actual_attribute_type<T>::type;

    template <typename T = x3::unused_type>
    using proxy_attribute_t = T;

    template <typename... Attribute>
    using proxy_rule_t = x3::rule<id<TagName>, Attribute...>;

    // subscript operator cannot be non-member
    template <typename Action>
    decltype(auto)
    operator[] (Action func) const
    {
        return as_spirit_parser(*this)[func];
    }

    // constexpr constructor ensures that non-local instances are
    // initialized before use (even from another translation unit)
    constexpr rule_proxy(char const* name_)
        : name(name_) {}

    char const* name;
};

template <typename TagName>
typename TagName::rule_type
as_spirit_parser(rule_proxy<TagName> const& rule)
{
    return { rule.name };
}

// Spirit functions keep fully spelling out x3::rule<ID, Attribute>.
// This alias template hides the ID wrapper and Attribute parameter
// to make error messages inside `parse_rule` a bit shorter.
template <typename TagName>
using rule_t = x3::rule<id<TagName>, typename TagName::attribute_type>;

template <typename TagName, typename Iterator,
          typename Context, typename ActualAttribute>
bool parse_rule(rule_t<TagName> rule,
                Iterator & first, Iterator const& last,
                Context const& ctx, ActualAttribute & attr)
{
    // "error: no member named 'parse' in 'TagName'" pointing on the
    // next line means MAPNIK_SPIRIT_RULE(TagName) was used without
    // subsequent MAPNIK_SPIRIT_RULE_DEF(TagName) = expression;
    // the trailing comment should be included in the error message
    return TagName::def(rule).parse // ~~~~~~ missing MAPNIK_SPIRIT_RULE_DEF ~~~~~~
           (first, last, ctx, x3::unused, attr);
}

}}} // namespace mapnik::util::spirit

///     MAPNIK_SPIRIT_RULE(Name, Attribute = unused_type)
#define MAPNIK_SPIRIT_RULE(Name, ...)                               \
        template <typename TagName>                                 \
        extern TagName _rule_def_;                                  \
        /* the implicit specialization of _rule_def_<T> serves   */ \
        /* only to catch errors, the variable is not defined     */ \
        struct Name : mapnik::util::spirit::rule_proxy<Name>        \
        {                                                           \
            static Name const proxy;   /* holds rule name string */ \
                                                                    \
            using actual_type = actual_attribute_t<__VA_ARGS__>;    \
            using attribute_type = proxy_attribute_t<__VA_ARGS__>;  \
            using rule_type = proxy_rule_t<__VA_ARGS__>;            \
            using base_type::base_type;   /* inherit constructor */ \
                                                                    \
            template <typename TagName>                             \
            static decltype(auto)                                   \
            def(mapnik::util::spirit::rule_t<TagName>)              \
            {                                                       \
                return _rule_def_<TagName>;                         \
            }                                                       \
            /* in order to defer the instantiation of variable   */ \
            /* template _rule_def_<T> until after its explicit   */ \
            /* specialization, the getter cannot just tell the   */ \
            /* compiler that T will be the Name defined hereby,  */ \
            /* it must use an unknown template parameter         */ \
        };                                                          \
        /* `struct Name` is the type, `Name` is the variable     */ \
        struct Name const Name = Name::proxy                        \
        /* ; expected */

///     MAPNIK_SPIRIT_RULE_DEF(Name) = parsing expression;
#define MAPNIK_SPIRIT_RULE_DEF(Name)                                \
        template <> auto const _rule_def_<struct Name>              \
                               = as_spirit_parser(Name::proxy)      \
        /***/

///     MAPNIK_SPIRIT_RULE_NAME(TagName) = "Fancy rule name";
#define MAPNIK_SPIRIT_RULE_NAME(TagName)                            \
        struct TagName const TagName::proxy                         \
        /***/

///     MAPNIK_SPIRIT_LOCAL_RULE(TagName, Attribute = unused_type)
#define MAPNIK_SPIRIT_LOCAL_RULE(TagName, ...)                      \
        MAPNIK_SPIRIT_RULE(TagName, __VA_ARGS__);                   \
        MAPNIK_SPIRIT_RULE_DEF(TagName)                             \
        /* = parsing expression; expected */

///     MAPNIK_SPIRIT_EXTERN_RULE(TagName, Attribute = unused_type)
#define MAPNIK_SPIRIT_EXTERN_RULE(TagName, ...)                     \
        MAPNIK_SPIRIT_RULE(TagName, __VA_ARGS__);                   \
        MAPNIK_SPIRIT_declare_parse_rule_(~, ~, TagName)            \
        /* ; expected */

///     MAPNIK_SPIRIT_EXTERN_RULE_DEF(TagName)
#define MAPNIK_SPIRIT_EXTERN_RULE_DEF(TagName)                      \
        MAPNIK_SPIRIT_define_parse_rule_(~, ~, TagName)             \
        MAPNIK_SPIRIT_RULE_DEF(TagName)                             \
        /* = parsing expression; expected */

///     MAPNIK_SPIRIT_INSTANTIATE(TagName, Iterator, Context)
#define MAPNIK_SPIRIT_INSTANTIATE(TagName, Iterator, Context)               \
        template bool parse_rule<Iterator, Context, TagName::actual_type>   \
                                (TagName::rule_type rule,                   \
                                 Iterator & first, Iterator const& last,    \
                                 Context const& ctx,                        \
                                 TagName::actual_type & attr)               \
        /* ; expected */

// The following two macros were originally intended to be called from
// a variadic macro similar to BOOST_SPIRIT_DEFINE. In case this usage
// needs to be re-introduced, I kept the two initial arguments that were
// used by BOOST_PP_SEQ_FOR_EACH, although they're unused now.

#define MAPNIK_SPIRIT_declare_parse_rule_(r, data, TagName)                 \
        template <typename Iterator, typename Context, typename Attribute>  \
        bool parse_rule(TagName::rule_type rule,                            \
                        Iterator & first, Iterator const& last,             \
                        Context const& ctx, Attribute & attr)               \
        /***/

#define MAPNIK_SPIRIT_define_parse_rule_(r, data, TagName)          \
        MAPNIK_SPIRIT_declare_parse_rule_(r, data, TagName)         \
        {                                                           \
            return mapnik::util::spirit::parse_rule                 \
                   (rule, first, last, ctx, attr);                  \
        }                                                           \
        /***/

#endif // MAPNIK_UTIL_SPIRIT_RULE_HPP
