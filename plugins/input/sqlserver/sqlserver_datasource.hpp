/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 we-do-IT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef SQLSERVER_DATASOURCE_HPP
#define SQLSERVER_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/value_types.hpp>

// boost
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

// stl
#include <vector>
#include <string>

// sql server (via odbc)
#ifdef _WINDOWS
#include <windows.h>
#endif
#include "sql.h"

// extended from mapnik/attribute_descriptor.hpp (added 8,9)
namespace mapnik {
    namespace sqlserver {
        enum eAttributeTypeEx {
            Integer=1,
            Float=2,
            Double=3,
            String=4,
            Boolean=5,
            Geometry=6,
            Object=7,
            Geography=8,
            Unknown=9
        };
    }
}

// SQL Server datasource for Mapnik
class sqlserver_datasource : public mapnik::datasource
{
public:
    sqlserver_datasource(mapnik::parameters const& params);
    virtual ~sqlserver_datasource ();
    static const char * name();
    mapnik::datasource::datasource_t type() const;
    mapnik::featureset_ptr features(mapnik::query const& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    boost::optional<mapnik::datasource_geometry_t> get_geometry_type() const;
    mapnik::layer_descriptor get_descriptor() const;

private:
    mapnik::datasource::datasource_t type_;
    
    std::string table_;
    std::string fields_;
    std::string geometry_field_;
    bool is_geometry_; // true=geometry, false=geography

    mutable bool extent_initialized_;
    mutable mapnik::box2d<double> extent_;
    mutable int srid_;

    mapnik::layer_descriptor desc_;

    SQLHENV henv_;
    SQLHDBC hdbc_;
    
    mapnik::featureset_ptr features_in_box(mapnik::box2d<double> const& box) const;
};

// if anything goes wrong in the sqlserver_datasource class, one of these
// exceptions will be thrown. usually the message will include details from
// the SQL Server error
class sqlserver_datasource_exception : public mapnik::datasource_exception
{
public:
    sqlserver_datasource_exception(std::string const& message);
    sqlserver_datasource_exception(std::string const& message, SQLSMALLINT HandleType, SQLHANDLE Handle);
    
    virtual ~sqlserver_datasource_exception() throw();
    
private:
    static std::string sql_diagnostics(SQLSMALLINT HandleType, SQLHANDLE Handle);
};

#endif // SQLSERVER_DATASOURCE_HPP
