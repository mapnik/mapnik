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

#ifndef MAPNIK_METAWRITER_PROPERTIES_HPP
#define MAPNIK_METAWRITER_PROPERTIES_HPP

// stl
#include <set>
#include <string>

// ICU
#include <unicode/unistr.h>

// boost
#include <boost/optional.hpp>

namespace mapnik {

/** All properties to be output by a metawriter. */
class metawriter_properties : public std::set<std::string>
{
public:
    metawriter_properties(boost::optional<std::string> str);
    metawriter_properties() {}
    template <class InputIterator> metawriter_properties(
        InputIterator first, InputIterator last) : std::set<std::string>(first, last) {}
    std::string to_string() const;
};

class metawriter;
typedef boost::shared_ptr<metawriter> metawriter_ptr;
typedef std::pair<metawriter_ptr, metawriter_properties> metawriter_with_properties;

/** Implementation of std::map that also returns const& for operator[]. */
class metawriter_property_map
{
public:
    typedef std::map<std::string, UnicodeString> property_map;
    typedef property_map::const_iterator const_iterator;

    metawriter_property_map() :
        m_(),
        not_found_() {}

    UnicodeString const& operator[](std::string const& key) const;
    UnicodeString& operator[](std::string const& key) {return m_[key];}

    std::map<std::string, UnicodeString>::const_iterator find(std::string const& key) const
    {
        return m_.find(key);
    }

    std::map<std::string, UnicodeString>::const_iterator end() const
    {
        return m_.end();
    }

    UnicodeString const& get(std::string const& key) const
    {
        return (*this)[key];
    }

private:
    property_map m_;
    UnicodeString not_found_;
};

}

#endif
