/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2007 Artem Pavlenko
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

#include <boost/optional/optional.hpp>
#include <boost/python.hpp>

// boost::optional<T> to/from converter from John Wiegley

template <typename T, typename TfromPy>
struct object_from_python
{
    object_from_python() {
        boost::python::converter::registry::push_back
            (&TfromPy::convertible, &TfromPy::construct,
             boost::python::type_id<T>());
    }
};

template <typename T, typename TtoPy, typename TfromPy>
struct register_python_conversion
{
    register_python_conversion() {
        boost::python::to_python_converter<T, TtoPy>();
        object_from_python<T, TfromPy>();
    }
};

template <typename T>
struct python_optional : public boost::noncopyable
{
    struct optional_to_python
    {
        static PyObject * convert(const boost::optional<T>& value)
        {
            return (value ? boost::python::to_python_value<T>()(*value) :
                    boost::python::detail::none());
        }
    };

    struct optional_from_python
    {
        static void * convertible(PyObject * source)
        {
            using namespace boost::python::converter;

            if (source == Py_None)
                return source;

            const registration& converters(registered<T>::converters);

            if (implicit_rvalue_convertible_from_python(source,
                                                        converters)) {
                rvalue_from_python_stage1_data data =
                    rvalue_from_python_stage1(source, converters);
                return rvalue_from_python_stage2(source, data, converters);
            }
            return NULL;
        }

        static void construct(PyObject * source,
                              boost::python::converter::rvalue_from_python_stage1_data * data)
        {
            using namespace boost::python::converter;

            void * const storage = ((rvalue_from_python_storage<T> *)
                                    data)->storage.bytes;

            if (data->convertible == source)        // == None
                new (storage) boost::optional<T>(); // A Boost uninitialized value
            else
                new (storage) boost::optional<T>(*static_cast<T *>(data->convertible));

            data->convertible = storage;
        }
    };

    explicit python_optional() {
        register_python_conversion<boost::optional<T>,
            optional_to_python, optional_from_python>();
    }
};

/** This class works around a bug in boost python.

    See http://osdir.com/ml/python.c++/2003-11/msg00158.html
*/
template <typename T, typename X1 = boost::python::detail::not_specified, typename X2 = boost::python::detail::not_specified, typename X3 = boost::python::detail::not_specified>
class class_with_converter : public boost::python::class_<T, X1, X2, X3>
{
public:
    typedef class_with_converter<T,X1,X2,X3> self;
    // Construct with the class name, with or without docstring, and default __init__() function
    class_with_converter(char const* name, char const* doc = 0) : boost::python::class_<T, X1, X2, X3>(name, doc)  { }

    // Construct with class name, no docstring, and an uncallable __init__ function
    class_with_converter(char const* name, boost::python::no_init_t y) : boost::python::class_<T, X1, X2, X3>(name, y) { }

    // Construct with class name, docstring, and an uncallable __init__ function
    class_with_converter(char const* name, char const* doc, boost::python::no_init_t y) : boost::python::class_<T, X1, X2, X3>(name, doc, y) { }

    // Construct with class name and init<> function
    template <class DerivedT> class_with_converter(char const* name, boost::python::init_base<DerivedT> const& i)
        : boost::python::class_<T, X1, X2, X3>(name, i) { }

    // Construct with class name, docstring and init<> function
    template <class DerivedT>
    inline class_with_converter(char const* name, char const* doc, boost::python::init_base<DerivedT> const& i)
        : boost::python::class_<T, X1, X2, X3>(name, doc, i) { }

    template <class D>
    self& def_readwrite_convert(char const* name, D const& d, char const* doc=0)
    {
        this->add_property(name,
                           boost::python::make_getter(d, boost::python::return_value_policy<boost::python::return_by_value>()),
                           boost::python::make_setter(d, boost::python::default_call_policies()));
        return *this;
    }
};
