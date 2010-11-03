#include "hello_featureset.hpp"
#include <mapnik/geometry.hpp>

hello_featureset::hello_featureset(mapnik::box2d<double> const& box, std::string const& encoding)
  : box_(box),
    count_(0),
    tr_(new mapnik::transcoder(encoding)) { }

hello_featureset::~hello_featureset() { }

mapnik::feature_ptr hello_featureset::next()
{
    if (!count_)
    {
        // create a new feature
        mapnik::feature_ptr feature(new mapnik::Feature(count_));

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
        
        // increment to count so that we only return one feature
        ++count_;
        
        // return the feature!
        return feature;
    }
    
    // otherwise return an empty feature_ptr
    return mapnik::feature_ptr();
}

