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

#ifndef MAPNIK_EXPRESSION_NODE_HPP
#define MAPNIK_EXPRESSION_NODE_HPP

// mapnik
#include <mapnik/value.hpp>
#include <mapnik/attribute.hpp>

// boost
#include <boost/regex.hpp>
#if defined(BOOST_REGEX_HAS_ICU)
#include <boost/regex/icu.hpp>
#endif
#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace mapnik
{

namespace tags  {
struct negate
{
    static const char* str()
    {
        return "-";
    }
};

struct plus
{
    static const char* str()
    {
        return "+";
    }
};

struct minus
{
    static const char* str()
    {
        return "-";
    }
};

struct mult
{
    static const char* str()
    {
        return "*";
    }
};

struct div
{
    static const char* str()
    {
        return "/";
    }
};


struct  mod
{
    static const char* str()
    {
        return "%";
    }
};

struct less
{
    static const char* str()
    {
        return "<";
    }
};

struct  less_equal
{
    static const char* str()
    {
        return "<=";
    }
};

struct greater
{
    static const char* str()
    {
        return ">";
    }
};

struct greater_equal
{
    static const char* str()
    {
        return ">=";
    }
};

struct equal_to
{
    static const char* str()
    {
        return "=";
    }
};

struct not_equal_to
{
    static const char* str()
    {
        return "!=";
    }
};

struct logical_not
{
    static const char* str()
    {
        return "not ";
    }
};

struct logical_and
{
    static const char* str()
    {
        return " and ";
    }
};

struct logical_or
{
    static const char* str()
    {
        return " or ";
    }
};

} // end operation tags


template <typename Tag> struct binary_node;
template <typename Tag> struct unary_node;
struct regex_match_node;
struct regex_replace_node;

typedef mapnik::value value_type;

typedef boost::variant <
value_type,
attribute,
geometry_type_attribute,
boost::recursive_wrapper<unary_node<tags::negate> >,
boost::recursive_wrapper<binary_node<tags::plus> >,
boost::recursive_wrapper<binary_node<tags::minus> >,
boost::recursive_wrapper<binary_node<tags::mult> >,
boost::recursive_wrapper<binary_node<tags::div> >,
boost::recursive_wrapper<binary_node<tags::mod> >,
boost::recursive_wrapper<binary_node<tags::less> >,
boost::recursive_wrapper<binary_node<tags::less_equal> >,
boost::recursive_wrapper<binary_node<tags::greater> >,
boost::recursive_wrapper<binary_node<tags::greater_equal> >,
boost::recursive_wrapper<binary_node<tags::equal_to> >,
boost::recursive_wrapper<binary_node<tags::not_equal_to> >,
boost::recursive_wrapper<unary_node<tags::logical_not> >,
boost::recursive_wrapper<binary_node<tags::logical_and> >,
boost::recursive_wrapper<binary_node<tags::logical_or> >,
boost::recursive_wrapper<regex_match_node>,
boost::recursive_wrapper<regex_replace_node>
> expr_node;

template <typename Tag> struct make_op;
template <> struct make_op<tags::negate> { typedef std::negate<value_type> type;};
template <> struct make_op<tags::plus> { typedef std::plus<value_type> type;};
template <> struct make_op<tags::minus> { typedef std::minus<value_type> type;};
template <> struct make_op<tags::mult> { typedef std::multiplies<value_type> type;};
template <> struct make_op<tags::div> { typedef std::divides<value_type> type;};
template <> struct make_op<tags::mod> { typedef std::modulus<value_type> type;};
template <> struct make_op<tags::less> { typedef std::less<value_type> type;};
template <> struct make_op<tags::less_equal> { typedef std::less_equal<value_type> type;};
template <> struct make_op<tags::greater> { typedef std::greater<value_type> type;};
template <> struct make_op<tags::greater_equal> { typedef std::greater_equal<value_type> type;};
template <> struct make_op<tags::equal_to> { typedef std::equal_to<value_type> type;};
template <> struct make_op<tags::not_equal_to> { typedef std::not_equal_to<value_type> type;};
template <> struct make_op<tags::logical_not> { typedef std::logical_not<value_type> type;};
template <> struct make_op<tags::logical_and> { typedef std::logical_and<value_type> type;};
template <> struct make_op<tags::logical_or> { typedef std::logical_or<value_type> type;};

template <typename Tag>
struct unary_node
{
    unary_node (expr_node const& a)
        : expr(a) {}

    static const char* type()
    {
        return Tag::str();
    }

    expr_node expr;
};

template <typename Tag>
struct binary_node
{
    binary_node(expr_node const& a, expr_node const& b)
        : left(a),
          right(b) {}

    static const char* type()
    {
        return Tag::str();
    }
    expr_node left,right;
};

#if defined(BOOST_REGEX_HAS_ICU)

struct regex_match_node
{
    regex_match_node (expr_node const& a, UnicodeString const& ustr);
    expr_node expr;
    boost::u32regex pattern;
};


struct regex_replace_node
{
    regex_replace_node (expr_node const& a, UnicodeString const& ustr, UnicodeString const& f);
    expr_node expr;
    boost::u32regex pattern;
    UnicodeString format;
};

#else

struct regex_match_node
{
    regex_match_node (expr_node const& a, std::string const& str);
    expr_node expr;
    boost::regex pattern;
};


struct regex_replace_node
{
    regex_replace_node (expr_node const& a, std::string const& str, std::string const& f);
    expr_node expr;
    boost::regex pattern;
    std::string format;
};
#endif

struct function_call
{
    template<typename Fun>
    explicit function_call (expr_node const a, Fun f)
        : expr(a),
          call_(f) {}

    expr_node expr;
    boost::function<value_type(value_type)> call_;
};

// ops

inline expr_node& operator- (expr_node& expr)
{
    return expr = unary_node<tags::negate>(expr);
}

inline expr_node & operator += ( expr_node &left ,const expr_node &right)
{
    return left =  binary_node<tags::plus>(left,right);
}

inline expr_node & operator -= ( expr_node &left ,const expr_node &right)
{
    return left =  binary_node<tags::minus>(left,right);
}

inline expr_node & operator *= ( expr_node &left ,const expr_node &right)
{
    return left =  binary_node<tags::mult>(left,right);
}

inline expr_node & operator /= ( expr_node &left ,const expr_node &right)
{
    return left =  binary_node<tags::div>(left,right);
}

inline expr_node & operator %= ( expr_node &left ,const expr_node &right)
{
    return left = binary_node<tags::mod>(left,right);
}

inline expr_node & operator < ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::less>(left,right);
}

inline expr_node & operator <= ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::less_equal>(left,right);
}

inline expr_node & operator > ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::greater>(left,right);
}

inline expr_node & operator >= ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::greater_equal>(left,right);
}

inline expr_node & operator == ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::equal_to>(left,right);
}

inline expr_node & operator != ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::not_equal_to>(left,right);
}

inline expr_node & operator ! (expr_node & expr)
{
    return expr = unary_node<tags::logical_not>(expr);
}

inline expr_node & operator && ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::logical_and>(left,right);
}

inline expr_node & operator || ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::logical_or>(left,right);
}

}


#endif //MAPNIK_EXPRESSION_NODE_HPP
