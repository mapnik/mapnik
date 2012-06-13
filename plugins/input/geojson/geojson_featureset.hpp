#ifndef GEOJSON_FEATURESET_HPP
#define GEOJSON_FEATURESET_HPP

#include <mapnik/datasource.hpp>
#include <vector>
#include "geojson_datasource.hpp"

class geojson_featureset : public mapnik::Featureset
{
public:
    geojson_featureset(std::vector<mapnik::feature_ptr> const& features,
                       geojson_datasource::spatial_index_type const& tree);
    virtual ~geojson_featureset();
    mapnik::feature_ptr next();

private:
    mapnik::box2d<double> box_;
    unsigned int feature_id_;
    std::vector<mapnik::feature_ptr> const& features_;
    geojson_datasource::spatial_index_type const& tree_;
};

#endif // GEOJSON_FEATURESET_HPP
