/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon, Dane Springmeyer
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


// boost
#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

// mapnik
#include <mapnik/layer.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>

using mapnik::Layer;
using mapnik::parameters;
using mapnik::datasource_cache;


struct layer_pickle_suite : boost::python::pickle_suite
{
   static boost::python::tuple
   getinitargs(const Layer& l)
   {
      return boost::python::make_tuple(l.name(),l.srs());
   }

   static  boost::python::tuple
   getstate(const Layer& l)
   {
        boost::python::list s;
        std::vector<std::string> const& style_names = l.styles();
        for (unsigned i = 0; i < style_names.size(); ++i)
        {
            s.append(style_names[i]);
        }      
        return boost::python::make_tuple(l.abstract(),l.title(),l.clear_label_cache(),l.getMinZoom(),l.getMaxZoom(),l.isQueryable(),l.datasource()->params(),s);
   }

   static void
   setstate (Layer& l, boost::python::tuple state)
   {
        using namespace boost::python;
        if (len(state) != 8)
        {
         PyErr_SetObject(PyExc_ValueError,
                         ("expected 8-item tuple in call to __setstate__; got %s"
                          % state).ptr()
            );
         throw_error_already_set();
        }

        if (state[0])
        {
            l.set_abstract(extract<std::string>(state[0]));
        }

        if (state[1])
        {
            l.set_title(extract<std::string>(state[1]));
        }

        if (state[2])
        {
            l.set_clear_label_cache(extract<bool>(state[2]));
        }

        if (state[3])
        {
            l.setMinZoom(extract<double>(state[3]));
        }

        if (state[4])
        {
            l.setMaxZoom(extract<double>(state[4]));
        }

        if (state[5])
        {
            l.setQueryable(extract<bool>(state[5]));
        }

        if (state[6])
        {
            mapnik::parameters params = extract<parameters>(state[6]);
            l.set_datasource(datasource_cache::instance()->create(params));
        }
        
        boost::python::list s = extract<boost::python::list>(state[7]);
        for (int i=0;i<len(s);++i)
        {
           l.add_style(extract<std::string>(s[i]));
        }
   }
};

std::vector<std::string> & (mapnik::Layer::*_styles_)() = &mapnik::Layer::styles;

void export_layer()
{
    using namespace boost::python;
    class_<std::vector<std::string> >("Names")
    	.def(vector_indexing_suite<std::vector<std::string>,true >())
    	;
    
    class_<Layer>("Layer", "A Mapnik map layer.", init<std::string const&,optional<std::string const&> >(
                "Create a Layer with a named string and, optionally, an srs string.\n"
                "\n"
                "The srs can be either a Proj.4 epsg code ('+init=epsg:<code>') or\n"
                "of a Proj.4 literal ('+proj=<literal>').\n"
                "If no srs is specified it will default to '+proj=latlong +datum=WGS84'\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr\n"
                "<mapnik._mapnik.Layer object at 0x6a270>\n"
                ))

        .def_pickle(layer_pickle_suite())
         
        .def("envelope",&Layer::envelope, 
                "Return the geographic envelope/bounding box."
                "\n"
                "Determined based on the layer datasource.\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr.envelope()\n"
                "Envelope(-1.0,-1.0,0.0,0.0) # default until a datasource is loaded\n"
                )
        
        .def("visible", &Layer::isVisible,
                "Return True if this layer's data is active and visible at a given scale.\n"
                "\n"
                "Otherwise returns False.\n"
                "Accepts a scale value as an integer or float input.\n"
                "Will return False if:\n"
                "\tscale >= minzoom - 1e-6\n"
                "\tor:\n"
                "\tscale < maxzoom + 1e-6\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr.visible(1.0/1000000)\n"
                "True\n"
                ">>> lyr.active = False\n"
                ">>> lyr.visible(1.0/1000000)\n"
                "False\n"
                )
                
        .add_property("abstract", 
                make_function(&Layer::abstract,return_value_policy<copy_const_reference>()),
                &Layer::set_abstract,
                "Get/Set the abstract of the layer.\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr.abstract\n"
                "'' # default is en empty string\n"
                ">>> lyr.abstract = 'My Shapefile rendered with Mapnik'\n"
                ">>> lyr.abstract\n"
                "'My Shapefile rendered with Mapnik'\n"
                )

        .add_property("active",
                &Layer::isActive,
                &Layer::setActive,
                "Get/Set whether this layer is active and will be rendered.\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr.active\n"
                "True # Active by default\n"
                ">>> lyr.active = False # set False to disable layer rendering\n"
                ">>> lyr.active\n"
                "False\n"
                )
                
        .add_property("clear_label_cache",
                &Layer::clear_label_cache,
                &Layer::set_clear_label_cache,
                "Get/Set whether this layer's labels are cached.\n"
                "\n"
                "Usage:\n"
                "TODO\n" 
                )
        
        .add_property("datasource",
                &Layer::datasource,
                &Layer::set_datasource,
                "The datasource attached to this layer.\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer, Datasource\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr.datasource = Datasource(type='shape',file='world_borders')\n"
                ">>> lyr.datasource\n"
                "<mapnik.Datasource object at 0x65470>\n"
                )

        .add_property("maxzoom",
                &Layer::getMaxZoom,
                &Layer::setMaxZoom,
                "Get/Set the maximum zoom lever of the layer.\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr.maxzoom\n"
                "1.7976931348623157e+308 # default is the numerical maximum\n"
                ">>> lyr.maxzoom = 1.0/1000000\n"
                ">>> lyr.maxzoom\n"
                "9.9999999999999995e-07\n"
                )
        
        .add_property("minzoom",
                &Layer::getMinZoom,
                &Layer::setMinZoom,
                "Get/Set the minimum zoom lever of the layer.\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr.minzoom # default is 0\n"
                "0.0\n"
                ">>> lyr.minzoom = 1.0/1000000\n"
                ">>> lyr.minzoom\n"
                "9.9999999999999995e-07\n"
                )     

        .add_property("name", 
                make_function(&Layer::name, return_value_policy<copy_const_reference>()),
                &Layer::set_name,
                "Get/Set the name of the layer.\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr.name\n"
                "'My Layer'\n"
                ">>> lyr.name = 'New Name'\n"
                ">>> lyr.name\n"
                "'New Name'\n"
                )

        .add_property("queryable",
                &Layer::isQueryable,
                &Layer::setQueryable,
                "Get/Set whether this layer is queryable.\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr.queryable\n"
                "False # Not queryable by default\n"
                ">>> lyr.queryable = True\n"
                ">>> lyr.queryable\n"
                "True\n"
                )

        .add_property("srs", 
                make_function(&Layer::srs,return_value_policy<copy_const_reference>()),
                &Layer::set_srs,
                "Get/Set the SRS of the layer.\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr.srs\n"
                "'+proj=latlong +datum=WGS84' # The default srs if not initialized with custom srs\n"
                ">>> # set to google mercator with Proj.4 literal\n"
                "... \n"
                ">>> lyr.srs = '+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over'\n"
                )

        .add_property("styles",
                make_function(_styles_,return_value_policy<reference_existing_object>()),
                "The styles list attached to this layer.\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr.styles\n"
                "<mapnik._mapnik.Names object at 0x6d3e8>\n"
                ">>> len(lyr.styles)\n"
                "0\n # no styles until you append them\n"
                "lyr.styles.append('My Style') # mapnik uses named styles for flexibility\n"
                ">>> len(lyr.styles)\n"
                "1\n"
                ">>> lyr.styles[0]\n"
                "'My Style'\n"
                )
                                                            
        .add_property("title",
                make_function(&Layer::title, return_value_policy<copy_const_reference>()),
                &Layer::set_title,
                "Get/Set the title of the layer.\n"
                "\n"
                "Usage:\n"
                ">>> from mapnik import Layer\n"
                ">>> lyr = Layer('My Layer','+proj=latlong +datum=WGS84')\n"
                ">>> lyr.title\n"
                "''\n"
                ">>> lyr.title = 'My first layer'\n"
                ">>> lyr.title\n"
                "'My first layer'\n"
                )
 
        ;
}
