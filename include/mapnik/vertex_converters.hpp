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

// boost
#include <boost/type_traits/is_same.hpp>

// mpl
#include <boost/mpl/begin_end.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/int.hpp>

// fusion
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/container/vector.hpp>

// mapnik
#include <mapnik/agg_helpers.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/offset_converter.hpp>
#include <mapnik/simplify_converter.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/polygon_clipper.hpp>

// agg
#include "agg_conv_clip_polygon.h"
#include "agg_conv_clip_polyline.h"
#include "agg_conv_close_polygon.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_conv_transform.h"

// stl
#include <stdexcept>
#include <array>

namespace mapnik {

struct transform_tag {};
struct clip_line_tag {};
struct clip_poly_tag {};
struct close_poly_tag {};
struct smooth_tag {};
struct simplify_tag {};
struct stroke_tag {};
struct dash_tag {};
struct affine_transform_tag {};
struct offset_transform_tag {};

namespace  detail {

template <typename T0, typename T1>
struct converter_traits
{
    typedef T0 geometry_type;
    typedef geometry_type conv_type;
    template <typename Args>
    static void setup(geometry_type & , Args const& )
    {
        throw std::runtime_error("invalid call to setup");
    }
};

template <typename T>
struct converter_traits<T,mapnik::smooth_tag>
{
    typedef T geometry_type;
    typedef typename agg::conv_smooth_poly1_curve<geometry_type> conv_type;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        geom.smooth_value(boost::fusion::at_c<2>(args).smooth());
    }
};

template <typename T>
struct converter_traits<T,mapnik::simplify_tag>
{
    typedef T geometry_type;
    typedef simplify_converter<geometry_type> conv_type;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        geom.set_simplify_algorithm(boost::fusion::at_c<2>(args).simplify_algorithm());
        geom.set_simplify_tolerance(boost::fusion::at_c<2>(args).simplify_tolerance());
    }
};

template <typename T>
struct converter_traits<T, mapnik::clip_line_tag>
{
    typedef T geometry_type;
    typedef typename agg::conv_clip_polyline<geometry_type> conv_type;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        typename boost::mpl::at<Args,boost::mpl::int_<0> >::type box = boost::fusion::at_c<0>(args);
        geom.clip_box(box.minx(),box.miny(),box.maxx(),box.maxy());
    }
};


template <typename T>
struct converter_traits<T, mapnik::dash_tag>
{
    typedef T geometry_type;
    typedef typename agg::conv_dash<geometry_type> conv_type;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        typename boost::mpl::at<Args,boost::mpl::int_<2> >::type sym = boost::fusion::at_c<2>(args);
        double scale_factor = boost::fusion::at_c<6>(args);
        stroke const& stroke_ = sym.get_stroke();
        dash_array const& d = stroke_.get_dash_array();
        dash_array::const_iterator itr = d.begin();
        dash_array::const_iterator end = d.end();
        for (;itr != end;++itr)
        {
            geom.add_dash(itr->first * scale_factor,
                          itr->second * scale_factor);
        }

    }
};

template <typename T>
struct converter_traits<T, mapnik::stroke_tag>
{
    typedef T geometry_type;
    typedef typename agg::conv_stroke<geometry_type> conv_type;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        typename boost::mpl::at<Args,boost::mpl::int_<2> >::type sym = boost::fusion::at_c<2>(args);
        stroke const& stroke_ = sym.get_stroke();
        set_join_caps(stroke_,geom);
        geom.generator().miter_limit(stroke_.get_miterlimit());
        double scale_factor = boost::fusion::at_c<6>(args);
        geom.generator().width(stroke_.get_width() * scale_factor);
    }
};


template <typename T>
struct converter_traits<T,mapnik::clip_poly_tag>
{
    typedef T geometry_type;
    typedef mapnik::polygon_clipper<geometry_type> conv_type;
    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        typename boost::mpl::at<Args,boost::mpl::int_<0> >::type box = boost::fusion::at_c<0>(args);
        geom.set_clip_box(box);
    }
};

template <typename T>
struct converter_traits<T,mapnik::close_poly_tag>
{
    typedef T geometry_type;
    typedef typename agg::conv_close_polygon<geometry_type> conv_type;
    template <typename Args>
    static void setup(geometry_type & , Args const&)
    {
        // no-op
    }
};

template <typename T>
struct converter_traits<T,mapnik::transform_tag>
{
    typedef T geometry_type;
    typedef coord_transform<CoordTransform, geometry_type> conv_type;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        geom.set_proj_trans(boost::fusion::at_c<4>(args));
        geom.set_trans(boost::fusion::at_c<3>(args));
    }
};


template <typename T>
struct converter_traits<T,mapnik::affine_transform_tag>
{
    typedef T geometry_type;
    typedef agg::conv_transform<geometry_type, agg::trans_affine const>
        conv_base_type;

    struct conv_type : public conv_base_type
    {
        conv_type(geometry_type& geom)
            : conv_base_type(geom, agg::trans_affine::identity) {}
    };

    template <typename Args>
    static void setup(geometry_type & geom, Args & args)
    {
        geom.transformer(boost::fusion::at_c<5>(args));
    }
};

template <typename T>
struct converter_traits<T,mapnik::offset_transform_tag>
{
    typedef T geometry_type;
    typedef offset_converter<geometry_type> conv_type;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        typename boost::mpl::at<Args,boost::mpl::int_<2> >::type sym = boost::fusion::at_c<2>(args);
        double scale_factor = boost::fusion::at_c<6>(args);
        geom.set_offset(sym.offset()*scale_factor);
    }
};

template <bool>
struct converter_fwd
{
    template <typename Base, typename T0,typename T1,typename T2, typename Iter,typename End>
    static void forward(Base& base, T0 & geom,T1 const& args)
    {
        typedef T0 geometry_type;
        typedef T2 conv_tag;
        typedef typename detail::converter_traits<geometry_type,conv_tag>::conv_type conv_type;
        conv_type conv(geom);
        detail::converter_traits<conv_type,conv_tag>::setup(conv,args);
        base.template dispatch<Iter,End>(conv, typename boost::is_same<Iter,End>::type());
    }
};

template <>
struct converter_fwd<true>
{
    template <typename Base, typename T0,typename T1,typename T2, typename Iter,typename End>
    static void forward(Base& base, T0 & geom,T1 const& args)
    {
        base.template dispatch<Iter,End>(geom, typename boost::is_same<Iter,End>::type());
    }
};

template <typename A, typename C>
struct dispatcher
{
    typedef dispatcher this_type;
    typedef A args_type;
    typedef C conv_types;

    dispatcher(args_type const& args)
        : args_(args)
    {
        std::memset(&vec_[0], 0,  sizeof(unsigned)*vec_.size());
    }

    template <typename Iter, typename End, typename Geometry>
    void dispatch(Geometry & geom, boost::mpl::true_)
    {
        boost::fusion::at_c<1>(args_).add_path(geom);
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
                template forward<this_type,Geometry,args_type,conv_tag,Next,End>(*this,geom,args_);
        }
        else
        {
            converter_fwd<boost::mpl::true_::value>::
                template forward<this_type,Geometry,args_type,conv_tag,Next,End>(*this,geom,args_);
        }

    }

    template <typename Geometry>
    void apply(Geometry & geom)
    {
        typedef typename boost::mpl::begin<conv_types>::type begin;
        typedef typename boost::mpl::end  <conv_types>::type end;
        dispatch<begin,end,Geometry>(geom, boost::false_type());
    }

    std::array<unsigned, boost::mpl::size<conv_types>::value> vec_;
    args_type args_;
};
}



template <typename B, typename R, typename S, typename T, typename P, typename A, typename C >
struct vertex_converter : private mapnik::noncopyable
{
    typedef C conv_types;
    typedef B bbox_type;
    typedef R rasterizer_type;
    typedef S symbolizer_type;
    typedef T trans_type;
    typedef P proj_trans_type;
    typedef A affine_trans_type;
    typedef typename boost::fusion::vector
    <
    bbox_type const&,
    rasterizer_type&,
    symbolizer_type const&,
    trans_type const&,
    proj_trans_type const&,
    affine_trans_type const&,
    double //scale-factor
    > args_type;

    vertex_converter(bbox_type const& b, rasterizer_type & ras,
                     symbolizer_type const& sym, trans_type & tr,
                     proj_trans_type const& prj_trans,
                     affine_trans_type const& affine_trans,
                     double scale_factor)
        : disp_(args_type(boost::cref(b), boost::ref(ras),
                          boost::cref(sym), boost::cref(tr),
                          boost::cref(prj_trans),
                          boost::cref(affine_trans),
                          scale_factor)) {}

    template <typename Geometry>
    void apply(Geometry & geom)
    {
        typedef Geometry geometry_type;
        disp_.template apply<geometry_type>(geom);
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

    template <typename Conv>
    void unset()
    {
        typedef typename boost::mpl::find<conv_types,Conv>::type iter;
        typedef typename boost::mpl::end<conv_types>::type end;
        std::size_t index = boost::mpl::distance<iter,end>::value - 1;
        if (index < disp_.vec_.size())
            disp_.vec_[index]=0;
    }


    detail::dispatcher<args_type,conv_types> disp_;
};

}

#endif // MAPNIK_VERTEX_CONVERTERS_HPP
