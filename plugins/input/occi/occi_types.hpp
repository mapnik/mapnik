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

// main OCCI include
#include <occi.h>

// OTT generated SDOGeometry classes
#include "spatial_classesh.h"
#include "spatial_classesm.h"

#if OCCI_MAJOR_VERSION == 10 && OCCI_MINOR_VERSION >= 1
  //     Support ORACLE 10g (>= 10.2.0.X)
#else
  #error    Only ORACLE 10g (>= 10.2.0.X) is supported !
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

#endif // OCCI_TYPES_HPP
