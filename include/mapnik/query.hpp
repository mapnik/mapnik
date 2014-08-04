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

#ifndef MAPNIK_QUERY_HPP
#define MAPNIK_QUERY_HPP

//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/attribute.hpp>

// stl
#include <set>
#include <string>
#include <tuple>

namespace mapnik {

class query
{
public:
    using resolution_type = std::tuple<double,double>;

    query(box2d<double> const& bbox,
          resolution_type const& resolution,
          double scale_denominator,
          box2d<double> const& unbuffered_bbox)
        : bbox_(bbox),
          resolution_(resolution),
          scale_denominator_(scale_denominator),
          filter_factor_(1.0),
          unbuffered_bbox_(unbuffered_bbox),
          names_(),
          vars_()
    {}

    query(box2d<double> const& bbox,
          resolution_type const& resolution,
          double scale_denominator = 1.0)
        : bbox_(bbox),
          resolution_(resolution),
          scale_denominator_(scale_denominator),
          filter_factor_(1.0),
          unbuffered_bbox_(bbox),
          names_(),
          vars_()
    {}

    query(box2d<double> const& bbox)
        : bbox_(bbox),
          resolution_(resolution_type(1.0,1.0)),
          scale_denominator_(1.0),
          filter_factor_(1.0),
          unbuffered_bbox_(bbox),
          names_(),
          vars_()
    {}

    query(query const& other)
        : bbox_(other.bbox_),
          resolution_(other.resolution_),
          scale_denominator_(other.scale_denominator_),
          filter_factor_(other.filter_factor_),
          unbuffered_bbox_(other.unbuffered_bbox_),
          names_(other.names_),
          vars_(other.vars_)
    {}

    query& operator=(query const& other)
    {
        if (this == &other) return *this;
        bbox_=other.bbox_;
        resolution_=other.resolution_;
        scale_denominator_=other.scale_denominator_;
        filter_factor_=other.filter_factor_;
        unbuffered_bbox_=other.unbuffered_bbox_;
        names_=other.names_;
        vars_=other.vars_;
        return *this;
    }

    query::resolution_type const& resolution() const
    {
        return resolution_;
    }

    double scale_denominator() const
    {
        return scale_denominator_;
    }

    box2d<double> const& get_bbox() const
    {
        return bbox_;
    }

    box2d<double> const& get_unbuffered_bbox() const
    {
        return unbuffered_bbox_;
    }

    void set_unbuffered_bbox(box2d<double> const& bbox)
    {
        unbuffered_bbox_ = bbox;
    }

    void set_bbox(box2d<double> const& bbox)
    {
        bbox_ = bbox;
    }

    double get_filter_factor() const
    {
        return filter_factor_;
    }

    void set_filter_factor(double factor)
    {
        filter_factor_ = factor;
    }

    void add_property_name(std::string const& name)
    {
        names_.insert(name);
    }

    std::set<std::string> const& property_names() const
    {
        return names_;
    }

    void set_variables(attributes const& vars)
    {
        vars_ = vars;
    }

    attributes const& variables() const
    {
        return vars_;
    }

private:
    box2d<double> bbox_;
    resolution_type resolution_;
    double scale_denominator_;
    double filter_factor_;
    box2d<double> unbuffered_bbox_;
    std::set<std::string> names_;
    attributes vars_;
};

}

#endif // MAPNIK_QUERY_HPP
