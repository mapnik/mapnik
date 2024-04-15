/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_ENUMERATION_HPP
#define MAPNIK_ENUMERATION_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/debug.hpp>

// stl
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <array>
#include <tuple>
#include <stdexcept>
#include <map>
#if __cpp_lib_string_view >= 201606L
#include <string_view>
#endif

#include <mapnik/warning.hpp>

namespace mapnik {

class illegal_enum_value : public std::exception
{
  public:
    illegal_enum_value()
        : what_()
    {}

    illegal_enum_value(std::string const& _what)
        : what_(_what)
    {}
    virtual ~illegal_enum_value() {}

    virtual const char* what() const noexcept { return what_.c_str(); }

  protected:
    const std::string what_;
};

namespace detail {
#if __cpp_lib_string_view >= 201606L
using mapnik_string_view = std::string_view;
#else
class mapnik_string_view // use std::string_view in C++17
{
  public:

    template<std::size_t N>
    constexpr mapnik_string_view(const char (&s)[N])
        : size_(N)
        , data_(s)
    {}

    constexpr mapnik_string_view(const char* s, std::size_t N)
        : size_(N)
        , data_(s)
    {}

    constexpr char operator[](std::size_t index) const
    {
        return (index >= size_) ? throw std::out_of_range("Invalid index.") : data_[index];
    }

    constexpr char const* data() const { return data_; }

  private:
    std::size_t size_;
    const char* data_;
};

constexpr bool mapnik_string_view_equals(mapnik_string_view const& a, char const* b, std::size_t i)
{
    if (a[i] != b[i])
    {
        return false;
    }
    else if (a[i] == 0 || b[i] == 0)
    {
        return true;
    }
    else
    {
        return mapnik_string_view_equals(a, b, i + 1);
    }
}

constexpr bool operator==(mapnik_string_view const& a, char const* b)
{
    return mapnik_string_view_equals(a, b, 0);
}
#endif

template<class EnumT>
using EnumStringT = std::tuple<EnumT, mapnik_string_view>;

template<class EnumT, std::size_t N>
using EnumMapT = std::array<EnumStringT<EnumT>, N>;

template<class EnumT, std::size_t N>
constexpr char const* EnumGetValue(EnumMapT<EnumT, N> const& map, EnumT key, std::size_t i = 0)
{
    if (i >= map.size())
    {
        throw illegal_enum_value{"Enum value not present in map."};
    }
    return (std::get<0>(map[i]) == key) ? std::get<1>(map[i]).data() : EnumGetValue(map, key, i + 1);
}

template<class EnumT, std::size_t N>
constexpr EnumT EnumGetKey(EnumMapT<EnumT, N> const& map, char const* value, std::size_t i = 0)
{
    if (i >= map.size())
    {
        throw illegal_enum_value{"Enum key not present in map."};
    }
    return (std::get<1>(map[i]) == value) ? std::get<0>(map[i]) : EnumGetKey(map, value, i + 1);
}
} // namespace detail

template<typename ENUM,
         char const* (*F_TO_STRING)(ENUM),
         ENUM (*F_FROM_STRING)(const char*),
         std::map<ENUM, std::string> (*F_LOOKUP)()>
struct MAPNIK_DECL enumeration
{
    using native_type = ENUM;
    constexpr operator ENUM() const { return value_; }
    void operator=(ENUM v) { value_ = v; }

    enumeration()
        : value_()
    {}

    enumeration(ENUM v)
        : value_(v)
    {}

    void from_string(const std::string& str) { value_ = F_FROM_STRING(str.c_str()); }
    std::string as_string() const { return F_TO_STRING(value_); }
    static std::map<ENUM, std::string> lookupMap() { return F_LOOKUP(); }

    ENUM value_;
};

#define DEFINE_ENUM_FNCS(fnc_name, enum_class)                                                                         \
    MAPNIK_DECL char const* fnc_name##_to_string(enum_class value);                                                    \
    MAPNIK_DECL enum_class fnc_name##_from_string(const char* value);                                                  \
    MAPNIK_DECL std::ostream& operator<<(std::ostream& stream, enum_class value);                                      \
    MAPNIK_DECL std::map<enum_class, std::string> fnc_name##_lookup();

#define IMPLEMENT_ENUM_FNCS(fnc_name, enum_class)                                                                      \
    char const* fnc_name##_to_string(enum_class value)                                                                 \
    {                                                                                                                  \
        return mapnik::detail::EnumGetValue(fnc_name##_map, value);                                                    \
    }                                                                                                                  \
    enum_class fnc_name##_from_string(const char* value)                                                               \
    {                                                                                                                  \
        return mapnik::detail::EnumGetKey(fnc_name##_map, value);                                                      \
    }                                                                                                                  \
    std::ostream& operator<<(std::ostream& stream, enum_class value)                                                   \
    {                                                                                                                  \
        stream << fnc_name##_to_string(value);                                                                         \
        return stream;                                                                                                 \
    }                                                                                                                  \
    std::map<enum_class, std::string> fnc_name##_lookup()                                                              \
    {                                                                                                                  \
        std::map<enum_class, std::string> val_map;                                                                     \
        std::transform(                                                                                                \
          fnc_name##_map.begin(),                                                                                      \
          fnc_name##_map.end(),                                                                                        \
          std::inserter(val_map, val_map.end()),                                                                       \
          [](const mapnik::detail::EnumStringT<enum_class>& val) {                                                     \
              return std::pair<enum_class, std::string>{std::get<0>(val), std::string{std::get<1>(val).data()}};       \
          });                                                                                                          \
        return val_map;                                                                                                \
    }

#define DEFINE_ENUM(type_alias, enum_class)                                                                            \
    DEFINE_ENUM_FNCS(type_alias, enum_class)                                                                           \
    using type_alias = enumeration<enum_class, type_alias##_to_string, type_alias##_from_string, type_alias##_lookup>; \
    extern template struct MAPNIK_DECL                                                                                 \
      enumeration<enum_class, type_alias##_to_string, type_alias##_from_string, type_alias##_lookup>;

#define IMPLEMENT_ENUM(type_alias, enum_class)                                                                         \
    IMPLEMENT_ENUM_FNCS(type_alias, enum_class)                                                                        \
    template struct MAPNIK_DECL                                                                                        \
      enumeration<enum_class, type_alias##_to_string, type_alias##_from_string, type_alias##_lookup>;

/** Slim wrapper for enumerations. It creates a new type from a native enum and
 * a char pointer array. It almost exactly behaves like a native enumeration
 * type. It supports string conversion through stream operators. This is useful
 * for debugging, serialization/deserialization and also helps with implementing
 * language bindings. The two convenient macros DEFINE_ENUM() and IMPLEMENT_ENUM()
 * are provided to help with instanciation.
 *
 * @par Limitations:
 *    - The enum must start at zero.
 *    - The enum must be consecutive.
 *    - The enum must be terminated with a special token consisting of the enum's
 *      name plus "_MAX".
 *    - The corresponding char pointer array must be terminated with an empty string.
 *    - The names must only consist of characters and digits (<i>a-z, A-Z, 0-9</i>),
 *      underscores (<i>_</i>) and dashes (<i>-</i>).
 *
 *
 * @warning At the moment the verify_mapnik_enum() method is called during static initialization.
 * It quits the application with exit code 1 if any error is detected. The other solution
 * i thought of is to do the checks at compile time (using boost::mpl).
 *
 * @par Example:
 * The following code goes into the header file:
 * @code
 * enum fruit_enum {
 *      APPLE,
 *      CHERRY,
 *      BANANA,
 *      PASSION_FRUIT,
 *      fruit_enum_MAX
 * };
 *
 * static const char * fruit_strings[] = {
 *      "apple",
 *      "cherry",
 *      "banana",
 *      "passion_fruit",
 *      ""
 * };
 *
 * DEFINE_ENUM( fruit, fruit_enum);
 * @endcode
 * In the corresponding cpp file do:
 * @code
 * IMPLEMENT_ENUM( fruit, fruit_strings );
 * @endcode
 * And here is how to use the resulting type Fruit
 * @code
 *
 * int
 * main(int argc, char * argv[]) {
 *      fruit f(APPLE);
 *      switch ( f ) {
 *          case BANANA:
 *          case APPLE:
 *              cerr << "No thanks. I hate " << f << "s" << endl;
 *              break;
 *          default:
 *              cerr << "Hmmm ... yummy " << f << endl;
 *              break;
 *      }
 *
 *      f = CHERRY;
 *
 *      fruit_enum native_enum = f;
 *
 *      f.from_string("passion_fruit");
 *
 *      for (unsigned i = 0; i < fruit::MAX; ++i) {
 *          cerr << i << " = " << fruit::get_string(i) << endl;
 *      }
 *
 *      f.from_string("elephant"); // throws illegal_enum_value
 *
 *      return 0;
 * }
 * @endcode
 */

} // namespace mapnik

#endif // MAPNIK_ENUMERATION_HPP
