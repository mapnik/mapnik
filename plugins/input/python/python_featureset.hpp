#ifndef PYTHON_FEATURESET_HPP
#define PYTHON_FEATURESET_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/feature.hpp>

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/datasource.hpp>

class python_featureset : public mapnik::Featureset
{
public:
    // this constructor can have any arguments you need
    python_featureset(boost::python::object iterator);

    // desctructor
    virtual ~python_featureset();

    // mandatory: you must expose a next() method, called when rendering
    mapnik::feature_ptr next();

private:
    using feature_iter = boost::python::stl_input_iterator<mapnik::feature_ptr>;

    feature_iter begin_, end_;
};

#endif // PYTHON_FEATURESET_HPP
