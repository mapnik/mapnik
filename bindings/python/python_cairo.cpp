/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2008 Tom Hughes
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

#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)

#include <boost/python/type_id.hpp>
#include <boost/python/converter/registry.hpp>

#include <pycairo.h>

static Pycairo_CAPI_t *Pycairo_CAPI;

static void *extract_surface(PyObject* op)
{
    if (PyObject_TypeCheck(op, const_cast<PyTypeObject*>(Pycairo_CAPI->Surface_Type)))
    {
        return op;
    }
    else
    {
        return 0;
    }
}

static void *extract_context(PyObject* op)
{
    if (PyObject_TypeCheck(op, const_cast<PyTypeObject*>(Pycairo_CAPI->Context_Type)))
    {
        return op;
    }
    else
    {
        return 0;
    }
}

void register_cairo()
{
    Pycairo_CAPI = (Pycairo_CAPI_t*) PyCObject_Import(const_cast<char *>("cairo"), const_cast<char *>("CAPI"));
    if (Pycairo_CAPI == NULL) return;

    boost::python::converter::registry::insert(&extract_surface, boost::python::type_id<PycairoSurface>());
    boost::python::converter::registry::insert(&extract_context, boost::python::type_id<PycairoContext>());
}

#else

void register_cairo()
{
}

#endif
