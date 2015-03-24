// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/value_types.hpp>

// boost

#include "hello_featureset.hpp"

hello_featureset::hello_featureset(mapnik::box2d<double> const& box, std::string const& encoding)
    : box_(box),
      feature_id_(1),
      tr_(new mapnik::transcoder(encoding)),
      ctx_(std::make_shared<mapnik::context_type>()) {
        // add known field names to attributes schema
        ctx_->push("key");
      }

hello_featureset::~hello_featureset() { }

mapnik::feature_ptr hello_featureset::next()
{
    if (feature_id_ == 1)
    {
        // create a new feature
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));

        // increment the count
        ++feature_id_;

        // create an attribute pair of key:value
        feature->put("key",tr_->transcode("hello world point!"));

        // take the center of the bbox that was used to query
        // to dynamically generate a fake point
        mapnik::coord2d center = box_.center();

        // create a new point geometry
        feature->set_geometry(mapnik::new_geometry::point(center.x,center.y));

        // return the feature!
        return feature;
    }
    else if (feature_id_ == 2)
    {
        // create a second feature
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));

        // increment the count
        ++feature_id_;

        // create an attribute pair of key:value
        feature->put("key",tr_->transcode("hello world line!"));

        // take the outer ring of the bbox that was used to query
        // to dynamically generate a fake line
        mapnik::new_geometry::line_string line;
        line.reserve(4);
        line.add_coord(box_.minx(),box_.maxy());
        line.add_coord(box_.maxx(),box_.maxy());
        line.add_coord(box_.maxx(),box_.miny());
        line.add_coord(box_.minx(),box_.miny());
        feature->set_geometry(std::move(line));
        return feature;
    }

    // otherwise return an empty feature
    return mapnik::feature_ptr();
}
