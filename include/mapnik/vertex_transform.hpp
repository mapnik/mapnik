/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
//$Id: vertex_transform.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef VERTEX_TRANSFORM_HPP
#define VERTEX_TRANSFORM_HPP

// mapnik
#include <mapnik/envelope.hpp>
#include <mapnik/vertex.hpp>

namespace mapnik
{
    template <typename T0 ,typename T1,int shift=8>
    struct Shift
    {
        typedef T0 value_type;
        typedef T1 return_type;
        static return_type apply(value_type val)
        {
            return static_cast<return_type>(val*(1<<shift));
        }
    };

    template <typename T0,typename T1>
    struct Shift<T0,T1,0> 
    {
        typedef T0 value_type;
        typedef T1 return_type;
        static return_type apply(value_type val)
        {
            return static_cast<return_type>(val);
        }
    };

    template <typename T>
    struct Shift<T,T,0>
    {
        typedef T value_type;
        typedef T return_type;
        static T& apply(T& val)
        {
            return val;
        }
    };

    typedef Shift<double,double,0> NO_SHIFT;
    typedef Shift<double,int,0> SHIFT0;
    typedef Shift<double,int,8> SHIFT8;
    
    
    template <typename T0,typename T1,typename Trans>
    struct view_transform;
    
    template <typename Trans>
    struct view_transform <vertex2d,vertex2d,Trans>   
    {
	
    };
    
    template <typename Trans>
    struct view_transform <vertex2d,vertex2i,Trans>   
    {
	
    };

    template <typename Trans>
    struct view_transform<Envelope<double>,Envelope<double>,Trans>
    {
	
    };
}

#endif //VERTEX_TRANSFORM_HPP
