#ifndef GEOJSON_FEATURESET_HPP
#define GEOJSON_FEATURESET_HPP

#include <mapnik/datasource.hpp>
#include "geojson_datasource.hpp"

#include <vector>
#include <deque>


class geojson_featureset : public mapnik::Featureset
{
public:
    geojson_featureset(std::vector<mapnik::feature_ptr> const& features,
                       std::deque<std::size_t>::const_iterator index_itr,
                       std::deque<std::size_t>::const_iterator index_end);
    virtual ~geojson_featureset();
    mapnik::feature_ptr next();

private:
    mapnik::box2d<double> box_;
    std::vector<mapnik::feature_ptr> const& features_;
    std::deque<std::size_t>::const_iterator index_itr_;
    std::deque<std::size_t>::const_iterator index_end_;
};

#endif // GEOJSON_FEATURESET_HPP
