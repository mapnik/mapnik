/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Hermann Kraus
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


#ifndef METAWRITER_HPP
#define METAWRITER_HPP

// Mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/feature.hpp>

// Boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

// STL
#include <set>
#include <string>


namespace mapnik {

class metawriter_property_map
{
public:
    metawriter_property_map() {}
    UnicodeString const& operator[](std::string const& key) const;
    UnicodeString& operator[](std::string const& key) {return m_[key];}
private:
    std::map<std::string, UnicodeString> m_;
    UnicodeString not_found_;
};


/** All properties to be output by a metawriter. */
class metawriter_properties : public std::set<std::string>
{
    public:
        metawriter_properties(boost::optional<std::string> str);
        metawriter_properties() {};
        template <class InputIterator> metawriter_properties(
                InputIterator first, InputIterator last) : std::set<std::string>(first, last) {};
};

/** Abstract baseclass for all metawriter classes. */
class metawriter
{
    public:
        metawriter(metawriter_properties dflt_properties) : dflt_properties_(dflt_properties) {}
        virtual ~metawriter() {};
        /** Output a rectangular area.
          * \param box Area (in pixel coordinates)
          * \param feature The feature being processed
          * \param prj_trans Projection transformation
          * \param t Cooridnate transformation
          * \param properties List of properties to output
          */
        virtual void add_box(box2d<double> box, Feature const &feature,
                             CoordTransform const &t,
                             metawriter_properties const& properties = metawriter_properties())=0;
        virtual void start(metawriter_property_map const& properties) {};
        void set_size(int width, int height) { width_ = width; height_ = height; }
        virtual void set_map_srs(projection const& proj) = 0;
        virtual void stop() {};
        metawriter_properties const& get_default_properties() const { return dflt_properties_;}
    protected:
        metawriter_properties dflt_properties_;
        int width_;
        int height_;
};

typedef boost::shared_ptr<metawriter> metawriter_ptr;
typedef std::pair<metawriter_ptr, metawriter_properties> metawriter_with_properties;

};

#endif
