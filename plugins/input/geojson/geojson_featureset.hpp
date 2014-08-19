#ifndef GEOJSON_FEATURESET_HPP
#define GEOJSON_FEATURESET_HPP

#include <mapnik/feature.hpp>
#include "geojson_datasource.hpp"

#include <vector>
#include <deque>


class geojson_featureset : public mapnik::Featureset
{
public:
    typedef std::deque<geojson_datasource::item_type> array_type;
    geojson_featureset(std::vector<mapnik::feature_ptr> const& features,
                       array_type::const_iterator index_itr,
                       array_type::const_iterator index_end);
    virtual ~geojson_featureset();
    mapnik::feature_ptr next();

private:
    mapnik::box2d<double> box_;
    std::vector<mapnik::feature_ptr> const& features_;
    array_type::const_iterator index_itr_;
    array_type::const_iterator index_end_;
};

#endif // GEOJSON_FEATURESET_HPP
