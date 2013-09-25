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

#ifndef MAPNIK_DATASOURCE_HPP
#define MAPNIK_DATASOURCE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/params.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/feature_style_processor_context.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

// stl
#include <map>
#include <string>

namespace mapnik {

struct MAPNIK_DECL Featureset : private mapnik::noncopyable
{
    virtual feature_ptr next() = 0;
    virtual ~Featureset() {}
};

typedef boost::shared_ptr<Featureset> featureset_ptr;

class MAPNIK_DECL datasource_exception : public std::exception
{
public:
    datasource_exception(std::string const& message)
      : message_(message)
    {
    }

    ~datasource_exception() throw()
    {
    }

    virtual const char* what() const throw()
    {
        return message_.c_str();
    }
private:
    std::string message_;
};

class MAPNIK_DECL datasource : private mapnik::noncopyable
{
public:
    enum datasource_t {
        Vector,
        Raster
    };

    enum geometry_t {
        Point = 1,
        LineString = 2,
        Polygon = 3,
        Collection = 4
    };

    datasource (parameters const& params)
       : params_(params) {}

    /*!
     * @brief Get the configuration parameters of the data source.
     *
     * These vary depending on the type of data source.
     *
     * @return The configuration parameters of the data source.
     */
    parameters const& params() const
    {
        return params_;
    }

    parameters & params()
    {
        return params_;
    }

    /*!
     * @brief Get the type of the datasource
     * @return The type of the datasource (Vector or Raster)
     */
    virtual datasource_t type() const = 0;

    virtual processor_context_ptr get_context(feature_style_context_map&) const { return processor_context_ptr(); }
    virtual featureset_ptr features_with_context(const query& q,processor_context_ptr ctx= processor_context_ptr()) const
    {
        // default implementation without context use features method
        return features(q);
    }
    virtual featureset_ptr features(query const& q) const = 0;
    virtual featureset_ptr features_at_point(coord2d const& pt, double tol = 0) const = 0;
    virtual box2d<double> envelope() const = 0;
    virtual boost::optional<geometry_t> get_geometry_type() const = 0;
    virtual layer_descriptor get_descriptor() const = 0;
    virtual ~datasource() {}
protected:
    parameters params_;
};

typedef const char * datasource_name();
typedef datasource* create_ds(parameters const& params);
typedef void destroy_ds(datasource *ds);

class datasource_deleter
{
public:
    void operator() (datasource* ds)
    {
        delete ds;
    }
};

typedef boost::shared_ptr<datasource> datasource_ptr;

#ifdef MAPNIK_STATIC_PLUGINS
    #define DATASOURCE_PLUGIN(classname)
#else
    #define DATASOURCE_PLUGIN(classname)                                    \
        extern "C" MAPNIK_EXP const char * datasource_name()                \
        {                                                                   \
            return classname::name();                                       \
        }                                                                   \
        extern "C"  MAPNIK_EXP datasource* create(parameters const& params) \
        {                                                                   \
            return new classname(params);                                   \
        }                                                                   \
        extern "C" MAPNIK_EXP void destroy(datasource *ds)                  \
        {                                                                   \
            delete ds;                                                      \
        }
#endif

}

#endif // MAPNIK_DATASOURCE_HPP
