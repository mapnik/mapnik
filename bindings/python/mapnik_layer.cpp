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
//$Id: mapnik_layer.cc 17 2005-03-08 23:58:43Z pavlenko $


#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <mapnik/layer.hpp>

using mapnik::Layer;
using mapnik::parameters;

void export_layer()
{
    using namespace boost::python;
    class_<std::vector<std::string> >("Styles")
    	.def(vector_indexing_suite<std::vector<std::string>,true >())
    	;
    
    class_<Layer>("Layer", "A map layer.", init<std::string const&,optional<std::string const&> >())
        .add_property("name", 
                      make_function(&Layer::name, return_value_policy<copy_const_reference>()),
                      &Layer::set_name,
                      "Get/Set the name of the layer.")
        
        .add_property("title",
                      make_function(&Layer::title, return_value_policy<copy_const_reference>()),
                      &Layer::set_title,
                      "Get/Set the title of the layer.")
 
        .add_property("abstract", 
                      make_function(&Layer::abstract,return_value_policy<copy_const_reference>()),
                      &Layer::set_abstract,
                      "Get/Set the abstract of the layer.")
        
        .add_property("srs", 
                      make_function(&Layer::srs,return_value_policy<copy_const_reference>()),
                      &Layer::set_srs,
                      "Get/Set the SRS of the layer.")
        
        .add_property("minzoom",
                      &Layer::getMinZoom,
                      &Layer::setMinZoom)
        
        .add_property("maxzoom",
                      &Layer::getMaxZoom,
                      &Layer::setMaxZoom)
        
        .add_property("styles",
                      make_function(&Layer::styles,
                                    return_value_policy<reference_existing_object>()))

        .add_property("datasource",
                      &Layer::datasource,
                      &Layer::set_datasource,
                      "The datasource attached to this layer")

        .add_property("active",
                      &Layer::isActive,
                      &Layer::setActive)
        
        .def("envelope",&Layer::envelope, 
             "Return the geographic envelope/bounding box "
             "of the data in the layer.")
        
        ;
}
