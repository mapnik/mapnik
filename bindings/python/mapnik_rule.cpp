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
#include <mapnik/expression_string.hpp>

using mapnik::rule;
using mapnik::expr_node;
using mapnik::expression_ptr;
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
using mapnik::glyph_symbolizer;
using mapnik::symbolizer;
using mapnik::to_expression_string;

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
    extract_symbolizer( rule& r): 
        r_(r) {}
        
    template <typename T>
    void operator () ( T const& sym )
    {
        r_.append(sym);
    }
private:
    rule& r_;
    
};

struct rule_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const rule& r)
    {
        return boost::python::make_tuple(r.get_name(),r.get_title(),r.get_min_scale(),r.get_max_scale());
    }

    static  boost::python::tuple
    getstate(const rule& r)
    {
        boost::python::list syms;
        
        rule::symbolizers::const_iterator begin = r.get_symbolizers().begin();
        rule::symbolizers::const_iterator end = r.get_symbolizers().end();        
        pickle_symbolizer serializer( syms );
        std::for_each( begin, end , boost::apply_visitor( serializer ));
        
        // We serialize filter expressions AST as strings
        std::string filter_expr = to_expression_string(*r.get_filter());
        
        return boost::python::make_tuple(r.get_abstract(),filter_expr,r.has_else_filter(),r.has_also_filter(),syms);
    }

    static void
    setstate (rule& r, boost::python::tuple state)
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
            rule dfl;
            std::string filter = extract<std::string>(state[1]);
            std::string default_filter = "<TODO>";//dfl.get_filter()->to_string();
            if ( filter != default_filter)
            {
                r.set_filter(mapnik::parse_expression(filter,"utf8"));
            }
        }    

        if (state[2])
        {
            r.set_else(true);
        }    

        if (state[3])
        {
            r.set_also(true);
        }
        
        boost::python::list syms=extract<boost::python::list>(state[4]);
        extract_symbolizer serializer( r );
        for (int i=0;i<len(syms);++i)
        {
            //symbolizer symbol = extract<symbolizer>(syms[i]);
            //boost::apply_visitor( serializer, symbol );
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
    implicitly_convertible<glyph_symbolizer,symbolizer>();
    implicitly_convertible<markers_symbolizer,symbolizer>();
    
    class_<rule::symbolizers>("Symbolizers",init<>("TODO"))
        .def(vector_indexing_suite<rule::symbolizers>())
        ;
    
    class_<rule>("Rule",init<>("default constructor"))
        .def(init<std::string const&,
             boost::python::optional<std::string const&,double,double> >())
        .def_pickle(rule_pickle_suite())
        .add_property("name",make_function
                      (&rule::get_name,
                       return_value_policy<copy_const_reference>()),
                      &rule::set_name)
        .add_property("title",make_function
                      (&rule::get_title,return_value_policy<copy_const_reference>()),
                      &rule::set_title)
        .add_property("abstract",make_function
                      (&rule::get_abstract,return_value_policy<copy_const_reference>()),
                      &rule::set_abstract)
        .add_property("filter",make_function
                      (&rule::get_filter,return_value_policy<copy_const_reference>()),
                      &rule::set_filter)
        .add_property("min_scale",&rule::get_min_scale,&rule::set_min_scale)
        .add_property("max_scale",&rule::get_max_scale,&rule::set_max_scale)
        .def("set_else",&rule::set_else)
        .def("has_else",&rule::has_else_filter)
        .def("set_also",&rule::set_also)
        .def("has_also",&rule::has_also_filter)
        .def("active",&rule::active)
        .add_property("symbols",make_function
                      (&rule::get_symbolizers,return_value_policy<reference_existing_object>()))
        ;
}

