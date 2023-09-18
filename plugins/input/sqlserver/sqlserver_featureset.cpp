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

#include "sqlserver_featureset.hpp"
#include "sqlserver_datasource.hpp"
#include "sqlserver_geometry_parser.hpp"

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/feature_factory.hpp>

// sql server (odbc)
#include <sqlext.h>
#include <msodbcsql.h>

using mapnik::query;
using mapnik::box2d;
using mapnik::feature_ptr;
using mapnik::geometry_utils;
using mapnik::transcoder;
using mapnik::feature_factory;
using mapnik::attribute_descriptor;

sqlserver_featureset::sqlserver_featureset(SQLHDBC hdbc,
                                 std::string const& sqlstring,
                                 mapnik::layer_descriptor const& desc
                                 )
    : hstmt_(0),
      desc_(desc),
      tr_(new transcoder(desc.get_encoding())),
      feature_id_(1)
{
    SQLRETURN retcode;
    
    // allocate statement handle
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt_);
    if (!SQL_SUCCEEDED(retcode)) {
        throw sqlserver_datasource_exception("could not allocate statement", SQL_HANDLE_DBC, hdbc);
    }
    
    // execute statement
    retcode = SQLExecDirect(hstmt_, (SQLCHAR*)sqlstring.c_str(), SQL_NTS);
    if (!SQL_SUCCEEDED(retcode)) {
        throw sqlserver_datasource_exception("could not execute statement", SQL_HANDLE_STMT, hstmt_);
    }
    
    std::vector<attribute_descriptor>::const_iterator itr = desc_.get_descriptors().begin();
    std::vector<attribute_descriptor>::const_iterator end = desc_.get_descriptors().end();
    ctx_ = std::make_shared<mapnik::context_type>();
    while (itr != end) {
        ctx_->push(itr->get_name());
        ++itr;
    }
    

}

sqlserver_featureset::~sqlserver_featureset() {
    if (hstmt_) {
        (void)SQLFreeStmt(hstmt_, SQL_CLOSE);
        hstmt_ = 0;
    }
}

feature_ptr sqlserver_featureset::next()
{
    SQLRETURN retcode;
    
    // fetch next result
    retcode = SQLFetch(hstmt_);
    if (retcode == SQL_NO_DATA) {
        // normal end of recordset
        return feature_ptr();
    }
    if (!SQL_SUCCEEDED(retcode)) {
        throw sqlserver_datasource_exception("could not fetch result", SQL_HANDLE_STMT, hstmt_);
    }
   
    // create an empty feature with the next id
    feature_ptr feature(feature_factory::create(ctx_, feature_id_));

    // populate feature geometry and attributes from this row
    std::vector<attribute_descriptor>::const_iterator itr = desc_.get_descriptors().begin();
    std::vector<attribute_descriptor>::const_iterator end = desc_.get_descriptors().end();
    SQLUSMALLINT ColumnNum=1;
    while (itr != end) {
        SQLCHAR sval[2048];
        long ival;
        double dval;
        SQLCHAR *BinaryPtr = NULL;    // Allocate dynamically
        SQLLEN BinaryLenOrInd;
        SQLLEN LenOrInd;
        switch (itr->get_type()) {
            case mapnik::sqlserver::String:
                retcode = SQLGetData(hstmt_, ColumnNum, SQL_C_CHAR, sval, sizeof(sval), &LenOrInd);
                if (!SQL_SUCCEEDED(retcode)) {
                    throw sqlserver_datasource_exception("could not get string data", SQL_HANDLE_STMT, hstmt_);
                }
                feature->put(itr->get_name(), (UnicodeString)tr_->transcode((char*)sval));
                break;
                
            case mapnik::sqlserver::Integer:
                retcode = SQLGetData(hstmt_, ColumnNum, SQL_C_SLONG, &ival, sizeof(ival), &LenOrInd);
                if (!SQL_SUCCEEDED(retcode)) {
                    throw sqlserver_datasource_exception("could not get int data", SQL_HANDLE_STMT, hstmt_);
                }
                feature->put(itr->get_name(), static_cast<mapnik::value_integer>(ival));
                break;
                
            case mapnik::sqlserver::Double:
                retcode = SQLGetData(hstmt_, ColumnNum, SQL_C_DOUBLE, &dval, sizeof(dval), &LenOrInd);
                if (!SQL_SUCCEEDED(retcode)) {
                    throw sqlserver_datasource_exception("could not get double data", SQL_HANDLE_STMT, hstmt_);
                }
                feature->put(itr->get_name(), dval);
                break;
    
            case mapnik::sqlserver::Geometry:
            case mapnik::sqlserver::Geography: {
                // Call SQLGetData with a zero buffer size to determine the amount of data that's waiting.
                SQLCHAR DummyBinaryPtr[10];     // cannot pass a NULL pointer, even though we pass length of zero
                retcode = SQLGetData(hstmt_, ColumnNum, SQL_C_BINARY, DummyBinaryPtr, 0, &BinaryLenOrInd);
                if (retcode != SQL_SUCCESS_WITH_INFO) {
                    throw sqlserver_datasource_exception("could not get geometry data - failed to get buffer length", SQL_HANDLE_STMT, hstmt_);
                }

                // allocate a suitably sized buffer
                BinaryPtr = new SQLCHAR[BinaryLenOrInd];

                // get the geometry data
                retcode = SQLGetData(hstmt_, ColumnNum, SQL_C_BINARY, BinaryPtr, BinaryLenOrInd, &BinaryLenOrInd);
                if (!SQL_SUCCEEDED(retcode)) {
					delete[] BinaryPtr;
					BinaryPtr = NULL;
					throw sqlserver_datasource_exception("could not get geometry data into buffer", SQL_HANDLE_STMT, hstmt_);
                }

                // attempt to parse
                try {
                    sqlserver_geometry_parser geometry_parser((itr->get_type() == mapnik::sqlserver::Geometry ? Geometry : Geography));
                    mapnik::geometry::geometry<double> geom = geometry_parser.parse(BinaryPtr, BinaryLenOrInd);
                    feature->set_geometry(std::move(geom));
                } catch (mapnik::datasource_exception e) {
                    // Cleanup and rethrow the caught exception
                    delete[] BinaryPtr;
                    BinaryPtr = NULL;
                    throw;
                }

                // normal cleanup
                delete[] BinaryPtr;
                BinaryPtr = NULL;
                break;
            }

            default:
                MAPNIK_LOG_WARN(sqlserver) << "sqlserver_datasource: unknown/unsupported datatype in column: " << itr->get_name() << " (" << itr->get_type() << ")";
                break;
        }
        ++ColumnNum;
        ++itr;
    }
    ++feature_id_;
    
    return feature;
}

