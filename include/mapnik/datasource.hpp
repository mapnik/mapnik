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

#ifndef MAPNIK_DATASOURCE_HPP
#define MAPNIK_DATASOURCE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/params.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/query.hpp>
#include <mapnik/featureset.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/feature_style_processor_context.hpp>
#include <mapnik/datasource_geometry_type.hpp>

// stl
#include <map>
#include <string>
#include <memory>

namespace mapnik {

class MAPNIK_DECL datasource_exception : public std::exception
{
  public:
    datasource_exception(std::string const& message)
        : message_(message)
    {}

    ~datasource_exception() {}

    virtual const char* what() const noexcept { return message_.c_str(); }

  private:
    std::string message_;
};

class MAPNIK_DECL datasource : private util::noncopyable
{
  public:
    enum datasource_t : std::uint8_t { Vector, Raster };

    datasource(parameters const& _params)
        : params_(_params)
    {}

    /*!
     * @brief Get the configuration parameters of the data source.
     *
     * These vary depending on the type of data source.
     *
     * @return The configuration parameters of the data source.
     */
    parameters const& params() const { return params_; }

    parameters& params() { return params_; }

    bool operator==(datasource const& rhs) const { return params_ == rhs.params(); }

    bool operator!=(datasource const& rhs) const { return !(*this == rhs); }

    /*!
     * @brief Get the type of the datasource
     * @return The type of the datasource (Vector or Raster)
     */
    virtual datasource_t type() const = 0;
    virtual processor_context_ptr get_context(feature_style_context_map&) const { return processor_context_ptr(); }
    virtual featureset_ptr features_with_context(query const& q, processor_context_ptr /*ctx*/) const
    {
        // default implementation without context use features method
        return features(q);
    }
    virtual boost::optional<datasource_geometry_t> get_geometry_type() const = 0;
    virtual featureset_ptr features(query const& q) const = 0;
    virtual featureset_ptr features_at_point(coord2d const& pt, double tol = 0) const = 0;
    virtual box2d<double> envelope() const = 0;
    virtual layer_descriptor get_descriptor() const = 0;
    virtual ~datasource() {}

  protected:
    parameters params_;
};

using datasource_name = const char* (*)();
using create_ds = datasource* (*)(parameters const&);
using destroy_ds = void (*)(datasource*);

class datasource_deleter
{
  public:
    void operator()(datasource* ds) { delete ds; }
};

using datasource_ptr = std::shared_ptr<datasource>;

#ifdef MAPNIK_STATIC_PLUGINS
#define DATASOURCE_PLUGIN(classname)
#else
#define DATASOURCE_PLUGIN(classname)                                                                                   \
    extern "C" MAPNIK_EXP const char* datasource_name()                                                                \
    {                                                                                                                  \
        return classname::name();                                                                                      \
    }                                                                                                                  \
    extern "C" MAPNIK_EXP datasource* create(parameters const& params)                                                 \
    {                                                                                                                  \
        return new classname(params);                                                                                  \
    }                                                                                                                  \
    extern "C" MAPNIK_EXP void destroy(datasource* ds)                                                                 \
    {                                                                                                                  \
        delete ds;                                                                                                     \
    }
#endif

} // namespace mapnik

#endif // MAPNIK_DATASOURCE_HPP
