// plugin
#include "geowave_featureset.hpp"

// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/debug.hpp>

// GeoWave
#include <jace/Jace.h>
using jace::java_cast;
using jace::java_new;
using jace::instanceof;

#include "jace/proxy/types/JBoolean.h"
using jace::proxy::types::JBoolean;
#include "jace/proxy/types/JDouble.h"
using jace::proxy::types::JDouble;
#include "jace/proxy/types/JFloat.h"
using jace::proxy::types::JFloat;
#include "jace/proxy/types/JInt.h"
using jace::proxy::types::JInt;
#include "jace/proxy/types/JLong.h"
using jace::proxy::types::JLong;
#include "jace/proxy/types/JShort.h"
using jace::proxy::types::JShort;

#include "jace/proxy/java/lang/Boolean.h"
using jace::proxy::java::lang::Boolean;
#include "jace/proxy/java/lang/Class.h"
using jace::proxy::java::lang::Class;
#include "jace/proxy/java/lang/Number.h"
using jace::proxy::java::lang::Number;
#include "jace/proxy/java/lang/Double.h"
using jace::proxy::java::lang::Double;
#include "jace/proxy/java/lang/Float.h"
using jace::proxy::java::lang::Float;
#include "jace/proxy/java/lang/Integer.h"
using jace::proxy::java::lang::Integer;
#include "jace/proxy/java/lang/Long.h"
using jace::proxy::java::lang::Long;
#include "jace/proxy/java/lang/Short.h"
using jace::proxy::java::lang::Short;
#include "jace/proxy/java/lang/String.h"
using jace::proxy::java::lang::String;
#include "jace/proxy/java/util/Date.h"
using jace::proxy::java::util::Date;
#include "jace/proxy/java/util/List.h"
using jace::proxy::java::util::List;

#include "jace/proxy/com/vividsolutions/jts/geom/Coordinate.h"
using jace::proxy::com::vividsolutions::jts::geom::Coordinate;
#include "jace/proxy/com/vividsolutions/jts/geom/Geometry.h"
using jace::proxy::com::vividsolutions::jts::geom::Geometry;
#include "jace/proxy/com/vividsolutions/jts/geom/GeometryCollection.h"
using jace::proxy::com::vividsolutions::jts::geom::GeometryCollection;

#include "jace/proxy/org/opengis/feature/simple/SimpleFeature.h"
using jace::proxy::org::opengis::feature::simple::SimpleFeature;
#include "jace/proxy/org/opengis/feature/simple/SimpleFeatureType.h"
using jace::proxy::org::opengis::feature::simple::SimpleFeatureType;
#include "jace/proxy/org/opengis/feature/type/AttributeType.h"
using jace::proxy::org::opengis::feature::type::AttributeType;
#include "jace/proxy/org/opengis/feature/type/AttributeDescriptor.h"
using jace::proxy::org::opengis::feature::type::AttributeDescriptor;

geowave_featureset::geowave_featureset(CloseableIterator iterator,
                                       std::string const& encoding,
                                       mapnik::context_ptr const& ctx) 
   :  ctx_ (ctx),
      iterator_ (iterator),
      feature_id_ (1),
      tr_(new mapnik::transcoder(encoding)) { }

geowave_featureset::~geowave_featureset() 
{
    if (!iterator_.isNull())
        iterator_.close();
}

mapnik::geometry::point<double> geowave_featureset::create_point(Point point)
{
    Coordinate coord = point.getCoordinate();
    return mapnik::geometry::point<double>(coord.x(), coord.y());
}

mapnik::geometry::multi_point<double> geowave_featureset::create_multi_point(MultiPoint multi_point)
{
    mapnik::geometry::multi_point<double> multi_point_out;
    for (int point_idx = 0; point_idx < multi_point.getNumGeometries(); ++point_idx)
    {
        Coordinate coord = java_cast<Point>(multi_point.getGeometryN(point_idx)).getCoordinate();
        multi_point_out.add_coord(coord.x(), coord.y());
    }
    return multi_point_out;
}

mapnik::geometry::line_string<double> geowave_featureset::create_line_string(LineString line_string)
{
    mapnik::geometry::line_string<double> line_string_out;
    for (int point_idx = line_string.getNumPoints()-1; point_idx >= 0; --point_idx)
    {
        Coordinate coord = line_string.getPointN(point_idx).getCoordinate();
        line_string_out.add_coord(coord.x(), coord.y());
    }
    if (line_string.isClosed())
    {
        Coordinate coord = line_string.getPointN(line_string.getNumPoints()-1).getCoordinate();
        line_string_out.add_coord(coord.x(), coord.y());
    }
    return line_string_out;
}

mapnik::geometry::multi_line_string<double> geowave_featureset::create_multi_line_string(MultiLineString multi_line_string)
{
    mapnik::geometry::multi_line_string<double> multi_line_string_out;
    for (int geom_idx = 0; geom_idx < multi_line_string.getNumGeometries(); ++geom_idx)
        multi_line_string_out.push_back(create_line_string(java_cast<LineString>(multi_line_string.getGeometryN(geom_idx))));
    return multi_line_string_out;
}

mapnik::geometry::polygon<double>  geowave_featureset::create_polygon(Polygon polygon)
{
    mapnik::geometry::polygon<double> polygon_out;
    
    // handle exterior ring
    {
        LineString geom = polygon.getExteriorRing();
        mapnik::geometry::linear_ring<double> linear_ring;
        for (int point_idx = geom.getNumPoints()-1; point_idx >= 0; --point_idx)
        {
            Coordinate coord = geom.getPointN(point_idx).getCoordinate();
            linear_ring.add_coord(coord.x(), coord.y());
        }
        if (geom.isClosed())
        {
            Coordinate coord = geom.getPointN(geom.getNumPoints()-1).getCoordinate();
            linear_ring.add_coord(coord.x(), coord.y());
        }
        polygon_out.set_exterior_ring(std::move(linear_ring));
    }

    // handle interior rings
    {
        for (int ring_idx = 0; ring_idx < polygon.getNumInteriorRing(); ++ring_idx)
        {
            LineString geom = polygon.getInteriorRingN(ring_idx);
            mapnik::geometry::linear_ring<double> linear_ring;
            for (int point_idx = geom.getNumPoints()-1; point_idx >= 0; --point_idx)
            {
                Coordinate coord = geom.getPointN(point_idx).getCoordinate();
                linear_ring.add_coord(coord.x(), coord.y());
            }
            if (geom.isClosed())
            {
                Coordinate coord = geom.getPointN(geom.getNumPoints()-1).getCoordinate();
                linear_ring.add_coord(coord.x(), coord.y());
            }
            polygon_out.add_hole(std::move(linear_ring));
        }
    }
    return polygon_out;
}

mapnik::geometry::multi_polygon<double>  geowave_featureset::create_multi_polygon(MultiPolygon multi_polygon)
{
    mapnik::geometry::multi_polygon<double> multi_polygon_out;
    for (int geom_idx = 0; geom_idx < multi_polygon.getNumGeometries(); ++geom_idx)
        multi_polygon_out.push_back(create_polygon(java_cast<Polygon>(multi_polygon.getGeometryN(geom_idx))));
    return multi_polygon_out;
}

mapnik::geometry::geometry<double> geowave_featureset::get_geometries(Object attrib)
{
    if (instanceof<Point>(attrib))
        return create_point(java_cast<Point>(attrib));
    else if (instanceof<MultiPoint>(attrib))
        return create_multi_point(java_cast<MultiPoint>(attrib));
    else if (instanceof<LineString>(attrib))
        return create_line_string(java_cast<LineString>(attrib));
    else if (instanceof<MultiLineString>(attrib))
        return create_multi_line_string(java_cast<MultiLineString>(attrib));
    else if (instanceof<Polygon>(attrib))
        return create_polygon(java_cast<Polygon>(attrib));
    else if (instanceof<MultiPolygon>(attrib))
        return create_multi_polygon(java_cast<MultiPolygon>(attrib));
    else if (instanceof<GeometryCollection>(attrib))
    {
        GeometryCollection geom_coll = java_cast<GeometryCollection>(attrib);
        mapnik::geometry::geometry_collection<double> geom_coll_out;
        for (int geom_idx = 0; geom_idx < geom_coll.getNumGeometries(); ++geom_idx)
            geom_coll_out.push_back(get_geometries(geom_coll.getGeometryN(geom_idx)));
        return geom_coll_out;
    }
    else
        MAPNIK_LOG_DEBUG(geowave) << "GeoWave Plugin: Unknown geometry type";
    return mapnik::geometry::geometry_empty();
}

mapnik::feature_ptr geowave_featureset::next()
{
    if (iterator_.hasNext())
    {
        // first, read a feature
        SimpleFeature simpleFeature = java_cast<SimpleFeature>(iterator_.next());
        List attribs = simpleFeature.getType().getAttributeDescriptors();
       
        // create a new mapnik feature
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_, feature_id_++));
        
        // build the mapnik feature using geotools feature attributes
        for (int i = 0; i < attribs.size(); ++i)
        {
            String name = java_cast<AttributeDescriptor>(attribs.get(i)).getLocalName();
            Object attrib = simpleFeature.getAttribute(name);
            if (attrib.isNull())
                continue;

            if (instanceof<Geometry>(attrib))
            {
                feature->set_geometry(get_geometries(attrib));
            }
            else if (instanceof<Number>(attrib))
            {
                if (instanceof<Double>(attrib))
                {
                    double value = java_cast<Double>(attrib).doubleValue();
                    feature->put(name, 
                                 value);
                }
                else if (instanceof<Float>(attrib))
                {
                    float value = java_cast<Float>(attrib).floatValue();
                    feature->put(name, 
                                 (double)value);
                }
                else if (instanceof<Integer>(attrib))
                {
                    int value = java_cast<Integer>(attrib).intValue();
                    feature->put<mapnik::value_integer>(name, 
                                                        value);
                }
                else if (instanceof<Long>(attrib))
                {
                    long value = java_cast<Long>(attrib).longValue();
                    feature->put<mapnik::value_integer>(name, 
                                                        value);
                }
                else if (instanceof<Short>(attrib))
                {
                    short value = java_cast<Short>(attrib).shortValue();
                    feature->put<mapnik::value_integer>(name, 
                                                        value);
                }
                else
                {
                    MAPNIK_LOG_DEBUG(geowave) << "GeoWave Plugin: Unknown number type";
                }
            }
            else if (instanceof<Boolean>(attrib))
            {
                bool value = java_cast<Boolean>(attrib).booleanValue();
                feature->put(name,
                             value);
            }
            else if (instanceof<String>(attrib))
            {
                mapnik::value_unicode_string ustr = tr_->transcode(((std::string)java_cast<String>(attrib)).c_str());
                feature->put(name, 
                             ustr);
            }
            else if (instanceof<Date>(attrib))
            {
                mapnik::value_unicode_string ustr = tr_->transcode(((std::string)java_cast<Date>(attrib).toString()).c_str());
                feature->put(name, 
                             ustr);
            }
            else
            {
                MAPNIK_LOG_DEBUG(geowave) << "GeoWave Plugin: Unknown attribute type";
            }
        }

        // return the feature!
        return feature;
    }

    // otherwise return an empty feature
    return mapnik::feature_ptr();
}
