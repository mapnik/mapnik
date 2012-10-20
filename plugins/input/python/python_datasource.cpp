// file plugin
#include "python_datasource.hpp"
#include "python_featureset.hpp"

// stl
#include <string>
#include <vector>

// boost
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/algorithm/string.hpp>

#include "python_utils.hpp"

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(python_datasource)

python_datasource::python_datasource(parameters const& params, bool bind)
  : datasource(params),
    desc_(*params_.get<std::string>("type"), *params_.get<std::string>("encoding","utf-8")),
    factory_(*params_.get<std::string>("factory", ""))
{
    // extract any remaining parameters as keyword args for the factory
    BOOST_FOREACH(const mapnik::parameters::value_type& kv, params_)
    {
        if((kv.first != "type") && (kv.first != "factory"))
        {
            kwargs_.insert(std::make_pair(kv.first, *params_.get<std::string>(kv.first)));
        }
    }

    if (bind)
    {
        this->bind();
    }
}

python_datasource::~python_datasource() { }

// This name must match the plugin filename, eg 'python.input'
const char* python_datasource::name_="python";

const char* python_datasource::name()
{
    return name_;
}

mapnik::layer_descriptor python_datasource::get_descriptor() const
{
    if (!is_bound_) bind();

    return desc_;
}

// The following methods call into the Python interpreter and hence require, unfortunately, that the GIL be held.

void python_datasource::bind() const
{
    using namespace boost;
    using namespace boost::python;

    if (is_bound_) return;

    // if no factory callable is defined, bind is a nop
    if (factory_.empty()) return;

    try
    {
        // split factory at ':' to parse out module and callable
        std::vector<std::string> factory_split;
        split(factory_split, factory_, is_any_of(":"));
        if ((factory_split.size() < 1) || (factory_split.size() > 2))
        {
            throw mapnik::datasource_exception(
                std::string("python: factory string must be of the form '[module:]callable' when parsing \"")
                      + factory_ + '"');
        }
        // extract the module and the callable
        str module_name("__main__"), callable_name;
        if (factory_split.size() == 1)
        {
            callable_name = str(factory_split[0]);
        }
        else
        {
            module_name = str(factory_split[0]);
            callable_name = str(factory_split[1]);
        }
        ensure_gil lock;
        // import the main module from Python (in case we're embedding the
        // interpreter directly) and also import the callable.
        object main_module = import("__main__");
        object callable_module = import(module_name);
        object callable = callable_module.attr(callable_name);
        // prepare the arguments
        dict kwargs;
        typedef std::map<std::string, std::string>::value_type kv_type;
        BOOST_FOREACH(const kv_type& kv, kwargs_)
        {
            kwargs[str(kv.first)] = str(kv.second);
        }

        // get our wrapped data source
        datasource_ = callable(*boost::python::make_tuple(), **kwargs);
    }
    catch ( error_already_set )
    {
        throw mapnik::datasource_exception(extractException());
    }

    is_bound_ = true;
}

mapnik::datasource::datasource_t python_datasource::type() const
{
    using namespace boost::python;

    typedef boost::optional<mapnik::datasource::geometry_t> return_type;

    if (!is_bound_) bind();

    try
    {
        ensure_gil lock;
        object data_type = datasource_.attr("data_type");
        long data_type_integer = extract<long>(data_type);
        return mapnik::datasource::datasource_t(data_type_integer);
    }
    catch ( error_already_set )
    {
        throw mapnik::datasource_exception(extractException());
    }

}

mapnik::box2d<double> python_datasource::envelope() const
{
    using namespace boost::python;

    if (!is_bound_) bind();

    try
    {
        ensure_gil lock;
        return extract<mapnik::box2d<double> >(datasource_.attr("envelope"));
    }
    catch ( error_already_set )
    {
        throw mapnik::datasource_exception(extractException());
    }
}

boost::optional<mapnik::datasource::geometry_t> python_datasource::get_geometry_type() const
{
    using namespace boost::python;

    typedef boost::optional<mapnik::datasource::geometry_t> return_type;

    if (!is_bound_) bind();

    try
    {
        ensure_gil lock;
        // if the datasource object has no geometry_type attribute, return a 'none' value
        if (!PyObject_HasAttrString(datasource_.ptr(), "geometry_type"))
        {
            return return_type();
        }
        object py_geometry_type = datasource_.attr("geometry_type");
        // if the attribute value is 'None', return a 'none' value
        if (py_geometry_type.ptr() == object().ptr())
        {
            return return_type();
        }
        long geom_type_integer = extract<long>(py_geometry_type);
        return mapnik::datasource::geometry_t(geom_type_integer);
    }
    catch ( error_already_set )
    {
        throw mapnik::datasource_exception(extractException());
    }
}

mapnik::featureset_ptr python_datasource::features(mapnik::query const& q) const
{
    using namespace boost::python;

    if (!is_bound_) bind();

    try
    {
        // if the query box intersects our world extent then query for features
        if (envelope().intersects(q.get_bbox()))
        {
            ensure_gil lock;
            object features(datasource_.attr("features")(q));
            // if 'None' was returned, return an empty feature set
            if(features.ptr() == object().ptr())
            {
                return mapnik::featureset_ptr();
            }
            return boost::make_shared<python_featureset>(features);
        }
        // otherwise return an empty featureset pointer
        return mapnik::featureset_ptr();
    }
    catch ( error_already_set )
    {
        throw mapnik::datasource_exception(extractException());
    }
}

mapnik::featureset_ptr python_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    using namespace boost::python;

    if (!is_bound_) bind();

    try
    {
        ensure_gil lock;
        object features(datasource_.attr("features_at_point")(pt));
        // if we returned none, return an empty set
        if(features.ptr() == object().ptr())
        {
            return mapnik::featureset_ptr();
        }
        // otherwise, return a feature set which can iterate over the iterator
        return boost::make_shared<python_featureset>(features);
    }
    catch ( error_already_set )
    {
        throw mapnik::datasource_exception(extractException());
    }

}
