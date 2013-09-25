#ifndef HELLO_FEATURESET_HPP
#define HELLO_FEATURESET_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>

// boost
 // needed for wrapping the transcoder

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
    mapnik::box2d<double> box_;
    mapnik::value_integer feature_id_;
    const std::unique_ptr<mapnik::transcoder> tr_;
    mapnik::context_ptr ctx_;
};

#endif // HELLO_FEATURESET_HPP
