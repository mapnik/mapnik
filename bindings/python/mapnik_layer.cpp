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

#include <mapnik/config.hpp>

// boost
#include "boost_std_shared_shim.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/layer.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>

using mapnik::layer;
using mapnik::parameters;
using mapnik::datasource_cache;


struct layer_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const layer& l)
    {
        return boost::python::make_tuple(l.name(),l.srs());
    }

    static  boost::python::tuple
    getstate(const layer& l)
    {
        boost::python::list s;
        std::vector<std::string> const& style_names = l.styles();
        for (unsigned i = 0; i < style_names.size(); ++i)
        {
            s.append(style_names[i]);
        }
        return boost::python::make_tuple(l.clear_label_cache(),l.min_zoom(),l.max_zoom(),l.queryable(),l.datasource()->params(),l.cache_features(),s);
    }

    static void
    setstate (layer& l, boost::python::tuple state)
    {
        using namespace boost::python;
        if (len(state) != 9)
        {
            PyErr_SetObject(PyExc_ValueError,
                            ("expected 9-item tuple in call to __setstate__; got %s"
                             % state).ptr()
                );
            throw_error_already_set();
        }

        l.set_clear_label_cache(extract<bool>(state[0]));

        l.set_min_zoom(extract<double>(state[1]));

        l.set_max_zoom(extract<double>(state[2]));

        l.set_queryable(extract<bool>(state[3]));

        mapnik::parameters params = extract<parameters>(state[4]);
        l.set_datasource(datasource_cache::instance().create(params));

        boost::python::list s = extract<boost::python::list>(state[5]);
        for (int i=0;i<len(s);++i)
        {
            l.add_style(extract<std::string>(s[i]));
        }

        l.set_cache_features(extract<bool>(state[6]));
    }
};

std::vector<std::string> & (mapnik::layer::*_styles_)() = &mapnik::layer::styles;

void set_maximum_extent(mapnik::layer & l, boost::optional<mapnik::box2d<double> > const& box)
{
    if (box)
    {
        l.set_maximum_extent(*box);
    }
    else
    {
        l.reset_maximum_extent();
    }
}

void set_buffer_size(mapnik::layer & l, boost::optional<int> const& buffer_size)
{
    if (buffer_size)
    {
        l.set_buffer_size(*buffer_size);
    }
    else
    {
        l.reset_buffer_size();
    }
}

PyObject * get_buffer_size(mapnik::layer & l)
{
    boost::optional<int> buffer_size = l.buffer_size();
    if (buffer_size)
    {
#if PY_VERSION_HEX >= 0x03000000
        return PyLong_FromLong(*buffer_size);
#else
        return PyInt_FromLong(*buffer_size);
#endif
    }
    else
    {
        Py_RETURN_NONE;
    }
}

void export_layer()
{
    using namespace boost::python;
    class_<std::vector<std::string> >("Names")
        .def(vector_indexing_suite<std::vector<std::string>,true >())
        ;

    class_<layer>("Layer", "A Mapnik map layer.", init<std::string const&,optional<std::string const&> >(
                      "Create a Layer with a named string and, optionally, an srs string.\n"
                      "\n"
                      "The srs can be either a Proj.4 epsg code ('+init=epsg:<code>') or\n"
                      "of a Proj.4 literal ('+proj=<literal>').\n"
                      "If no srs is specified it will default to '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs'\n"
                      "\n"
                      "Usage:\n"
                      ">>> from mapnik import Layer\n"
                      ">>> lyr = Layer('My Layer','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')\n"
                      ">>> lyr\n"
                      "<mapnik._mapnik.Layer object at 0x6a270>\n"
                      ))

        .def_pickle(layer_pickle_suite())

        .def("envelope",&layer::envelope,
             "Return the geographic envelope/bounding box."
             "\n"
             "Determined based on the layer datasource.\n"
             "\n"
             "Usage:\n"
             ">>> from mapnik import Layer\n"
             ">>> lyr = Layer('My Layer','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')\n"
             ">>> lyr.envelope()\n"
             "box2d(-1.0,-1.0,0.0,0.0) # default until a datasource is loaded\n"
            )

        .def("visible", &layer::visible,
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
             ">>> lyr = Layer('My Layer','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')\n"
             ">>> lyr.visible(1.0/1000000)\n"
             "True\n"
             ">>> lyr.active = False\n"
             ">>> lyr.visible(1.0/1000000)\n"
             "False\n"
            )

        .add_property("active",
                      &layer::active,
                      &layer::set_active,
                      "Get/Set whether this layer is active and will be rendered (same as status property).\n"
                      "\n"
                      "Usage:\n"
                      ">>> from mapnik import Layer\n"
                      ">>> lyr = Layer('My Layer','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')\n"
                      ">>> lyr.active\n"
                      "True # Active by default\n"
                      ">>> lyr.active = False # set False to disable layer rendering\n"
                      ">>> lyr.active\n"
                      "False\n"
            )

        .add_property("status",
                      &layer::active,
                      &layer::set_active,
                      "Get/Set whether this layer is active and will be rendered.\n"
                      "\n"
                      "Usage:\n"
                      ">>> from mapnik import Layer\n"
                      ">>> lyr = Layer('My Layer','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')\n"
                      ">>> lyr.status\n"
                      "True # Active by default\n"
                      ">>> lyr.status = False # set False to disable layer rendering\n"
                      ">>> lyr.status\n"
                      "False\n"
            )

        .add_property("clear_label_cache",
                      &layer::clear_label_cache,
                      &layer::set_clear_label_cache,
                      "Get/Set whether to clear the label collision detector cache for this layer during rendering\n"
                      "\n"
                      "Usage:\n"
                      ">>> lyr.clear_label_cache\n"
                      "False # False by default, meaning label positions from other layers will impact placement \n"
                      ">>> lyr.clear_label_cache = True # set to True to clear the label collision detector cache\n"
            )

        .add_property("cache_features",
                      &layer::cache_features,
                      &layer::set_cache_features,
                      "Get/Set whether features should be cached during rendering if used between multiple styles\n"
                      "\n"
                      "Usage:\n"
                      ">>> lyr.cache_features\n"
                      "False # False by default\n"
                      ">>> lyr.cache_features = True # set to True to enable feature caching\n"
            )

        .add_property("datasource",
                      &layer::datasource,
                      &layer::set_datasource,
                      "The datasource attached to this layer.\n"
                      "\n"
                      "Usage:\n"
                      ">>> from mapnik import Layer, Datasource\n"
                      ">>> lyr = Layer('My Layer','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')\n"
                      ">>> lyr.datasource = Datasource(type='shape',file='world_borders')\n"
                      ">>> lyr.datasource\n"
                      "<mapnik.Datasource object at 0x65470>\n"
            )

        .add_property("buffer_size",
                      &get_buffer_size,
                      &set_buffer_size,
                      "Get/Set the size of buffer around layer in pixels.\n"
                      "\n"
                      "Usage:\n"
                      ">>> print(l.buffer_size)\n"
                      "None # None by default\n"
                      ">>> l.buffer_size = 2\n"
                      ">>> l.buffer_size\n"
                      "2\n"
            )

        .add_property("maximum_extent",make_function
                      (&layer::maximum_extent,return_value_policy<copy_const_reference>()),
                      &set_maximum_extent,
                      "The maximum extent of the map.\n"
                      "\n"
                      "Usage:\n"
                      ">>> m.maximum_extent = Box2d(-180,-90,180,90)\n"
            )

        .add_property("maxzoom",
                      &layer::max_zoom,
                      &layer::set_max_zoom,
                      "Get/Set the maximum zoom lever of the layer.\n"
                      "\n"
                      "Usage:\n"
                      ">>> from mapnik import Layer\n"
                      ">>> lyr = Layer('My Layer','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')\n"
                      ">>> lyr.maxzoom\n"
                      "1.7976931348623157e+308 # default is the numerical maximum\n"
                      ">>> lyr.maxzoom = 1.0/1000000\n"
                      ">>> lyr.maxzoom\n"
                      "9.9999999999999995e-07\n"
            )

        .add_property("minzoom",
                      &layer::min_zoom,
                      &layer::set_min_zoom,
                      "Get/Set the minimum zoom lever of the layer.\n"
                      "\n"
                      "Usage:\n"
                      ">>> from mapnik import Layer\n"
                      ">>> lyr = Layer('My Layer','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')\n"
                      ">>> lyr.minzoom # default is 0\n"
                      "0.0\n"
                      ">>> lyr.minzoom = 1.0/1000000\n"
                      ">>> lyr.minzoom\n"
                      "9.9999999999999995e-07\n"
            )

        .add_property("name",
                      make_function(&layer::name, return_value_policy<copy_const_reference>()),
                      &layer::set_name,
                      "Get/Set the name of the layer.\n"
                      "\n"
                      "Usage:\n"
                      ">>> from mapnik import Layer\n"
                      ">>> lyr = Layer('My Layer','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')\n"
                      ">>> lyr.name\n"
                      "'My Layer'\n"
                      ">>> lyr.name = 'New Name'\n"
                      ">>> lyr.name\n"
                      "'New Name'\n"
            )

        .add_property("queryable",
                      &layer::queryable,
                      &layer::set_queryable,
                      "Get/Set whether this layer is queryable.\n"
                      "\n"
                      "Usage:\n"
                      ">>> from mapnik import layer\n"
                      ">>> lyr = layer('My layer','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')\n"
                      ">>> lyr.queryable\n"
                      "False # Not queryable by default\n"
                      ">>> lyr.queryable = True\n"
                      ">>> lyr.queryable\n"
                      "True\n"
            )

        .add_property("srs",
                      make_function(&layer::srs,return_value_policy<copy_const_reference>()),
                      &layer::set_srs,
                      "Get/Set the SRS of the layer.\n"
                      "\n"
                      "Usage:\n"
                      ">>> from mapnik import layer\n"
                      ">>> lyr = layer('My layer','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')\n"
                      ">>> lyr.srs\n"
                      "'+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs' # The default srs if not initialized with custom srs\n"
                      ">>> # set to google mercator with Proj.4 literal\n"
                      "... \n"
                      ">>> lyr.srs = '+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over'\n"
            )

        .add_property("group_by",
                      make_function(&layer::group_by,return_value_policy<copy_const_reference>()),
                      &layer::set_group_by,
                      "Get/Set the optional layer group name.\n"
                      "\n"
                      "More details at https://github.com/mapnik/mapnik/wiki/Grouped-rendering:\n"
            )

        .add_property("styles",
                      make_function(_styles_,return_value_policy<reference_existing_object>()),
                      "The styles list attached to this layer.\n"
                      "\n"
                      "Usage:\n"
                      ">>> from mapnik import layer\n"
                      ">>> lyr = layer('My layer','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs')\n"
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
        // comparison
        .def(self == self)
        ;
}
