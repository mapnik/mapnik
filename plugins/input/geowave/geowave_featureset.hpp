#ifndef GEOWAVE_FEATURESET_HPP
#define GEOWAVE_FEATURESET_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/geometry.hpp>

// GeoWave
#include "jace/proxy/java/lang/Object.h"
using jace::proxy::java::lang::Object;

#include "jace/proxy/mil/nga/giat/geowave/core/store/CloseableIterator.h"
using jace::proxy::mil::nga::giat::geowave::core::store::CloseableIterator;

#include "jace/proxy/com/vividsolutions/jts/geom/LineString.h"
using jace::proxy::com::vividsolutions::jts::geom::LineString;
#include "jace/proxy/com/vividsolutions/jts/geom/MultiLineString.h"
using jace::proxy::com::vividsolutions::jts::geom::MultiLineString;
#include "jace/proxy/com/vividsolutions/jts/geom/MultiPoint.h"
using jace::proxy::com::vividsolutions::jts::geom::MultiPoint;
#include "jace/proxy/com/vividsolutions/jts/geom/MultiPolygon.h"
using jace::proxy::com::vividsolutions::jts::geom::MultiPolygon;
#include "jace/proxy/com/vividsolutions/jts/geom/Point.h"
using jace::proxy::com::vividsolutions::jts::geom::Point;
#include "jace/proxy/com/vividsolutions/jts/geom/Polygon.h"
using jace::proxy::com::vividsolutions::jts::geom::Polygon;

class geowave_featureset : public mapnik::Featureset
{
public:
    geowave_featureset(CloseableIterator iterator,
                       std::string const& encoding,
                       mapnik::context_ptr const& ctx);
    virtual ~geowave_featureset();
    mapnik::feature_ptr next();

protected:

    mapnik::geometry::point<double> create_point(Point point);
    mapnik::geometry::multi_point<double> create_multi_point(MultiPoint multi_point);
    mapnik::geometry::line_string<double> create_line_string(LineString line_string);
    mapnik::geometry::multi_line_string<double> create_multi_line_string(MultiLineString multi_line_string);
    mapnik::geometry::polygon<double>  create_polygon(Polygon polygon);
    mapnik::geometry::multi_polygon<double>  create_multi_polygon(MultiPolygon multi_polygon);

    // this method is used to recursively add geometries to our Mapnik feature
    mapnik::geometry::geometry<double> get_geometries(Object attrib);

private:
    mapnik::context_ptr ctx_;
    CloseableIterator iterator_;
    mapnik::value_integer feature_id_;
    const std::unique_ptr<mapnik::transcoder> tr_;
};

#endif // GEOWAVE_FEATURESET_HPP
