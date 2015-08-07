// mapnik
#include <mapnik/geometry.hpp>

enum spatial_data_type {
    Geometry,
    Geography
};

class sqlserver_geometry_parser
{
protected:    
    unsigned char* pszData;
    /* serialization propeties */
    char chProps;
    /* point array */
    int nPointSize;
    int nPointPos;
    int nNumPoints;
    /* figure array */
    int nFigurePos;
    int nNumFigures;
    /* shape array */
    int nShapePos;
    int nNumShapes;
    int nSRSId;
    /* geometry or geography */
    spatial_data_type colType;

protected:
    mapnik::geometry::point<double> ReadPoint(int iShape);
    mapnik::geometry::multi_point<double> ReadMultiPoint(int iShape);
    mapnik::geometry::line_string<double> ReadLineString(int iShape);
    mapnik::geometry::multi_line_string<double> ReadMultiLineString(int iShape);
    mapnik::geometry::polygon<double> ReadPolygon(int iShape);
    mapnik::geometry::multi_polygon<double> ReadMultiPolygon(int iShape);
    mapnik::geometry::geometry_collection<double> ReadGeometryCollection(int iShape);

public:
    sqlserver_geometry_parser(spatial_data_type columnType);
    
    mapnik::geometry::geometry<double> parse(unsigned char* pszInput, int nLen);
    int get_srs_id() { return nSRSId; };
};


