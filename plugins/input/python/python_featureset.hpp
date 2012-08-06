#ifndef PYTHON_FEATURESET_HPP
#define PYTHON_FEATURESET_HPP

// boost
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>

// mapnik
#include <mapnik/datasource.hpp>

// extend the mapnik::Featureset defined in include/mapnik/datasource.hpp
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
    typedef boost::python::stl_input_iterator<mapnik::feature_ptr> feature_iter;

    feature_iter begin_, end_;
};

#endif // PYTHON_FEATURESET_HPP
