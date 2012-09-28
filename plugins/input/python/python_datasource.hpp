#ifndef PYTHON_DATASOURCE_HPP
#define PYTHON_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>

// boost
#include <boost/python.hpp>

class python_datasource : public mapnik::datasource
{
public:
    // constructor
    // arguments must not change
    python_datasource(mapnik::parameters const& params, bool bind=true);

    // destructor
    virtual ~python_datasource ();

    // mandatory: type of the plugin, used to match at runtime
    mapnik::datasource::datasource_t type() const;

    // mandatory: name of the plugin
    static const char* name();

    // mandatory: function to query features by box2d
    // this is called when rendering, specifically in feature_style_processor.hpp
    mapnik::featureset_ptr features(mapnik::query const& q) const;

    // mandatory: function to query features by point (coord2d)
    // not used by rendering, but available to calling applications
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;

    // mandatory: return the box2d of the datasource
    // called during rendering to determine if the layer should be processed
    mapnik::box2d<double> envelope() const;

    // mandatory: optionally return the overal geometry type of the datasource
    boost::optional<mapnik::datasource::geometry_t> get_geometry_type() const;

    // mandatory: return the layer descriptor
    mapnik::layer_descriptor get_descriptor() const;

    // mandatory: will bind the datasource given params
    void bind() const;

private:
    static const char* name_;
    mutable mapnik::layer_descriptor desc_;
    const std::string factory_;
    std::map<std::string, std::string> kwargs_;
    mutable boost::python::object datasource_;
};


#endif // PYTHON_DATASOURCE_HPP
