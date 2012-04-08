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
#include <mapnik/ctrans.hpp>
#include <mapnik/params.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature_layer_desc.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

// stl
#include <map>
#include <string>

namespace mapnik {

typedef MAPNIK_DECL boost::shared_ptr<Feature> feature_ptr;

struct MAPNIK_DECL Featureset : private boost::noncopyable
{
    virtual feature_ptr next() = 0;
    virtual ~Featureset() {}
};

typedef MAPNIK_DECL boost::shared_ptr<Featureset> featureset_ptr;

class MAPNIK_DECL datasource_exception : public std::exception
{
public:
    datasource_exception(const std::string& message = std::string("no reason"))
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

class MAPNIK_DECL datasource : private boost::noncopyable
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
      : params_(params),
        log_enabled_(false),
        is_bound_(false)
    {
    }

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

    /*!
     * @brief Get the status of detasource logging
     * @return Return true if log is enabled, false otherwise
     */
    bool log_enabled() const
    {
        return log_enabled_;
    }

    /*!
     * @brief Get the type of the datasource
     * @return The type of the datasource (Vector or Raster)
     */
    virtual datasource_t type() const = 0;

    /*!
     * @brief Connect to the datasource
     */
    virtual void bind() const {}

    virtual featureset_ptr features(const query& q) const = 0;
    virtual featureset_ptr features_at_point(coord2d const& pt) const = 0;
    virtual box2d<double> envelope() const = 0;
    virtual boost::optional<geometry_t> get_geometry_type() const = 0;
    virtual layer_descriptor get_descriptor() const = 0;
    virtual ~datasource() {}
protected:
    parameters params_;
    bool log_enabled_;
    mutable bool is_bound_;
};

typedef std::string datasource_name();
typedef datasource* create_ds(const parameters& params, bool bind);
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

#define DATASOURCE_PLUGIN(classname)                                    \
    extern "C" MAPNIK_EXP std::string datasource_name()                 \
    {                                                                   \
        return classname::name();                                       \
    }                                                                   \
    extern "C"  MAPNIK_EXP datasource* create(const parameters &params, bool bind) \
    {                                                                   \
        return new classname(params, bind);                             \
    }                                                                   \
    extern "C" MAPNIK_EXP void destroy(datasource *ds)                  \
    {                                                                   \
        delete ds;                                                      \
    }

}

#endif // MAPNIK_DATASOURCE_HPP
