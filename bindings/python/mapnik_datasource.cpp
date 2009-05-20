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
#include <vector>

// mapnik
#include <mapnik/envelope.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/memory_datasource.hpp>


using mapnik::datasource;
using mapnik::point_datasource;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;

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
            extract<std::string> ex0(obj);
            extract<int> ex1(obj);
            extract<double> ex2(obj);
            
            if (ex0.check())
            {
               params[key] = ex0();
            }
            else if (ex1.check())
            {
               params[key] = ex1();
            }
            else if (ex2.check())
            {
               params[key] = ex2();
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
    
    std::string encoding(boost::shared_ptr<mapnik::datasource> const& ds)
    {
            layer_descriptor ld = ds->get_descriptor();
            return ld.get_encoding();
    }

    std::string name(boost::shared_ptr<mapnik::datasource> const& ds)
    {
            layer_descriptor ld = ds->get_descriptor();
            return ld.get_name();
    }

    boost::python::list fields(boost::shared_ptr<mapnik::datasource> const& ds)
    {
        boost::python::list flds;
        if (ds)
        {
            layer_descriptor ld = ds->get_descriptor();
            std::vector<attribute_descriptor> const& desc_ar = ld.get_descriptors();
            std::vector<attribute_descriptor>::const_iterator it = desc_ar.begin();
            std::vector<attribute_descriptor>::const_iterator end = desc_ar.end();
            for (; it != end; ++it)
            {
               flds.append(it->get_name());
            }
        }
        return flds;
    }
    boost::python::list field_types(boost::shared_ptr<mapnik::datasource> const& ds)
    {
        boost::python::list fld_types;
        if (ds)
        {
            layer_descriptor ld = ds->get_descriptor();
            std::vector<attribute_descriptor> const& desc_ar = ld.get_descriptors();
            std::vector<attribute_descriptor>::const_iterator it = desc_ar.begin();
            std::vector<attribute_descriptor>::const_iterator end = desc_ar.end();
            for (; it != end; ++it)
            {  
               unsigned type = it->get_type();
               fld_types.append(type);
            }
        }
        return fld_types;
    }}

void export_datasource()
{
    using namespace boost::python;
    
    class_<datasource,boost::shared_ptr<datasource>,
        boost::noncopyable>("Datasource",no_init)
        .def("envelope",&datasource::envelope)
        .def("descriptor",&datasource::get_descriptor) //todo
        .def("features",&datasource::features)
        .def("fields",&fields)
        .def("_field_types",&field_types)
        .def("encoding",&encoding) //todo expose as property
        .def("name",&name)
        .def("features_at_point",&datasource::features_at_point)
        .def("params",&datasource::params,return_value_policy<copy_const_reference>(), 
             "The configuration parameters of the data source. "  
             "These vary depending on the type of data source.")
        ;
    
    def("Describe",&describe);
    def("CreateDatasource",&create_datasource);

    class_<point_datasource, bases<datasource>, boost::noncopyable>("PointDatasource", init<>())
        .def("add_point",&point_datasource::add_point)
        ;
}
