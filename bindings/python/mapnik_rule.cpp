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
#include <boost/python/implicit.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <mapnik/rule.hpp>
#include <mapnik/filter_factory.hpp>

using mapnik::rule_type;
using mapnik::filter;
using mapnik::filter_ptr;
using mapnik::filter_factory;
using mapnik::Feature;
using mapnik::point_symbolizer;
using mapnik::line_symbolizer;
using mapnik::line_pattern_symbolizer;
using mapnik::polygon_symbolizer;
using mapnik::polygon_pattern_symbolizer;
using mapnik::raster_symbolizer;
using mapnik::shield_symbolizer;
using mapnik::text_symbolizer;
using mapnik::building_symbolizer;
using mapnik::markers_symbolizer;
using mapnik::symbolizer;
using mapnik::symbolizers;

struct pickle_symbolizer : public boost::static_visitor<>
{
public:
    pickle_symbolizer( boost::python::list syms): 
        syms_(syms) {}

    template <typename T>
    void operator () ( T const& sym )
    {
	syms_.append(sym);
    }
    
private:
    boost::python::list syms_;
};


struct extract_symbolizer : public boost::static_visitor<>
{
public:
    extract_symbolizer( rule_type& r): 
        r_(r) {}
        
    template <typename T>
    void operator () ( T const& sym )
    {
	r_.append(sym);
    }
private:
    rule_type& r_;
    
};

struct rule_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const rule_type& r)
    {
	return boost::python::make_tuple(r.get_name(),r.get_title(),r.get_min_scale(),r.get_max_scale());
    }

    static  boost::python::tuple
    getstate(const rule_type& r)
    {
        boost::python::list syms;
        
        symbolizers::const_iterator begin = r.get_symbolizers().begin();
        symbolizers::const_iterator end = r.get_symbolizers().end();        
        pickle_symbolizer serializer( syms );
        std::for_each( begin, end , boost::apply_visitor( serializer ));
        
        // Here the filter string is used rather than the actual Filter object
        // Need to look into how to get the Filter object
        std::string filter_expr = r.get_filter()->to_string();
        return boost::python::make_tuple(r.get_abstract(),filter_expr,r.has_else_filter(),syms);
    }

    static void
    setstate (rule_type& r, boost::python::tuple state)
    {
        using namespace boost::python;
        if (len(state) != 4)
        {
            PyErr_SetObject(PyExc_ValueError,
			    ("expected 4-item tuple in call to __setstate__; got %s"
			     % state).ptr()
		);
            throw_error_already_set();
        }
                
        if (state[0])
        {
            r.set_title(extract<std::string>(state[0]));
        }    

        if (state[1])
        {
            rule_type dfl;
            std::string filter = extract<std::string>(state[1]);
            std::string default_filter = dfl.get_filter()->to_string();
            if ( filter != default_filter)
            {
                r.set_filter(mapnik::create_filter(filter,"utf8"));
            }
        }    

        if (state[2])
        {
            r.set_else(true);
        }    
        
        boost::python::list syms=extract<boost::python::list>(state[3]);
        extract_symbolizer serializer( r );
        for (int i=0;i<len(syms);++i)
        {
            symbolizer symbol = extract<symbolizer>(syms[i]);
            boost::apply_visitor( serializer, symbol );
        }        
    }

};

void export_rule()
{
    using namespace boost::python;
    implicitly_convertible<point_symbolizer,symbolizer>();
    implicitly_convertible<line_symbolizer,symbolizer>();
    implicitly_convertible<line_pattern_symbolizer,symbolizer>();
    implicitly_convertible<polygon_symbolizer,symbolizer>();
    implicitly_convertible<building_symbolizer,symbolizer>();
    implicitly_convertible<polygon_pattern_symbolizer,symbolizer>();
    implicitly_convertible<raster_symbolizer,symbolizer>();
    implicitly_convertible<shield_symbolizer,symbolizer>();
    implicitly_convertible<text_symbolizer,symbolizer>();
    
    class_<symbolizers>("Symbolizers",init<>("TODO"))
    	.def(vector_indexing_suite<symbolizers>())
    	;
    
    class_<rule_type>("Rule",init<>("default constructor"))
        .def(init<std::string const&,
             boost::python::optional<std::string const&,double,double> >())
        .def_pickle(rule_pickle_suite())
        .add_property("name",make_function
                      (&rule_type::get_name,
                       return_value_policy<copy_const_reference>()),
                      &rule_type::set_name)
        .add_property("title",make_function
                      (&rule_type::get_title,return_value_policy<copy_const_reference>()),
                      &rule_type::set_title)
        .add_property("abstract",make_function
                      (&rule_type::get_abstract,return_value_policy<copy_const_reference>()),
                      &rule_type::set_abstract)
        .add_property("filter",make_function
                      (&rule_type::get_filter,return_value_policy<copy_const_reference>()),
                      &rule_type::set_filter)
        .add_property("min_scale",&rule_type::get_min_scale,&rule_type::set_min_scale)
        .add_property("max_scale",&rule_type::get_max_scale,&rule_type::set_max_scale)
        .def("set_else",&rule_type::set_else)
        .def("has_else",&rule_type::has_else_filter)
        .def("active",&rule_type::active)
        .add_property("symbols",make_function
                      (&rule_type::get_symbolizers,return_value_policy<reference_existing_object>()))
        ;
}

