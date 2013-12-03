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

#include <mapnik/noncopyable.hpp>

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
struct python_optional : public mapnik::noncopyable
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
            return 0;
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

    explicit python_optional()
    {
        register_python_conversion<boost::optional<T>,
            optional_to_python, optional_from_python>();
    }
};

// to/from boost::optional<bool>
template <>
struct python_optional<float> : public mapnik::noncopyable
{
    struct optional_to_python
    {
        static PyObject * convert(const boost::optional<float>& value)
        {
            return (value ? PyFloat_FromDouble(*value) :
                    boost::python::detail::none());
        }
    };

    struct optional_from_python
    {
        static void * convertible(PyObject * source)
        {
            using namespace boost::python::converter;

            if (source == Py_None || PyFloat_Check(source))
                return source;
            return 0;
        }

        static void construct(PyObject * source,
                              boost::python::converter::rvalue_from_python_stage1_data * data)
        {
            using namespace boost::python::converter;
            void * const storage = ((rvalue_from_python_storage<boost::optional<bool> > *)
                                    data)->storage.bytes;
            if (source == Py_None)  // == None
                new (storage) boost::optional<float>(); // A Boost uninitialized value
            else
                new (storage) boost::optional<float>(PyFloat_AsDouble(source));
            data->convertible = storage;
        }
    };

    explicit python_optional()
    {
        register_python_conversion<boost::optional<float>,
            optional_to_python, optional_from_python>();
    }
};

// to/from boost::optional<float>
template <>
struct python_optional<bool> : public mapnik::noncopyable
{
    struct optional_to_python
    {
        static PyObject * convert(const boost::optional<bool>& value)
        {
            if (value)
            {
                if (*value) Py_RETURN_TRUE;
                else Py_RETURN_FALSE;
            }
            else return boost::python::detail::none();
        }
    };
    struct optional_from_python
    {
        static void * convertible(PyObject * source)
        {
            using namespace boost::python::converter;

            if (source == Py_None || PyBool_Check(source))
                return source;
            return 0;
        }

        static void construct(PyObject * source,
                              boost::python::converter::rvalue_from_python_stage1_data * data)
        {
            using namespace boost::python::converter;
            void * const storage = ((rvalue_from_python_storage<boost::optional<bool> > *)
                                    data)->storage.bytes;
            if (source == Py_None)  // == None
                new (storage) boost::optional<bool>(); // A Boost uninitialized value
            else
            {
                new (storage) boost::optional<bool>(source == Py_True ? true : false);
            }
            data->convertible = storage;
        }
    };

    explicit python_optional()
    {
        register_python_conversion<boost::optional<bool>,
            optional_to_python, optional_from_python>();
    }
};
