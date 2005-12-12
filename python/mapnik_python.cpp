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

//$Id: mapnik_python.cc 27 2005-03-30 21:45:40Z pavlenko $

#include <boost/python.hpp>
#include <boost/get_pointer.hpp>
#include <boost/python/detail/api_placeholder.hpp>

#include "mapnik.hpp"
#include "image_symbolizer.hpp"

using namespace mapnik;

void export_color();
void export_layer();
void export_parameters();
void export_envelope();
void export_image();
void export_map();
void export_python();
void export_filter();
void export_rule();
void export_style();
void export_stroke();
void export_datasource_cache();

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


boost::shared_ptr<symbolizer> create_point_symbolizer(std::string const& file,unsigned w,unsigned h)
{
    return boost::shared_ptr<symbolizer>(new image_symbolizer(file,"png",w,h));
}

boost::shared_ptr<symbolizer> create_line_symbolizer(const Color& pen,float width)
{
    return boost::shared_ptr<symbolizer>(new line_symbolizer(pen,width));
} 

boost::shared_ptr<symbolizer> create_line_symbolizer2(stroke const& strk)
{
    return boost::shared_ptr<symbolizer>(new line_symbolizer(strk));
} 

boost::shared_ptr<symbolizer> create_polygon_symbolizer(const Color& fill) 
{   
    return boost::shared_ptr<symbolizer>(new polygon_symbolizer(fill));
} 

boost::shared_ptr<symbolizer> create_polygon_symbolizer2(std::string const& file,unsigned w,unsigned h) 
{   
    return boost::shared_ptr<symbolizer>(new pattern_symbolizer(file,"png",w,h));
} 

BOOST_PYTHON_MODULE(_mapnik)
{
    using namespace boost::python;
    
    class_<datasource,boost::shared_ptr<datasource>,
	boost::noncopyable>("datasource",no_init)
        .def("envelope",&datasource::envelope,
	     return_value_policy<reference_existing_object>())
        ;
    
    class_<symbolizer,boost::noncopyable> ("symbolizer_",no_init) 
    	;
    class_<boost::shared_ptr<symbolizer>,
	boost::noncopyable>("symbolizer",no_init)
	;
    export_parameters();
    export_color(); 
    export_envelope();   
    export_image();
    export_filter();
    export_rule();
    export_style();    
    export_layer();
    export_stroke();
    export_datasource_cache();
    
    
    class_<coord<double,2> >("coord",init<double,double>())
        .def_readwrite("x", &coord<double,2>::x)
        .def_readwrite("y", &coord<double,2>::y)
        ;

    export_map();
  
    def("render_to_file",&render_to_file);
    def("render",&render);
    def("point_symbolizer",&create_point_symbolizer);
    def("line_symbolizer",&create_line_symbolizer);
    def("line_symbolizer",&create_line_symbolizer2);
    def("polygon_symbolizer",&create_polygon_symbolizer);
    def("polygon_symbolizer",&create_polygon_symbolizer2);
    register_ptr_to_python<boost::shared_ptr<symbolizer> >();
    register_ptr_to_python<filter_ptr>();
}
