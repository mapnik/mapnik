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
