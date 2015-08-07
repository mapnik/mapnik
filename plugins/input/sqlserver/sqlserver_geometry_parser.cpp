/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * adapted from ogrmssqlgeometryparser.cpp, part of OGR
 * original copyright notice follows
 *
 *****************************************************************************/


/******************************************************************************
 * $Id: ogrmssqlgeometryparser.cpp 24918 2012-09-07 12:02:01Z tamas $
 *
 * Project:  MSSQL Spatial driver
 * Purpose:  Implements ogrmssqlgeometryparser class to parse native SqlGeometries.
 * Author:   Tamas Szekeres, szekerest at gmail.com
 *
 ******************************************************************************
 * Copyright (c) 2010, Tamas Szekeres
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
 ****************************************************************************/

#include "sqlserver_geometry_parser.hpp"
#include "sqlserver_datasource.hpp"

#include <mapnik/datasource.hpp>

class sqlserver_geometry_parser_exception : public sqlserver_datasource_exception
{
public:
    sqlserver_geometry_parser_exception(std::string const& message)
    : sqlserver_datasource_exception("Geometry Parser: "+message) {}
    
    virtual ~sqlserver_geometry_parser_exception() throw() {}
};


/*   SqlGeometry serialization format

Simple Point (SerializationProps & IsSinglePoint)
  [SRID][0x01][SerializationProps][Point][z][m]

Simple Line Segment (SerializationProps & IsSingleLineSegment)
  [SRID][0x01][SerializationProps][Point1][Point2][z1][z2][m1][m2]

Complex Geometries
  [SRID][0x01][SerializationProps][NumPoints][Point1]..[PointN][z1]..[zN][m1]..[mN]
  [NumFigures][Figure]..[Figure][NumShapes][Shape]..[Shape]

SRID
  Spatial Reference Id (4 bytes)

SerializationProps (bitmask) 1 byte
  0x01 = HasZValues
  0x02 = HasMValues
  0x04 = IsValid
  0x08 = IsSinglePoint
  0x10 = IsSingleLineSegment
  0x20 = IsWholeGlobe

Point (2-4)x8 bytes, size depends on SerializationProps & HasZValues & HasMValues
  [x][y]                  - SqlGeometry
  [latitude][longitude]   - SqlGeography

Figure
  [FigureAttribute][PointOffset]

FigureAttribute (1 byte)
  0x00 = Interior Ring
  0x01 = Stroke
  0x02 = Exterior Ring

Shape
  [ParentFigureOffset][FigureOffset][ShapeType]

ShapeType (1 byte)
  0x00 = Unknown
  0x01 = Point
  0x02 = LineString
  0x03 = Polygon
  0x04 = MultiPoint
  0x05 = MultiLineString
  0x06 = MultiPolygon
  0x07 = GeometryCollection

*/

/************************************************************************/
/*                         Geometry parser macros                       */
/************************************************************************/

#define SP_NONE 0
#define SP_HASZVALUES 1
#define SP_HASMVALUES 2
#define SP_ISVALID 4
#define SP_ISSINGLEPOINT 8
#define SP_ISSINGLELINESEGMENT 0x10
#define SP_ISWHOLEGLOBE 0x20

#define ST_UNKNOWN 0
#define ST_POINT 1
#define ST_LINESTRING 2
#define ST_POLYGON 3
#define ST_MULTIPOINT 4
#define ST_MULTILINESTRING 5
#define ST_MULTIPOLYGON 6
#define ST_GEOMETRYCOLLECTION 7

#define ReadInt32(nPos) (*((unsigned int*)(pszData + (nPos))))

#define ReadByte(nPos) (pszData[nPos])

#define ReadDouble(nPos) (*((double*)(pszData + (nPos))))

#define ParentOffset(iShape) (ReadInt32(nShapePos + (iShape) * 9 ))
#define FigureOffset(iShape) (ReadInt32(nShapePos + (iShape) * 9 + 4))
#define ShapeType(iShape) (ReadByte(nShapePos + (iShape) * 9 + 8))

#define NextFigureOffset(iShape) (iShape + 1 < nNumShapes? FigureOffset((iShape) +1) : nNumFigures)

#define FigureAttribute(iFigure) (ReadByte(nFigurePos + (iFigure) * 5))
#define PointOffset(iFigure) (ReadInt32(nFigurePos + (iFigure) * 5 + 1))
#define NextPointOffset(iFigure) (iFigure + 1 < nNumFigures? PointOffset((iFigure) +1) : nNumPoints)

#define ReadX(iPoint) (ReadDouble(nPointPos + 16 * (iPoint)))
#define ReadY(iPoint) (ReadDouble(nPointPos + 16 * (iPoint) + 8))
#define ReadZ(iPoint) (ReadDouble(nPointPos + 16 * nNumPoints + 8 * (iPoint)))
#define ReadM(iPoint) (ReadDouble(nPointPos + 24 * nNumPoints + 8 * (iPoint)))

/************************************************************************/
/*                   sqlserver_geometry_parser()                           */
/************************************************************************/

sqlserver_geometry_parser::sqlserver_geometry_parser(spatial_data_type columnType)
{
    colType = columnType;
}

/************************************************************************/
/*                         ReadPoint()                                  */
/************************************************************************/

mapnik::geometry::point<double> sqlserver_geometry_parser::ReadPoint(int iShape)
{
	mapnik::geometry::point<double> geom;
    int iFigure = FigureOffset(iShape);
    if ( iFigure < nNumFigures ) {
        int iPoint = PointOffset(iFigure);
        if ( iPoint < nNumPoints ) {
            if (colType == Geography) {
				geom.x = ReadY(iPoint);
		        geom.y = ReadX(iPoint);
            } else {
				geom.x = ReadX(iPoint);
				geom.y = ReadY(iPoint);
			}
        }
    }
    return geom;
}

/************************************************************************/
/*                         ReadMultiPoint()                             */
/************************************************************************/

mapnik::geometry::multi_point<double> sqlserver_geometry_parser::ReadMultiPoint(int iShape)
{
	mapnik::geometry::multi_point<double> geom;

    for (int i = iShape + 1; i < nNumShapes; i++) {
        if (ParentOffset(i) == (unsigned int)iShape) {
            if  ( ShapeType(i) == ST_POINT ) {
                geom.emplace_back(ReadPoint(i));
            }
        }
    }

    return geom;
}

/************************************************************************/
/*                         ReadLineString()                             */
/************************************************************************/

mapnik::geometry::line_string<double> sqlserver_geometry_parser::ReadLineString(int iShape)
{
	mapnik::geometry::line_string<double> geom;
    int iFigure = FigureOffset(iShape);

    int iPoint = PointOffset(iFigure);
    int iNextPoint = NextPointOffset(iFigure);
    while (iPoint < iNextPoint) {
        if (colType == Geography) {
			geom.emplace_back(ReadY(iPoint), ReadX(iPoint));
        } else {
			geom.emplace_back(ReadX(iPoint), ReadY(iPoint));
        }
        
        ++iPoint;
    }

    return geom;
}

/************************************************************************/
/*                         ReadMultiLineString()                        */
/************************************************************************/

mapnik::geometry::multi_line_string<double> sqlserver_geometry_parser::ReadMultiLineString(int iShape)
{
	mapnik::geometry::multi_line_string<double> geom;

    for (int i = iShape + 1; i < nNumShapes; i++) {
        if (ParentOffset(i) == (unsigned int)iShape) {
            if ( ShapeType(i) == ST_LINESTRING ) {
                geom.emplace_back(ReadLineString(i));
            }
        }
    }
    
    return geom;
}

/************************************************************************/
/*                         ReadPolygon()                                */
/************************************************************************/

mapnik::geometry::polygon<double> sqlserver_geometry_parser::ReadPolygon(int iShape)
{
	mapnik::geometry::polygon<double> geom;
    int iNextFigure = NextFigureOffset(iShape);
    
    for (int iFigure = FigureOffset(iShape); iFigure < iNextFigure; iFigure++) {
		mapnik::geometry::linear_ring<double> ring;
		int iPoint = PointOffset(iFigure);
        int iNextPoint = NextPointOffset(iFigure);
        while (iPoint < iNextPoint) {
            if (colType == Geography) {
                ring.emplace_back(ReadY(iPoint), ReadX(iPoint));
            } else {
                ring.emplace_back(ReadX(iPoint), ReadY(iPoint));
            }

            ++iPoint;
        }
		if (iFigure == 0) {
			geom.set_exterior_ring(std::move(ring));
		} else {
			geom.add_hole(std::move(ring));
		}
	}
    return geom;
}

/************************************************************************/
/*                         ReadMultiPolygon()                           */
/************************************************************************/

mapnik::geometry::multi_polygon<double> sqlserver_geometry_parser::ReadMultiPolygon(int iShape)
{
    mapnik::geometry::multi_polygon<double> geom;

    for (int i = iShape + 1; i < nNumShapes; i++) {
        if (ParentOffset(i) == (unsigned int)iShape) {
            if ( ShapeType(i) == ST_POLYGON ) {
                geom.emplace_back(ReadPolygon(i));
            }
        }
    }

    return geom;
}

/************************************************************************/
/*                         ReadGeometryCollection()                     */
/************************************************************************/

mapnik::geometry::geometry_collection<double> sqlserver_geometry_parser::ReadGeometryCollection(int iShape)
{
    mapnik::geometry::geometry_collection<double> geom;

    for (int i = iShape + 1; i < nNumShapes; i++) {
        mapnik::geometry::geometry<double> shape = mapnik::geometry::geometry_empty();
        if (ParentOffset(i) == (unsigned int)iShape) {
            switch (ShapeType(i))
            {
            case ST_POINT:
                shape = ReadPoint(i);
                break;
            case ST_LINESTRING:
                shape = ReadLineString(i);
                break;
            case ST_POLYGON:
                shape = ReadPolygon(i);
                break;
            case ST_MULTIPOINT:
                shape = ReadMultiPoint(i);
                break;
            case ST_MULTILINESTRING:
                shape = ReadMultiLineString(i);
                break;
            case ST_MULTIPOLYGON:
                shape = ReadMultiPolygon(i);
                break;
            case ST_GEOMETRYCOLLECTION:
                shape = ReadGeometryCollection(i);
                break;
            }
        }
        geom.push_back(shape);
    }

  return geom;
}


/************************************************************************/
/*                         parse_sql_geometry()                         */
/************************************************************************/

mapnik::geometry::geometry<double> sqlserver_geometry_parser::parse(unsigned char* pszInput, int nLen)
{
    if (nLen < 10) {
        throw sqlserver_geometry_parser_exception("not enough data, nLen < 10");
    }
    
    pszData = pszInput;
    
    /* store the SRS id for further use */
    nSRSId = ReadInt32(0);
    
    if ( ReadByte(4) != 1 )
    {
        throw sqlserver_geometry_parser_exception("corrupt data, ReadByte(4) != 1");
    }

    chProps = ReadByte(5);

    if ( chProps & SP_HASMVALUES )
        nPointSize = 32;
    else if ( chProps & SP_HASZVALUES )
        nPointSize = 24;
    else
        nPointSize = 16;

    mapnik::geometry::geometry<double> geom;
    if ( chProps & SP_ISSINGLEPOINT )
    {
        // single point geometry
        nNumPoints = 1;
        nPointPos = 6;

        if (nLen < 6 + nPointSize)
        {
            throw sqlserver_geometry_parser_exception("not enough data, nLen < 6 + nPointSize");
        }
        
        mapnik::geometry::point<double> point;
        
        if (colType == Geography)
        {
            point.x = ReadY(0);
            point.y = ReadX(0);
        }
        else
        {
            point.x = ReadX(0);
            point.y = ReadY(0);
        }
        geom = point;
    }
    else if ( chProps & SP_ISSINGLELINESEGMENT )
    {
        // single line segment with 2 points
        nNumPoints = 2;
        nPointPos = 6;

        if (nLen < 6 + 2 * nPointSize)
        {
            throw sqlserver_geometry_parser_exception("not enough data, nLen < 6 + 2 * nPointSize");
        }

        mapnik::geometry::line_string<double> line;

        if (colType == Geography)
        {
            line.emplace_back(ReadY(0), ReadX(0));
            line.emplace_back(ReadY(1), ReadX(1));
        }
        else
        {
            line.emplace_back(ReadX(0), ReadY(0));
            line.emplace_back(ReadX(1), ReadY(1));
        }
        geom = line;
        
    }
    else
    {
        // complex geometries
        nNumPoints = ReadInt32(6);

        if ( nNumPoints <= 0 )
        {
            throw sqlserver_geometry_parser_exception("negative number of points, nNumPoints <= 0");
        }

        // position of the point array
        nPointPos = 10;

        // position of the figures
        nFigurePos = nPointPos + nPointSize * nNumPoints + 4;
        
        if (nLen < nFigurePos)
        {
            throw sqlserver_geometry_parser_exception("not enough data, nLen < nFigurePos");
        }

        nNumFigures = ReadInt32(nFigurePos - 4);

        if ( nNumFigures <= 0 )
        {
            throw sqlserver_geometry_parser_exception("negative number of figures, nNumFigures <= 0");
        }
        
        // position of the shapes
        nShapePos = nFigurePos + 5 * nNumFigures + 4;

        if (nLen < nShapePos)
        {
            throw sqlserver_geometry_parser_exception("not enough data, nLen < nShapePos");
        }

        nNumShapes = ReadInt32(nShapePos - 4);

        if (nLen < nShapePos + 9 * nNumShapes)
        {
            throw sqlserver_geometry_parser_exception("not enough data, nLen < nShapePos + 9 * nNumShapes");
        }

        if ( nNumShapes <= 0 )
        {
            throw sqlserver_geometry_parser_exception("negative number of shapes, nNumShapes <= 0");
        }

        // pick up the root shape
        if ( ParentOffset(0) != 0xFFFFFFFF)
        {
            throw sqlserver_geometry_parser_exception("corrupt data, ParentOffset(0) != 0xFFFFFFFF");
        }

        // determine the shape type
        switch (ShapeType(0))
        {
        case ST_POINT:
            geom = ReadPoint(0);
            break;
        case ST_LINESTRING:
            geom = ReadLineString(0);
            break;
        case ST_POLYGON:
            geom = ReadPolygon(0);
            break;
        case ST_MULTIPOINT:
            geom = ReadMultiPoint(0);
            break;
        case ST_MULTILINESTRING:
            geom = ReadMultiLineString(0);
            break;
        case ST_MULTIPOLYGON:
            geom = ReadMultiPolygon(0);
            break;
        case ST_GEOMETRYCOLLECTION:
            geom = ReadGeometryCollection(0);
            break;
        default:
            throw sqlserver_geometry_parser_exception("unsupported geometry type");
        }
    }

    return geom;
}

