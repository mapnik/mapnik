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
#include <mapnik/image_util.hpp>
#include <mapnik/polygon_pattern_symbolizer.hpp>
#include "mapnik_enumeration.hpp"
#include <mapnik/parse_path.hpp>

using namespace mapnik;
using mapnik::polygon_pattern_symbolizer;
using mapnik::path_expression_ptr;
using mapnik::path_processor_type;
using mapnik::guess_type;

struct polygon_pattern_symbolizer_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const polygon_pattern_symbolizer& p)
    {
        std::string filename = path_processor_type::to_string(*p.get_filename());
        return boost::python::make_tuple(filename,guess_type(filename));
    }

    static boost::python::tuple
    getstate(const polygon_pattern_symbolizer& p)
    {
        return boost::python::make_tuple(p.get_alignment());
    }

    static void
    setstate (polygon_pattern_symbolizer& p, boost::python::tuple state)
    {
        using namespace boost::python;
        if (len(state) != 1)
        {
            PyErr_SetObject(PyExc_ValueError,
                            ("expected 1-item tuple in call to __setstate__; got %s"
                             % state).ptr()
                );
            throw_error_already_set();
        }
                
        p.set_alignment(extract<pattern_alignment_e>(state[0]));
    }

};

void export_polygon_pattern_symbolizer()
{
    using namespace boost::python;

    enumeration_<pattern_alignment_e>("pattern_alignment")
        .value("LOCAL",LOCAL_ALIGNMENT)
        .value("GLOBAL",GLOBAL_ALIGNMENT)
        ;
    
    class_<polygon_pattern_symbolizer>("PolygonPatternSymbolizer",
                                       init<path_expression_ptr>("<path_expression_ptr>"))
        .def_pickle(polygon_pattern_symbolizer_pickle_suite())
        .add_property("pattern_alignment",
              &polygon_pattern_symbolizer::get_alignment,
              &polygon_pattern_symbolizer::set_alignment,
              "Set/get the alignment of the pattern")

        ;    
}
