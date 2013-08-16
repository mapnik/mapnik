/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/
// boost
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>

#include "python_datasource.hpp"

namespace {

// Use RAII to acquire and release the GIL as needed.
class ensure_gil
{
    public:
        ensure_gil() : gil_state_(PyGILState_Ensure()) {}
        ~ensure_gil() { PyGILState_Release( gil_state_ ); }
    protected:
        PyGILState_STATE gil_state_;
};

} // end anonymous namespace

namespace mapnik {

python_datasource::python_datasource(boost::python::object ds)
  : datasource(parameters()),
    desc_("python", "utf-8"),
		datasource_(ds)
{
}

python_datasource::~python_datasource() { }

layer_descriptor python_datasource::get_descriptor() const
{
    return desc_;
}

datasource::datasource_t python_datasource::type() const
{
    typedef boost::optional<datasource::geometry_t> return_type;

    try
    {
        ensure_gil lock;
        boost::python::object data_type = datasource_.attr("data_type");
        long data_type_integer = boost::python::extract<long>(data_type);
        return datasource::datasource_t(data_type_integer);
    }
    catch ( boost::python::error_already_set )
    {
        //throw datasource_exception(extractException());
    }

}

box2d<double> python_datasource::envelope() const
{
    box2d<double> box;
    try
    {
        ensure_gil lock;
        if (!PyObject_HasAttrString(datasource_.ptr(), "envelope"))
        {
            throw datasource_exception("Python: could not access envelope property");
        }
        else
        {
            boost::python::object py_envelope = datasource_.attr("envelope");
            if (py_envelope.ptr() == boost::python::object().ptr())
            {
                throw datasource_exception("Python: could not access envelope property");
            }
            else
            {
                boost::python::extract<double> ex(py_envelope.attr("minx"));
                if (!ex.check()) throw datasource_exception("Python: could not convert envelope.minx");
                box.set_minx(ex());
                boost::python::extract<double> ex1(py_envelope.attr("miny"));
                if (!ex1.check()) throw datasource_exception("Python: could not convert envelope.miny");
                box.set_miny(ex1());
                boost::python::extract<double> ex2(py_envelope.attr("maxx"));
                if (!ex2.check()) throw datasource_exception("Python: could not convert envelope.maxx");
                box.set_maxx(ex2());
                boost::python::extract<double> ex3(py_envelope.attr("maxy"));
                if (!ex3.check()) throw datasource_exception("Python: could not convert envelope.maxy");
                box.set_maxy(ex3());
            }
        }
    }
    catch ( boost::python::error_already_set )
    {
        //throw datasource_exception(extractException());
    }
    return box;
}

boost::optional<datasource::geometry_t> python_datasource::get_geometry_type() const
{
    typedef boost::optional<datasource::geometry_t> return_type;

    try
    {
        ensure_gil lock;
        // if the datasource object has no geometry_type attribute, return a 'none' value
        if (!PyObject_HasAttrString(datasource_.ptr(), "geometry_type"))
        {
            return return_type();
        }
        boost::python::object py_geometry_type = datasource_.attr("geometry_type");
        // if the attribute value is 'None', return a 'none' value
        if (py_geometry_type.ptr() == boost::python::object().ptr())
        {
            return return_type();
        }
        long geom_type_integer = boost::python::extract<long>(py_geometry_type);
        return datasource::geometry_t(geom_type_integer);
    }
    catch ( boost::python::error_already_set )
    {
        //throw datasource_exception(extractException());
    }
}

featureset_ptr python_datasource::features(query const& q) const
{
    try
    {
        // if the query box intersects our world extent then query for features
        if (envelope().intersects(q.get_bbox()))
        {
            ensure_gil lock;
            boost::python::object features(datasource_.attr("features")(q));
            // if 'None' was returned, return an empty feature set
            if(features.ptr() == boost::python::object().ptr())
            {
                return featureset_ptr();
            }
            return boost::make_shared<python_featureset>(features);
        }
        // otherwise return an empty featureset pointer
        return featureset_ptr();
    }
    catch ( boost::python::error_already_set )
    {
        //throw datasource_exception(extractException());
    }
}

featureset_ptr python_datasource::features_at_point(coord2d const& pt, double tol) const
{

    try
    {
        ensure_gil lock;
        boost::python::object features(datasource_.attr("features_at_point")(pt));
        // if we returned none, return an empty set
        if(features.ptr() == boost::python::object().ptr())
        {
            return featureset_ptr();
        }
        // otherwise, return a feature set which can iterate over the iterator
        return boost::make_shared<python_featureset>(features);
    }
    catch ( boost::python::error_already_set )
    {
        //throw datasource_exception(extractException());
    }

}


python_featureset::python_featureset(boost::python::object iterator)
{
    ensure_gil lock;
    begin_ = boost::python::stl_input_iterator<feature_ptr>(iterator);
}

python_featureset::~python_featureset()
{
    ensure_gil lock;
    begin_ = end_;
}

feature_ptr python_featureset::next()
{
    // checking to see if we've reached the end does not require the GIL.
    if(begin_ == end_)
        return feature_ptr();

    // getting the next feature might call into the interpreter and so the GIL must be held.
    ensure_gil lock;

    return *(begin_++);
}

} // end namespace mapnik

namespace {
boost::shared_ptr<mapnik::datasource> create_python_datasource(boost::python::object ds)
{
    return mapnik::datasource_ptr(new mapnik::python_datasource(ds));
}

} // end anonymous namespace

void export_python_datasource()
{
    using namespace boost::python;

    class_<mapnik::python_datasource,boost::shared_ptr<mapnik::python_datasource>,
        boost::noncopyable>("PythonDatasource",
            "This class represents a python datasource", no_init);
		
		def("Python", &create_python_datasource);
}
