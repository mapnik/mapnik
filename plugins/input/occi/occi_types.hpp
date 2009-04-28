/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2007 Artem Pavlenko
 *
 * This library is free software, you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library, if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/
//$Id$

#ifndef OCCI_TYPES_HPP
#define OCCI_TYPES_HPP

// mapnik
#include <mapnik/utils.hpp>

// occi
#include <occi.h>

// ott generated SDOGeometry classes
#include "spatial_classesh.h"
#include "spatial_classesm.h"

// check for oracle support
#if OCCI_MAJOR_VERSION == 10 && OCCI_MINOR_VERSION >= 1
  //     Only ORACLE 10g (>= 10.2.0.X) is supported !
#else
  #error Only ORACLE 10g (>= 10.2.0.X) is supported !
#endif


#define SDO_GEOMETRY_METADATA_TABLE     "ALL_SDO_GEOM_METADATA"


enum
{
    SDO_GTYPE_UNKNOWN                   = 0,
    SDO_GTYPE_POINT                     = 1,
    SDO_GTYPE_LINE                      = 2,
    SDO_GTYPE_POLYGON                   = 3,
    SDO_GTYPE_COLLECTION                = 4,
    SDO_GTYPE_MULTIPOINT                = 5,
    SDO_GTYPE_MULTILINE                 = 6,
    SDO_GTYPE_MULTIPOLYGON              = 7,

    SDO_GTYPE_2DPOINT                   = 2001,
    SDO_GTYPE_2DLINE                    = 2002,
    SDO_GTYPE_2DPOLYGON                 = 2003,
    SDO_GTYPE_2DMULTIPOINT              = 2005,
    SDO_GTYPE_2DMULTILINE               = 2006,
    SDO_GTYPE_2DMULTIPOLYGON            = 2007,

    SDO_ELEM_INFO_SIZE                  = 3,

    SDO_ETYPE_UNKNOWN                   = 0,
    SDO_ETYPE_POINT                     = 1,
    SDO_ETYPE_LINESTRING                = 2,
    SDO_ETYPE_POLYGON                   = 1003,
    SDO_ETYPE_POLYGON_INTERIOR          = 2003,
    SDO_ETYPE_COMPOUND_LINESTRING       = 4,
    SDO_ETYPE_COMPOUND_POLYGON          = 1005,
    SDO_ETYPE_COMPOUND_POLYGON_INTERIOR = 2005,

    SDO_INTERPRETATION_POINT            = 1,
    SDO_INTERPRETATION_RECTANGLE        = 3,
    SDO_INTERPRETATION_CIRCLE           = 4,
    SDO_INTERPRETATION_STRAIGHT         = 1,
    SDO_INTERPRETATION_CIRCULAR         = 2
};


class occi_environment : public mapnik::singleton<occi_environment,mapnik::CreateStatic>
{
    friend class mapnik::CreateStatic<occi_environment>;

public:

    static oracle::occi::Environment* get_environment ()
    {
        if (env_ == 0)
        {
#ifdef MAPNIK_DEBUG
            std::clog << "occi_environment constructor" << std::endl;
#endif

            int mode = oracle::occi::Environment::OBJECT
                     | oracle::occi::Environment::THREADED_MUTEXED;

            env_ = oracle::occi::Environment::createEnvironment ((oracle::occi::Environment::Mode) mode);
            RegisterClasses (env_);
        }

        return env_;
    }

private:

    occi_environment()
    {
    }

    ~occi_environment()
    {
        if (env_)
        {
#ifdef MAPNIK_DEBUG
            std::clog << "occi_environment destructor" << std::endl;
#endif

            oracle::occi::Environment::terminateEnvironment (env_);
            env_ = 0;
        }
    }

    static oracle::occi::Environment* env_;
};


class occi_connection_ptr
{
public:
    occi_connection_ptr (oracle::occi::StatelessConnectionPool* pool)
        : pool_ (pool),
          conn_ (pool->getConnection ()),
          stmt_ (0),
          rs_ (0)
    {
    }
    
    ~occi_connection_ptr ()
    {
        close_query (true);
    }

    oracle::occi::ResultSet* execute_query (const std::string& s, const unsigned prefetch = 0)
    {
        close_query (false);

        stmt_ = conn_->createStatement (s);

        if (prefetch > 0)
        {
            stmt_->setPrefetchMemorySize (0);
            stmt_->setPrefetchRowCount (prefetch);
        }

        rs_ = stmt_->executeQuery ();
        
        return rs_;
    }

    oracle::occi::Connection* operator*()
    {
        return conn_;
    }

private:

    void close_query (const bool release_connection)
    {
        if (conn_)
        {
            if (stmt_)
            {
                if (rs_)
                { 
                    stmt_->closeResultSet (rs_);
                    rs_ = 0;
                }

                conn_->terminateStatement (stmt_);
                stmt_ = 0;
            }
            
            if (release_connection)
            {
                pool_->releaseConnection (conn_);
                conn_ = 0;
            }
        }
    }

    oracle::occi::StatelessConnectionPool* pool_;
    oracle::occi::Connection* conn_;
    oracle::occi::Statement* stmt_;
    oracle::occi::ResultSet* rs_;
};

#endif // OCCI_TYPES_HPP
