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

//$Id$

#include <boost/get_pointer.hpp>
#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>

#include "mapnik.hh"
#include "polygon_symbolizer.hh"
#include "line_symbolizer.hh"
#include "image_symbolizer.hh"


void export_color();
void export_layer();
void export_parameters();
void export_envelope();
void export_image();
void export_map();

using namespace mapnik;

namespace boost
{
    namespace python
    {
	
	template <typename T,
		  template <typename T> class DeallocPolicy>
	T* get_pointer(ref_ptr< T , DeallocPolicy> const& ptr)
	{
	    return ( T* )ptr.get();
	}

        template <typename T>	    
	struct pointee<ref_ptr<T> > 
	{
	    typedef T type;
	};
	
	template <> struct pointee<ref_ptr<datasource,datasource_delete> >
	{
	    typedef datasource type;
	};
    }
}

void render_to_file(const Map& map,const std::string& file,const std::string& format)
{
    Image32 image(map.getWidth(),map.getHeight());
    Renderer<Image32>::render(map,image);
    image.saveToFile(file,format);
}

void render(const Map& map,Image32& image)
{
    Renderer<Image32>::render(map,image);    
}

//BEGIN quick hack 
ref_ptr<Symbolizer> create_point_symbolizer(const std::string& file,unsigned w,unsigned h)
{
    return ref_ptr<Symbolizer>(new ImageSymbolizer(file,"png",w,h));
}

ref_ptr<Symbolizer> create_line_symbolizer(const Color& stroke,double minScale,double maxScale)
{
    return ref_ptr<Symbolizer>(new LineSymbolizer(stroke,minScale,maxScale));
} 

ref_ptr<Symbolizer> create_polygon_symbolizer(const Color& stroke,const Color& fill,double minScale,double maxScale) 
{   
    return ref_ptr<Symbolizer>(new PolygonSymbolizer(fill,minScale,maxScale));
} 

//END


BOOST_PYTHON_MODULE(mapnik)
{
    using namespace boost::python;

    class_<datasource,ref_ptr<datasource,datasource_delete>,
	boost::noncopyable>("datasource",no_init)
        .def("envelope",&datasource::envelope,
	     return_value_policy<reference_existing_object>())
        ;

    export_parameters();
    export_color(); 
    export_envelope();   
    export_image();

    class_<Style>("style",init<>("Style default constructor"))
	.def(init<ref_ptr<Symbolizer> >())
	.def("add",&Style::add)
	;

    class_<Symbolizer,boost::noncopyable> ("symbolizer",no_init) 
    	;
 
    export_layer();

    class_<singleton<datasource_cache,CreateStatic>,boost::noncopyable>("singleton",no_init)
        .def("instance",&singleton<datasource_cache,CreateStatic>::instance,
	     return_value_policy<reference_existing_object>())
        .staticmethod("instance")
        ;

    class_<datasource_cache,bases<singleton<datasource_cache,CreateStatic> >,
        boost::noncopyable>("datasource_cache",no_init)
        .def("create",&datasource_cache::create)
        .staticmethod("create")
        ;

    class_<singleton<style_cache,CreateStatic>,boost::noncopyable>("singleton",no_init)
	.def("instance",&singleton<style_cache,CreateStatic>::instance,
	     return_value_policy<reference_existing_object>())
	.staticmethod("instance")
	;

    class_<style_cache,bases<singleton<style_cache,CreateStatic> >,
	boost::noncopyable>("style_cache",no_init)
	.def("insert",&style_cache::insert)
	.staticmethod("insert")
	.def("remove",&style_cache::remove)
	.staticmethod("remove")
	;

    class_<coord<double,2> >("coord",init<double,double>())
        .def_readwrite("x", &coord<double,2>::x)
        .def_readwrite("y", &coord<double,2>::y)
        ;

    export_map();
  
    def("render_to_file",&render_to_file);
    def("render",&render);
    def("create_point_symbolizer",&create_point_symbolizer);
    def("create_line_symbolizer",&create_line_symbolizer);
    def("create_polygon_symbolizer",&create_polygon_symbolizer);
    register_ptr_to_python<ref_ptr<Symbolizer> >();
}
