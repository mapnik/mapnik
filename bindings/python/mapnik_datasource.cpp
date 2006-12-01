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
//$Id$
// boost
#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>
// stl
#include <sstream>
// mapnik
#include <mapnik/envelope.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/feature_layer_desc.hpp>

namespace  
{
    //user-friendly wrapper that uses Python dictionary
    using namespace boost::python;
    boost::shared_ptr<mapnik::datasource> create_datasource(const dict& d)
    {
        mapnik::parameters params;
        boost::python::list keys=d.keys();
        for (int i=0; i<len(keys); ++i)
        {
            std::string key = extract<std::string>(keys[i]);
            object obj = d[key];
            extract<std::string> ex(obj);
            if (ex.check())
            {
                params[key] = ex();
            }            
        }
        
        return mapnik::datasource_cache::create(params);
    }
    
    std::string describe(boost::shared_ptr<mapnik::datasource> const& ds)
    {
        std::stringstream ss;
        if (ds)
        {
            ss << ds->get_descriptor() << "\n";
        }
        else
        {
            ss << "Null\n";
        }
        return ss.str();
    }
}

inline object pass_through(object const& o) { return o; }

inline mapnik::feature_ptr next(mapnik::featureset_ptr const& itr)
{
    mapnik::feature_ptr f = itr->next();
    if (!f)
    {
        PyErr_SetString(PyExc_StopIteration, "No more features.");
        boost::python::throw_error_already_set();
    }
    return f; 
}

void export_datasource()
{
    using namespace boost::python;
    using mapnik::datasource;
    using mapnik::Featureset;
    using mapnik::Feature;

    class_<Feature,boost::shared_ptr<Feature>,
        boost::noncopyable>("Feature",no_init)
        .def("id",&Feature::id)
        .def("__str__",&Feature::to_string)
        ;
    
    class_<Featureset,boost::shared_ptr<Featureset>,
        boost::noncopyable>("Datasource",no_init)
        .def("next",next)
        .def("__iter__",pass_through)
        ;
    
    class_<datasource,boost::shared_ptr<datasource>,
        boost::noncopyable>("Datasource",no_init)
        .def("envelope",&datasource::envelope)
        .def("descriptor",&datasource::get_descriptor) //todo
        .def("features",&datasource::features)
        .def("features_at_point",&datasource::features_at_point)
        .def("params",&datasource::params,return_value_policy<copy_const_reference>(), 
             "The configuration parameters of the data source. "  
             "These vary depending on the type of data source.")
        ;
    
    def("Describe",&describe);
    def("CreateDatasource",&create_datasource);
}
