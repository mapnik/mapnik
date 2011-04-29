#include "hello_featureset.hpp"
#include <mapnik/geometry.hpp>

hello_featureset::hello_featureset(mapnik::box2d<double> const& box, std::string const& encoding)
  : box_(box),
    feature_id_(1),
    tr_(new mapnik::transcoder(encoding)) { }

hello_featureset::~hello_featureset() { }

mapnik::feature_ptr hello_featureset::next()
{
    if (feature_id_ == 1)
    {
        // create a new feature
        mapnik::feature_ptr feature(new mapnik::Feature(feature_id_));
        ++feature_id_;

        // create an attribute pair of key:value
        UnicodeString ustr = tr_->transcode("hello world!");
        boost::put(*feature,"key",ustr);

        // we need a geometry to display so just for fun here
        // we take the center of the bbox that was used to query
        // since we don't actually have any data to pull from...
        mapnik::coord2d center = box_.center();
        
        // create a new point geometry
        mapnik::geometry_type * pt = new mapnik::geometry_type(mapnik::Point);
        
        // we use path type geometries in Mapnik to fit nicely with AGG and Cairo
        // here we stick an x,y pair into the geometry using move_to()
        pt->move_to(center.x,center.y);
        
        // add the geometry to the feature
        feature->add_geometry(pt);
        
        // A feature usually will have just one geometry of a given type
        // but mapnik does support many geometries per feature of any type
        // so here we draw a line around the point
        mapnik::geometry_type * line = new mapnik::geometry_type(mapnik::LineString);
        line->move_to(box_.minx(),box_.miny());
        line->line_to(box_.minx(),box_.maxy());
        line->line_to(box_.maxx(),box_.maxy());
        line->line_to(box_.maxx(),box_.miny());
        line->line_to(box_.minx(),box_.miny());
        feature->add_geometry(line);
        
        // increment the count so that we only return one feature
        ++count_;
        
        // return the feature!
        return feature;
    }
    
    // otherwise return an empty feature
    return mapnik::feature_ptr();
}

