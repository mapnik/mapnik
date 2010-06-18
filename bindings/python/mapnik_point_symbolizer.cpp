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

#include <boost/python.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/point_symbolizer.hpp>
#include <mapnik/parse_path.hpp>

using mapnik::point_symbolizer;
using mapnik::symbolizer_with_image;
using mapnik::path_processor_type;

struct point_symbolizer_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const point_symbolizer& p)
    {
        std::string filename = path_processor_type::to_string(*p.get_filename());
        return boost::python::make_tuple(filename,mapnik::guess_type(filename));
    }

    static  boost::python::tuple
    getstate(const point_symbolizer& p)
    {
        return boost::python::make_tuple(p.get_allow_overlap(),p.get_opacity());
    }

    static void
    setstate (point_symbolizer& p, boost::python::tuple state)
    {
        using namespace boost::python;
        if (len(state) != 2)
        {
            PyErr_SetObject(PyExc_ValueError,
                            ("expected 2-item tuple in call to __setstate__; got %s"
                             % state).ptr()
                );
            throw_error_already_set();
        }
                
        p.set_allow_overlap(extract<bool>(state[0]));
        p.set_opacity(extract<float>(state[1]));
        
    }

};

namespace  
{ 
using namespace boost::python;
const std::string get_filename(mapnik::point_symbolizer& symbolizer) 
{ 
    return path_processor_type::to_string(*symbolizer.get_filename()); 
} 

}

void export_point_symbolizer()
{
    using namespace boost::python;
    
    class_<point_symbolizer>("PointSymbolizer",
                             init<>("Default Point Symbolizer - 4x4 black square"))
        .def (init<mapnik::path_expression_ptr>("<path expression ptr>"))
        //.def_pickle(point_symbolizer_pickle_suite())
        .add_property("filename",
                      // DS - Using workaround as the normal make_function does not work for unknown reasons...
                      //make_function(&point_symbolizer::get_filename,return_value_policy<copy_const_reference>()),
                      get_filename,
                      &point_symbolizer::set_filename)   
        .add_property("allow_overlap",
                      &point_symbolizer::get_allow_overlap,
                      &point_symbolizer::set_allow_overlap)
        .add_property("opacity",
                      &point_symbolizer::get_opacity,
                      &point_symbolizer::set_opacity)
        ;
}
