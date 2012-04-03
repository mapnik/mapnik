/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef MAPNIK_VERTEX_CONVERTERS_HPP
#define MAPNIK_VERTEX_CONVERTERS_HPP

#include <boost/type_traits/is_same.hpp>

#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/begin_end.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/size.hpp>

// fusion
//#include <boost/fusion/container/vector.hpp>
//#include <boost/fusion/adapted/mpl.hpp>

#include <boost/foreach.hpp>
#include <boost/utility.hpp>
#include <boost/array.hpp>

// agg
#include "agg_conv_clip_polygon.h"
#include "agg_conv_clip_polyline.h"
#include "agg_conv_smooth_poly1.h"


namespace mapnik {

struct transform {};
struct clip_line {};
struct clip_poly {};
struct smooth {};

namespace  detail {

template <typename T0, typename T1>
struct converter_traits
{
    typedef T0 geometry_type;
    typedef geometry_type conv_type;
    template <typename Symbolizer>
    static void setup(geometry_type & geom,Symbolizer const& sym) 
    {
        throw "BOOM!";
    }
};

template <typename T>
struct converter_traits<T,mapnik::smooth>
{
    typedef T geometry_type;   
    typedef typename agg::conv_smooth_poly1_curve<geometry_type> conv_type;
    
    template <typename Symbolizer>
    static void setup(geometry_type & geom, Symbolizer const& sym) 
    {
        //std::cout << "SETUP:" << typeid(geom).name() << std::endl;        
        geom.smooth_value(sym.smooth());
    }
};

/*
template <typename T>
struct converter_traits<T,mapnik::clip_line>
{
    typedef T geometry_type;   
    typedef typename agg::conv_clip_polyline<geometry_type> conv_type;
    
    template <typename Symbolizer>
    static void setup(geometry_type & geom, Symbolizer const& sym) 
    {
        //std::cout << "SETUP:" << typeid(geom).name() << std::endl;
        geom.clip_box(0,0,100,100);
    }
};
*/

template <typename T>
struct converter_traits<T,mapnik::clip_poly>
{
    typedef T geometry_type;   
    typedef typename agg::conv_clip_polygon<geometry_type> conv_type;
    
    template <typename Symbolizer>
    static void setup(geometry_type & geom, Symbolizer const& sym) 
    {
        //std::cout << "SETUP:" << typeid(geom).name() << std::endl;
        geom.clip_box(0,0,100,100);
    }
};

template <bool>
struct converter_fwd
{
    template <typename Base, typename T0,typename T1,typename T2, typename Iter,typename End>
    static void forward(Base& base, T0 & geom,T1 const& sym)          
    {
        typedef T0 geometry_type;
        typedef T2 conv_tag;
        typedef typename detail::converter_traits<geometry_type,conv_tag>::conv_type conv_type;
        conv_type conv(geom);
        detail::converter_traits<conv_type,conv_tag>::setup(conv, sym);            
        base.template dispatch<Iter,End>(conv, typename boost::is_same<Iter,End>::type());
    }                
};

template <>
struct converter_fwd<true>
{
    template <typename Base, typename T0,typename T1,typename T2, typename Iter,typename End>
    static void forward(Base& base, T0 & geom,T1 const& sym)          
    {
        base.template dispatch<Iter,End>(geom, typename boost::is_same<Iter,End>::type());
    }                
};

template <typename R, typename S, typename C>
struct dispatcher 
{
    typedef dispatcher this_type;    
    typedef S symbolizer_type;
    typedef R rasterizer_type;
    typedef C conv_types;
    dispatcher(rasterizer_type & ras, symbolizer_type const& sym)
        : ras_(ras),
          sym_(sym) 
    {
        std::memset(&vec_[0], 0,  sizeof(unsigned)*vec_.size());
    }
    
    template <typename Iter, typename End, typename Geometry> 
    void dispatch(Geometry & geom, boost::mpl::true_)
    {  
        ras_.add_path(geom);
    }
    
    template <typename Iter, typename End, typename Geometry>
    void dispatch(Geometry & geom, boost::mpl::false_)
    {
        typedef typename boost::mpl::deref<Iter>::type conv_tag;
        typedef typename detail::converter_traits<Geometry,conv_tag>::conv_type conv_type;
        typedef typename boost::mpl::next<Iter>::type Next;
        
        std::size_t index = boost::mpl::distance<Iter,End>::value - 1;
        if (vec_[index] == 1)
        {
            converter_fwd<boost::is_same<Geometry,conv_type>::value>::
                template forward<this_type,Geometry,symbolizer_type,conv_tag,Next,End>(*this,geom,sym_);
        }
        else 
        {
            converter_fwd<boost::mpl::true_::value>::
                template forward<this_type,Geometry,symbolizer_type,conv_tag,Next,End>(*this,geom,sym_);   
        }
        
    }

    template <typename Geometry>
    void apply(Geometry & geom)
    {
        typedef typename boost::mpl::begin<conv_types>::type begin;
        typedef typename boost::mpl::end  <conv_types>::type end;
        dispatch<begin,end,Geometry>(geom, boost::false_type());   
    }

    boost::array<unsigned, boost::mpl::size<conv_types>::value> vec_;
    rasterizer_type & ras_;
    symbolizer_type const& sym_;
    
};
}

template <typename R, typename S, typename C>
struct vertex_converter : private boost::noncopyable
{ 
    typedef C conv_types;
    typedef R rasterizer_type;
    typedef S symbolizer_type;
    vertex_converter(rasterizer_type & ras, symbolizer_type const& sym)
        : disp_(ras,sym) {}
    
    template <typename T>
    void apply(T & geom)
    {   
        typedef T geometry_type;
        //BOOST_FOREACH(geometry_type & geom, cont)
        {           
            disp_.template apply<geometry_type>(geom);
        }
    }
    
    template <typename Conv>
    void set()
    {
        typedef typename boost::mpl::find<conv_types,Conv>::type iter;
        typedef typename boost::mpl::end<conv_types>::type end;
        std::size_t index = boost::mpl::distance<iter,end>::value - 1;
        if (index < disp_.vec_.size())
            disp_.vec_[index]=1;
    }
    
    detail::dispatcher<rasterizer_type,symbolizer_type,conv_types> disp_;
};

}

#endif // MAPNIK_VERTEX_CONVERTERS_HPP
