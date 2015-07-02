/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <bitset>
#include <iostream>
#include <cstdlib>
#include <algorithm>

namespace mapnik {

class illegal_enum_value : public std::exception
{
public:
    illegal_enum_value():
        what_() {}

    illegal_enum_value( std::string const& what ) :
        what_( what )
    {
    }
    virtual ~illegal_enum_value() throw() {}

    virtual const char * what() const throw()
    {
        return what_.c_str();
    }

protected:
    std::string what_;
};


/** Slim wrapper for enumerations. It creates a new type from a native enum and
 * a char pointer array. It almost exactly behaves like a native enumeration
 * type. It supports string conversion through stream operators. This is usefull
 * for debugging, serialization/deserialization and also helps with implementing
 * language bindings. The two convinient macros DEFINE_ENUM() and IMPLEMENT_ENUM()
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

template <typename ENUM, int THE_MAX>
class MAPNIK_DECL enumeration {
public:
    using native_type = ENUM;

    enumeration()
        :  value_() {}

    enumeration( ENUM v )
        :  value_(v) {}

    enumeration( enumeration const& other )
        : value_(other.value_) {}

    /** Assignment operator for native enum values. */
    void operator=(ENUM v)
    {
        value_ = v;
    }

    /** Assignment operator. */
    void operator=(enumeration const& other)
    {
        value_ = other.value_;
    }

    /** Conversion operator for native enum values. */
    operator ENUM() const
    {
        return value_;
    }

    enum Max
    {
        MAX = THE_MAX
    };

    /** Converts @p str to an enum.
     * @throw illegal_enum_value @p str is not a legal identifier.
     * */
    void from_string(std::string const& str)
    {
        // TODO: Enum value strings with underscore are deprecated in Mapnik 3.x
        // and support will be removed in Mapnik 4.x.
        bool deprecated = false;
        std::string str_copy(str);
        if (str_copy.find('_') != std::string::npos)
        {
            std::replace(str_copy.begin(), str_copy.end(), '_', '-');
            deprecated = true;
        }
        for (unsigned i = 0; i < THE_MAX; ++i)
        {
            if (str_copy == our_strings_[i])
            {
                value_ = static_cast<ENUM>(i);
                if (deprecated)
                {
                    MAPNIK_LOG_ERROR(enumerations) << "enumeration value (" << str << ") using \"_\" is deprecated and will be removed in Mapnik 4.x, use '" << str_copy << "' instead";
                }
                return;
            }
        }
        throw illegal_enum_value(std::string("Illegal enumeration value '") +
                                 str + "' for enum " + our_name_);
    }

    /** Parses the input stream @p is for a word consisting of characters and
     * digits (<i>a-z, A-Z, 0-9</i>) and underscores (<i>_</i>).
     * The failbit of the stream is set if the word is not a valid identifier.
     */
    std::istream & parse(std::istream & is)
    {
        std::string word;
        char c;

        while ( is.peek() != std::char_traits< char >::eof())
        {
            is >> c;
            if ( isspace(c) && word.empty() )
            {
                continue;
            }
            if ( isalnum(c) || (c == '_') || c == '-' )
            {
                word += c;
            }
            else
            {
                is.unget();
                break;
            }
        }

        try
        {
            from_string( word );
        }
        catch (illegal_enum_value const&)
        {
            is.setstate(std::ios::failbit);
        }

        return is;
    }

    /** Returns the current value as a string identifier. */
    std::string as_string() const
    {
        return our_strings_[value_];
    }

    /** Prints the string identifier to the output stream @p os. */
    std::ostream & print(std::ostream & os = std::cerr) const
    {
        return os << our_strings_[value_];
    }

    /** Static helper function to iterate over valid identifiers. */
    static const char * get_string(unsigned i)
    {
        return our_strings_[i];
    }

    /** Performs some simple checks and quits the application if
     * any error is detected. Tries to print helpful error messages.
     */
    static bool verify_mapnik_enum(const char * filename, unsigned line_no)
    {
        for (unsigned i = 0; i < THE_MAX; ++i)
        {
            if (our_strings_[i] == 0 )
            {
                std::cerr << "### FATAL: Not enough strings for enum "
                          << our_name_ << " defined in file '" << filename
                          << "' at line " << line_no;
            }
        }
        if ( std::string("") != our_strings_[THE_MAX])
        {
            std::cerr << "### FATAL: The string array for enum " << our_name_
                      << " defined in file '" << filename << "' at line " << line_no
                      << " has too many items or is not terminated with an "
                      << "empty string";
        }
        return true;
    }

    static std::string const& get_full_qualified_name()
    {
        return our_name_;
    }

    static std::string get_name()
    {
        std::string::size_type idx = our_name_.find_last_of(":");
        if ( idx == std::string::npos )
        {
            return our_name_;
        } else {
            return our_name_.substr( idx + 1 );
        }
    }

private:
    ENUM value_;
    static const char ** our_strings_ ;
    static std::string our_name_ ;
    static bool  our_verified_flag_;
};

/** ostream operator for enumeration
 * @relates mapnik::enumeration
 */
template <class ENUM, int THE_MAX>
std::ostream &
operator<<(std::ostream & os, const mapnik::enumeration<ENUM, THE_MAX> & e)
{
    e.print( os );
    return os;
}

/** istream operator for enumeration
 * @relates mapnik::enumeration
 */
template <class ENUM, int THE_MAX>
std::istream &
operator>>(std::istream & is, mapnik::enumeration<ENUM, THE_MAX> & e)
{
    e.parse( is );
    return is;
}

} // end of namespace

/** Helper macro.
 * @relates mapnik::enumeration
 */
#ifdef _MSC_VER
#define DEFINE_ENUM( name, e)                   \
    template enumeration<e, e ## _MAX>;         \
    using name = enumeration<e, e ## _MAX>;
#else
#define DEFINE_ENUM( name, e)                   \
    using name = enumeration<e, e ## _MAX>;
#endif

/** Helper macro. Runs the verify_mapnik_enum() method during static initialization.
 * @relates mapnik::enumeration
 */

#define IMPLEMENT_ENUM( name, strings )                                 \
    template <> MAPNIK_DECL const char ** name ::our_strings_ = strings;            \
    template <> MAPNIK_DECL std::string name ::our_name_ = #name;                   \
    template <> MAPNIK_DECL bool name ::our_verified_flag_( name ::verify_mapnik_enum(__FILE__, __LINE__));

#endif // MAPNIK_ENUMERATION_HPP
