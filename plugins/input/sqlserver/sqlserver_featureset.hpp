/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 we-do-IT
 *
 *****************************************************************************/

#ifndef SQLSERVER_FEATURESET_HPP
#define SQLSERVER_FEATURESET_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/attribute_descriptor.hpp>

// boost
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

// sql server (via odbc)
#ifdef _WINDOWS
#include <windows.h>
#endif
#include "sql.h"

#include <utility>
#include <vector>

class sqlserver_featureset : public mapnik::Featureset
{
public:
    sqlserver_featureset(SQLHDBC hdbc,
                         std::string const& sqlstring,
                         mapnik::layer_descriptor const& desc);
    virtual ~sqlserver_featureset();
    mapnik::feature_ptr next();

private:
    SQLHANDLE hstmt_;
    mapnik::layer_descriptor desc_;
    boost::scoped_ptr<mapnik::transcoder> tr_;
    mapnik::value_integer feature_id_;
    mapnik::context_ptr ctx_;
};

#endif // SQLSERVER_FEATURESET_HPP
