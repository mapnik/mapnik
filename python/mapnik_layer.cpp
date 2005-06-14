/* This file is part of python_mapnik (c++/python mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: mapnik_layer.cc 17 2005-03-08 23:58:43Z pavlenko $


#include <mapnik.hpp>
#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>

using mapnik::Layer;
using mapnik::Parameters;

struct layer_pickle_suite : boost::python::pickle_suite
{
    
    static boost::python::tuple
    getinitargs(const Layer& l)
    {    
	using namespace boost::python;
        return boost::python::make_tuple(l.params());
    }
    static boost::python::tuple
    getstate(const Layer& l)
    {
	using namespace boost::python;
	
	std::vector<std::string> const& styles=l.styles();
	std::vector<std::string>::const_iterator itr=styles.begin();
	list py_styles;

	while (itr!=styles.end())
	{
	    py_styles.append(*itr++);
	}
	
	return boost::python::make_tuple(l.getMinZoom(),
			  l.getMaxZoom(),
			  py_styles);
    }

    static void
    setstate (Layer& l, boost::python::tuple state)
    {
	using namespace boost::python;
	if (len(state) != 3)
	{
	    PyErr_SetObject(PyExc_ValueError,
			    ("expected 3-item tuple in call to __setstate__; got %s"
			     % state).ptr()
			    );
	    throw_error_already_set();
	}

	l.setMinZoom(extract<double>(state[0]));
	l.setMaxZoom(extract<double>(state[1]));

	list styles=extract<list>(state[2]);
	for (int i=0;i<len(styles);++i)
	{
	    l.add_style(extract<std::string>(styles[i]));
	}
    }
};

namespace  
{
    //user-friendly wrapper that uses Python dictionary
    using namespace boost::python;
    Layer create_layer(const dict& d)
    {
	Parameters params;
	list keys=d.keys();
	for (int i=0;i<len(keys);++i)
	{
	    std::string key=extract<std::string>(keys[i]);
            std::string value=extract<std::string>(d[key]);
	    std::cout<<key<<":"<<value<<std::endl;
	    params.add(key,value);
	}
	return Layer(params);
    }
}

void export_layer()
{
    using namespace boost::python;
    class_<Layer>("layer",init<const Parameters&>("Layer constructor"))
	//class_<Layer>("layer",no_init)
        .def("name",&Layer::name,return_value_policy<copy_const_reference>())
        .def("params",&Layer::params,return_value_policy<reference_existing_object>())
        .def("envelope",&Layer::envelope,return_value_policy<reference_existing_object>())
	.def("minzoom",&Layer::setMinZoom)
	.def("maxzoom",&Layer::setMaxZoom)
	.def("style",&Layer::add_style)
        .def_pickle(layer_pickle_suite())
        ;
    def("create_layer",&create_layer);
}
