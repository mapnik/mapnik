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
#ifndef MAPNIK_PYTHON_BINDING_DATASOURCE_INCLUDED
#define MAPNIK_PYTHON_BINDING_DATASOURCE_INCLUDED

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>

// boost
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>

namespace mapnik
{

class python_datasource : public datasource
{
public:
    // constructor
    // arguments must not change
    python_datasource(boost::python::object ds);

    // destructor
    virtual ~python_datasource ();

    // mandatory: type of the plugin, used to match at runtime
    datasource::datasource_t type() const;

    // mandatory: function to query features by box2d
    // this is called when rendering, specifically in feature_style_processor.hpp
    featureset_ptr features(query const& q) const;

    // mandatory: function to query features by point (coord2d)
    // not used by rendering, but available to calling applications
    featureset_ptr features_at_point(coord2d const& pt, double tol = 0) const;

    // mandatory: return the box2d of the datasource
    // called during rendering to determine if the layer should be processed
    box2d<double> envelope() const;

    // mandatory: optionally return the overal geometry type of the datasource
    boost::optional<datasource::geometry_t> get_geometry_type() const;

    // mandatory: return the layer descriptor
    layer_descriptor get_descriptor() const;

private:
    layer_descriptor desc_;
    const std::string factory_;
    std::map<std::string, std::string> kwargs_;
    boost::python::object datasource_;
};

class python_featureset : public Featureset
{
public:
    // this constructor can have any arguments you need
    python_featureset(boost::python::object iterator);

    // desctructor
    virtual ~python_featureset();

    // mandatory: you must expose a next() method, called when rendering
    feature_ptr next();

private:
    typedef boost::python::stl_input_iterator<feature_ptr> feature_iter;

    feature_iter begin_, end_;
};

} // end of namespace mapnik

#endif // MAPNIK_PYTHON_BINDING_DATASOURCE_INCLUDED
