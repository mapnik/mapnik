#ifndef GEOWAVE_DATASOURCE_HPP
#define GEOWAVE_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>

// boost
#include <boost/optional.hpp>
#include <memory>

// stl
#include <string>

// GeoWave
#include "jace/proxy/org/opengis/filter/Filter.h"
using jace::proxy::org::opengis::filter::Filter;

#include "jace/proxy/mil/nga/giat/geowave/datastore/accumulo/BasicAccumuloOperations.h"
using jace::proxy::mil::nga::giat::geowave::datastore::accumulo::BasicAccumuloOperations;
#include "jace/proxy/mil/nga/giat/geowave/adapter/vector/FeatureDataAdapter.h"
using jace::proxy::mil::nga::giat::geowave::adapter::vector::FeatureDataAdapter;

class geowave_datasource : public mapnik::datasource
{
public:
    geowave_datasource(mapnik::parameters const& params);
    virtual ~geowave_datasource ();
    mapnik::datasource::datasource_t type() const;
    static const char * name();
    mapnik::featureset_ptr features(mapnik::query const& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    boost::optional<mapnik::datasource_geometry_t> get_geometry_type() const;
    mapnik::layer_descriptor get_descriptor() const;

private:
    void init(mapnik::parameters const& params);
    int create_jvm();
    bool init_extent();
    void init_layer_descriptor();
    bool init_geometry_type();
    
    mapnik::layer_descriptor desc_;
    mapnik::context_ptr ctx_;
    mapnik::box2d<double> extent_;
    std::string zookeeper_url_;
    std::string instance_name_;
    std::string username_;
    std::string password_;
    std::string table_namespace_;
    std::string adapter_id_;
    std::string cql_filter_;
    mapnik::datasource_geometry_t geometry_type_;
    
    BasicAccumuloOperations accumulo_operations_;
    FeatureDataAdapter feature_data_adapter_;
    Filter filter_;
};


#endif // GEOWAVE_DATASOURCE_HPP
