#ifndef HELLO_FEATURESET_HPP
#define HELLO_FEATURESET_HPP

// mapnik
#include <mapnik/datasource.hpp>

// boost
#include <boost/scoped_ptr.hpp> // needed for wrapping the transcoder

// extend the mapnik::Featureset defined in include/mapnik/datasource.hpp
class hello_featureset : public mapnik::Featureset
{
public:
    // this constructor can have any arguments you need
    hello_featureset(mapnik::box2d<double> const& box, std::string const& encoding);

    // desctructor
    virtual ~hello_featureset();

    // mandatory: you must expose a next() method, called when rendering
    mapnik::feature_ptr next();

private:
    // members are up to you, but these are recommended
    mapnik::box2d<double> const& box_;
    mutable int feature_id_;
    boost::scoped_ptr<mapnik::transcoder> tr_;
    mapnik::context_ptr ctx_;
};

#endif // HELLO_FEATURESET_HPP
