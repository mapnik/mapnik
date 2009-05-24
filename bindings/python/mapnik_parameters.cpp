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
//$Id: mapnik_parameters.cpp 17 2005-03-08 23:58:43Z pavlenko $

// boost
#include <boost/python.hpp>

// mapnik
#include <mapnik/params.hpp>

using mapnik::parameter;
using mapnik::parameters;

struct pickle_value : public boost::static_visitor<>
{
    public:
        pickle_value( boost::python::list vals): 
        vals_(vals) {}
            
        void operator () ( int val )
        {
            vals_.append(val);
        }
        
        void operator () ( double val )
        {
            vals_.append(val);
        }

        void operator () ( std::string val )
        {
            vals_.append(val);
        }
        
    private:
        boost::python::list vals_;

};

struct parameter_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const parameter& p)
    {
        using namespace boost::python;
        return boost::python::make_tuple(p.first,boost::get<std::string>(p.second));
    }
};

struct parameters_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getstate(const parameters& p)
    {
        using namespace boost::python;
        dict d;
        parameters::const_iterator pos=p.begin();
        while(pos!=p.end())
        {
            boost::python::list vals;
            pickle_value serializer( vals );
            mapnik::value_holder val = pos->second;
            boost::apply_visitor( serializer, val );
            d[pos->first] = vals[0];
            ++pos;
        }
        return boost::python::make_tuple(d);
    }

    static void setstate(parameters& p, boost::python::tuple state)
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
        
        dict d = extract<dict>(state[0]);
        boost::python::list keys = d.keys();
        for (int i=0; i<len(keys); ++i)
        {
            std::string key = extract<std::string>(keys[i]);
            object obj = d[key];
            extract<std::string> ex0(obj);
            extract<int> ex1(obj);
            extract<double> ex2(obj);
            
            if (ex0.check())
            {
               p[key] = ex0();
            }
            else if (ex1.check())
            {
               p[key] = ex1();
            }
            else if (ex2.check())
            {
               p[key] = ex2();
            }           
            
            /*
            extract_value serializer( p, key );
            mapnik::value_holder val = extract<mapnik::value_holder>(d[key]);
            boost::apply_visitor( serializer, val );
            */
        }        
    }
};

boost::python::dict dict_params(parameters& p)
{
    boost::python::dict d;
    parameters::const_iterator pos=p.begin();
    while(pos!=p.end())
    {
        boost::python::list vals;
        pickle_value serializer( vals );
        mapnik::value_holder val = pos->second;
        boost::apply_visitor( serializer, val );
        d[pos->first] = vals[0];
        ++pos;
    }
    return d;
}

boost::python::list list_params(parameters& p)
{
    boost::python::list l;
    parameters::const_iterator pos=p.begin();
    while(pos!=p.end())
    {
        boost::python::list vals;
        pickle_value serializer( vals );
        mapnik::value_holder val = pos->second;
        boost::apply_visitor( serializer, val );
        l.append(boost::python::make_tuple(pos->first,vals[0]));
        ++pos;
    }
    return l;
}

boost::python::dict dict_param(parameter& p)
{
    boost::python::dict d;
    d[p.first] = boost::get<std::string>(p.second);
    return d;
}

boost::python::tuple tuple_param(parameter& p)
{
    return boost::python::make_tuple(p.first,boost::get<std::string>(p.second));
}

void export_parameters()
{
    using namespace boost::python;
    class_<parameter>("Parameter",init<std::string,std::string>())
        .def_pickle(parameter_pickle_suite())
        .def("as_dict",dict_param)
        .def("as_tuple",tuple_param)
        ;

    class_<parameters>("Parameters",init<>())
        .def_pickle(parameters_pickle_suite())
        .def("as_dict",dict_params)
        .def("as_list",list_params)
        ;
}