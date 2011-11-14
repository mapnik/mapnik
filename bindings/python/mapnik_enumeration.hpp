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
#ifndef MAPNIK_PYTHON_BINDING_ENUMERATION_INCLUDED
#define MAPNIK_PYTHON_BINDING_ENUMERATION_INCLUDED

namespace mapnik {

template <typename EnumWrapper>
class enumeration_ :
        public boost::python::enum_<typename EnumWrapper::native_type>
{
    // some short cuts
    typedef boost::python::enum_<typename EnumWrapper::native_type> base_type;
    typedef typename EnumWrapper::native_type native_type;
public:
    enumeration_() :
        base_type( EnumWrapper::get_name().c_str() )
    {
        init();
    }
    enumeration_(const char * python_alias) :
        base_type( python_alias )
    {
        init();
    }
#if BOOST_VERSION >= 103500
    enumeration_(const char * python_alias, const char * doc) :
        base_type( python_alias, doc )
#else
        enumeration_(const char * python_alias, const char * /*doc*/) :
        // Boost.Python < 1.35.0 doesn't support
        // docstrings for enums so we ignore it.
        base_type( python_alias )
#endif
    {
        init();
    }

private:
    struct converter
    {
        static PyObject* convert(EnumWrapper const& v)
        {
            // Redirect conversion to a static method of our base class's
            // base class. A free template converter will not work because
            // the base_type::base typedef is protected.
            // Lets hope MSVC agrees that this is legal C++
            using namespace boost::python::converter;
            return base_type::base::to_python(
                registered<native_type>::converters.m_class_object
                ,  static_cast<long>( v ));

        }
    };

    void init() {
        boost::python::implicitly_convertible<native_type, EnumWrapper>();
        boost::python::to_python_converter<EnumWrapper, converter >();

        for (unsigned i = 0; i < EnumWrapper::MAX; ++i)
        {
            // Register the strings already defined for this enum.
            base_type::value( EnumWrapper::get_string( i ), native_type( i ) );
        }
    }

};

} // end of namespace mapnik

#endif // MAPNIK_PYTHON_BINDING_ENUMERATION_INCLUDED
