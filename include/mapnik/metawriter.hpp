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

#ifndef MAPNIK_METAWRITER_HPP
#define MAPNIK_METAWRITER_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/projection.hpp>
// boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/concept_check.hpp>
#include <boost/variant.hpp>
// stl
#include <set>
#include <string>


namespace mapnik {

class text_placement_info;
class text_path;

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


// All properties to be output by a metawriter.
class metawriter_properties : public std::set<std::string>
{
public:
    metawriter_properties(boost::optional<std::string> str);
    metawriter_properties() {}
    template <class InputIterator> metawriter_properties(
        InputIterator first, InputIterator last) : std::set<std::string>(first, last) {}
    std::string to_string() const;
};

// Abstract baseclass for all metawriter classes.
class metawriter_base
{
public:    
    explicit metawriter_base(metawriter_properties dflt_properties) :
        dflt_properties_(dflt_properties),
        width_(0),
        height_(0) {}
    
    void set_size(int width, int height) { width_ = width; height_ = height; }    
    metawriter_properties const& get_default_properties() const { return dflt_properties_;}
protected:
    metawriter_properties dflt_properties_;
    int width_;
    int height_;
};

}

#endif // MAPNIK_METAWRITER_HPP
