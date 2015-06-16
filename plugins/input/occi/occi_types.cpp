/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#include "occi_types.hpp"

std::string occi_enums::resolve_gtype(int gtype)
{
    switch (gtype)
    {
    case SDO_GTYPE_UNKNOWN:        return "SDO_GTYPE_UNKNOWN";
    case SDO_GTYPE_POINT:          return "SDO_GTYPE_POINT";
    case SDO_GTYPE_LINE:           return "SDO_GTYPE_LINE";
    case SDO_GTYPE_POLYGON:        return "SDO_GTYPE_POLYGON";
    case SDO_GTYPE_MULTIPOINT:     return "SDO_GTYPE_MULTIPOINT";
    case SDO_GTYPE_MULTILINE:      return "SDO_GTYPE_MULTILINE";
    case SDO_GTYPE_MULTIPOLYGON:   return "SDO_GTYPE_MULTIPOLYGON";
    case SDO_GTYPE_COLLECTION:     return "SDO_GTYPE_COLLECTION";
    default:                       return "<unknown SDO_GTYPE>";
    }
}

std::string occi_enums::resolve_etype(int etype)
{
    switch (etype)
    {
    case SDO_ETYPE_UNKNOWN:                   return "SDO_ETYPE_UNKNOWN";
    case SDO_ETYPE_POINT:                     return "SDO_ETYPE_POINT";
    case SDO_ETYPE_LINESTRING:                return "SDO_ETYPE_LINESTRING";
    case SDO_ETYPE_POLYGON:                   return "SDO_ETYPE_POLYGON";
    case SDO_ETYPE_POLYGON_INTERIOR:          return "SDO_ETYPE_POLYGON_INTERIOR";
    case SDO_ETYPE_COMPOUND_LINESTRING:       return "SDO_ETYPE_COMPOUND_LINESTRING";
    case SDO_ETYPE_COMPOUND_POLYGON:          return "SDO_ETYPE_COMPOUND_POLYGON";
    case SDO_ETYPE_COMPOUND_POLYGON_INTERIOR: return "SDO_ETYPE_COMPOUND_POLYGON_INTERIOR";
    default:                                  return "<unknown SDO_ETYPE>";
    }
}

std::string occi_enums::resolve_datatype(int type_id)
{
    switch (type_id)
    {
    case oracle::occi::OCCIINT:                 return "OCCIINT";
    case oracle::occi::OCCIUNSIGNED_INT:        return "OCCIUNSIGNED_INT";
    case oracle::occi::OCCIFLOAT:               return "OCCIFLOAT";
    case oracle::occi::OCCIBFLOAT:              return "OCCIBFLOAT";
    case oracle::occi::OCCIDOUBLE:              return "OCCIDOUBLE";
    case oracle::occi::OCCIBDOUBLE:             return "OCCIBDOUBLE";
    case oracle::occi::OCCINUMBER:              return "OCCINUMBER";
    case oracle::occi::OCCI_SQLT_NUM:           return "OCCI_SQLT_NUM";
    case oracle::occi::OCCICHAR:                return "OCCICHAR";
    case oracle::occi::OCCISTRING:              return "OCCISTRING";
    case oracle::occi::OCCI_SQLT_AFC:           return "OCCI_SQLT_AFC";
    case oracle::occi::OCCI_SQLT_AVC:           return "OCCI_SQLT_AVC";
    case oracle::occi::OCCI_SQLT_CHR:           return "OCCI_SQLT_CHR";
    case oracle::occi::OCCI_SQLT_LVC:           return "OCCI_SQLT_LVC";
    case oracle::occi::OCCI_SQLT_LNG:           return "OCCI_SQLT_LNG";
    case oracle::occi::OCCI_SQLT_STR:           return "OCCI_SQLT_STR";
    case oracle::occi::OCCI_SQLT_VCS:           return "OCCI_SQLT_VCS";
    case oracle::occi::OCCI_SQLT_VNU:           return "OCCI_SQLT_VNU";
    case oracle::occi::OCCI_SQLT_VBI:           return "OCCI_SQLT_VBI";
    case oracle::occi::OCCI_SQLT_VST:           return "OCCI_SQLT_VST";
    case oracle::occi::OCCI_SQLT_RDD:           return "OCCI_SQLT_RDD";
    case oracle::occi::OCCIDATE:                return "OCCIDATE";
    case oracle::occi::OCCITIMESTAMP:           return "OCCITIMESTAMP";
    case oracle::occi::OCCI_SQLT_DAT:           return "OCCI_SQLT_DAT";
    case oracle::occi::OCCI_SQLT_TIMESTAMP:     return "OCCI_SQLT_TIMESTAMP";
    case oracle::occi::OCCI_SQLT_TIMESTAMP_LTZ: return "OCCI_SQLT_TIMESTAMP_LTZ";
    case oracle::occi::OCCI_SQLT_TIMESTAMP_TZ:  return "OCCI_SQLT_TIMESTAMP_TZ";
    case oracle::occi::OCCIPOBJECT:             return "OCCIPOBJECT";
    default:                                    return "<unknown ATTR_DATA_TYPE>";
    }
}
