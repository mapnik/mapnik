// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/value_types.hpp>

// boost

#include "hello_featureset.hpp"

hello_featureset::hello_featureset(mapnik::box2d<double> const& box, std::string const& encoding)
    : box_(box),
      feature_id_(1),
      tr_(new mapnik::transcoder(encoding)),
      ctx_(std::make_shared<mapnik::context_type>()) { }

hello_featureset::~hello_featureset() { }

mapnik::feature_ptr hello_featureset::next()
{
    if (feature_id_ == 1)
    {
        // let us pretend it just has one column/attribute name
        std::string attribute("key");

        // the featureset context needs to know the field schema
        ctx_->push(attribute);

        // create a new feature
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));

        // increment the count so that we only return one feature
        ++feature_id_;

        // create an attribute pair of key:value
        mapnik::value_unicode_string ustr = tr_->transcode("hello world!");
        feature->put(attribute,ustr);

        // we need a geometry to display so just for fun here
        // we take the center of the bbox that was used to query
        // since we don't actually have any data to pull from...
        mapnik::coord2d center = box_.center();

        // create a new point geometry
        feature->set_geometry(mapnik::new_geometry::point(center.x,center.y));

        // A feature usually will have just one geometry of a given type
        // but mapnik supports many geometries per feature of any type
        // so here we draw a line around the point
        mapnik::new_geometry::line_string line;
        line.reserve(4);
        line.add_coord(box_.minx(),box_.maxy());
        line.add_coord(box_.maxx(),box_.maxy());
        line.add_coord(box_.maxx(),box_.miny());
        line.add_coord(box_.minx(),box_.miny());
        feature->set_geometry(std::move(line));

        // return the feature!
        return feature;
    }

    // otherwise return an empty feature
    return mapnik::feature_ptr();
}
