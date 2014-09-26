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


// mapnik
#include <mapnik/config.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/transform_path_adapter.hpp>
#include <mapnik/offset_converter.hpp>
#include <mapnik/simplify.hpp>
#include <mapnik/simplify_converter.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/symbolizer.hpp>

#include <type_traits>

// boost
//#include <boost/type_traits/is_same.hpp>

// mpl
#include <boost/mpl/begin_end.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/size.hpp>
//#include <boost/mpl/at.hpp>
#include <boost/mpl/int.hpp>

// fusion
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/container/vector.hpp>



// agg
#include "agg_math_stroke.h"
#include "agg_trans_affine.h"
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
    using geometry_type = T0;
    using conv_type = geometry_type;
    template <typename Args>
    static void setup(geometry_type & , Args const& )
    {
        throw std::runtime_error("invalid call to setup");
    }
};

template <typename T>
struct converter_traits<T,mapnik::smooth_tag>
{
    using geometry_type = T;
    using conv_type = typename agg::conv_smooth_poly1_curve<geometry_type>;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        auto const& sym = boost::fusion::at_c<2>(args);
        auto const& feat = boost::fusion::at_c<6>(args);
        auto const& vars = boost::fusion::at_c<7>(args);
        geom.smooth_value(get<value_double>(sym, keys::smooth, feat, vars));
    }
};

template <typename T>
struct converter_traits<T,mapnik::simplify_tag>
{
    using geometry_type = T;
    using conv_type = simplify_converter<geometry_type>;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        auto const& sym = boost::fusion::at_c<2>(args);
        auto const& feat = boost::fusion::at_c<6>(args);
        auto const& vars = boost::fusion::at_c<7>(args);
        geom.set_simplify_algorithm(static_cast<simplify_algorithm_e>(get<value_integer>(sym, keys::simplify_algorithm, feat, vars)));
        geom.set_simplify_tolerance(get<value_double>(sym, keys::simplify_tolerance, feat, vars));
    }
};

template <typename T>
struct converter_traits<T, mapnik::clip_line_tag>
{
    using geometry_type = T;
    using conv_type = typename agg::conv_clip_polyline<geometry_type>;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        auto const& box = boost::fusion::at_c<0>(args);
        geom.clip_box(box.minx(),box.miny(),box.maxx(),box.maxy());
    }
};

template <typename T>
struct converter_traits<T, mapnik::dash_tag>
{
    using geometry_type = T;
    using conv_type = typename agg::conv_dash<geometry_type>;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        auto const& sym = boost::fusion::at_c<2>(args);
        auto const& feat = boost::fusion::at_c<6>(args);
        auto const& vars = boost::fusion::at_c<7>(args);
        double scale_factor = boost::fusion::at_c<8>(args);
        auto dash = get_optional<dash_array>(sym, keys::stroke_dasharray, feat, vars);
        if (dash)
        {
            for (auto const& d : *dash)
            {
                geom.add_dash(d.first * scale_factor,
                              d.second * scale_factor);
            }
        }
    }
};

template <typename Symbolizer, typename PathType, typename Feature>
void set_join_caps(Symbolizer const& sym, PathType & stroke, Feature const& feature, attributes const& vars)
{
    line_join_enum join = get<line_join_enum>(sym, keys::stroke_linejoin, feature, vars, MITER_JOIN);
    switch (join)
    {
    case MITER_JOIN:
        stroke.generator().line_join(agg::miter_join);
        break;
    case MITER_REVERT_JOIN:
        stroke.generator().line_join(agg::miter_join);
        break;
    case ROUND_JOIN:
        stroke.generator().line_join(agg::round_join);
        break;
    default:
        stroke.generator().line_join(agg::bevel_join);
    }

    line_cap_enum cap = get<line_cap_enum>(sym, keys::stroke_linecap, feature, vars, BUTT_CAP);

    switch (cap)
    {
    case BUTT_CAP:
        stroke.generator().line_cap(agg::butt_cap);
        break;
    case SQUARE_CAP:
        stroke.generator().line_cap(agg::square_cap);
        break;
    default:
        stroke.generator().line_cap(agg::round_cap);
    }
}

template <typename T>
struct converter_traits<T, mapnik::stroke_tag>
{
    using geometry_type = T;
    using conv_type = typename agg::conv_stroke<geometry_type>;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        auto const& sym = boost::fusion::at_c<2>(args);
        auto const& feat = boost::fusion::at_c<6>(args);
        auto const& vars = boost::fusion::at_c<7>(args);
        set_join_caps(sym, geom, feat, vars);
        double miterlimit = get<value_double>(sym, keys::stroke_miterlimit, feat, vars, 4.0);
        geom.generator().miter_limit(miterlimit);
        double scale_factor = boost::fusion::at_c<8>(args);
        double width = get<value_double>(sym, keys::stroke_width, feat, vars, 1.0);
        geom.generator().width(width * scale_factor);
    }
};

template <typename T>
struct converter_traits<T,mapnik::clip_poly_tag>
{
    using geometry_type = T;
    using conv_type = typename agg::conv_clip_polygon<geometry_type>;
    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        auto const& box = boost::fusion::at_c<0>(args);
        geom.clip_box(box.minx(),box.miny(),box.maxx(),box.maxy());
        //geom.set_clip_box(box);
    }
};

template <typename T>
struct converter_traits<T,mapnik::close_poly_tag>
{
    using geometry_type = T;
    using conv_type = typename agg::conv_close_polygon<geometry_type>;
    template <typename Args>
    static void setup(geometry_type & , Args const&)
    {
        // no-op
    }
};

template <typename T>
struct converter_traits<T,mapnik::transform_tag>
{
    using geometry_type = T;
    using conv_type = transform_path_adapter<view_transform, geometry_type>;

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
    using geometry_type = T;
    using conv_base_type =  agg::conv_transform<geometry_type, agg::trans_affine const>;
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
    using geometry_type = T;
    using conv_type = offset_converter<geometry_type>;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        auto const& sym = boost::fusion::at_c<2>(args);
        auto const& feat = boost::fusion::at_c<6>(args);
        auto const& vars = boost::fusion::at_c<7>(args);
        double offset = get<value_double>(sym, keys::offset, feat, vars);
        double scale_factor = boost::fusion::at_c<8>(args);
        geom.set_offset(offset * scale_factor);
    }
};

template <bool>
struct converter_fwd
{
    template <typename Base, typename T0,typename T1,typename T2, typename Iter,typename End>
    static void forward(Base& base, T0 & geom,T1 const& args)
    {
        using geometry_type = T0;
        using conv_tag = T2;
        using conv_type = typename detail::converter_traits<geometry_type,conv_tag>::conv_type;
        conv_type conv(geom);
        detail::converter_traits<conv_type,conv_tag>::setup(conv,args);
        base.template dispatch<Iter,End>(conv, typename std::is_same<Iter,End>::type());
    }
};

template <>
struct converter_fwd<true>
{
    template <typename Base, typename T0,typename T1,typename T2, typename Iter,typename End>
    static void forward(Base& base, T0 & geom,T1 const& args)
    {
        base.template dispatch<Iter,End>(geom, typename std::is_same<Iter,End>::type());
    }
};

template <typename A, typename C>
struct dispatcher
{
    using this_type = dispatcher;
    using args_type = A;
    using conv_types = C;

    dispatcher(args_type const& args)
        : args_(args)
    {
        std::fill(vec_.begin(), vec_.end(), 0);
    }

    template <typename Iter, typename End, typename Geometry>
    void dispatch(Geometry & geom, std::true_type)
    {
        boost::fusion::at_c<1>(args_).add_path(geom);
    }

    template <typename Iter, typename End, typename Geometry>
    void dispatch(Geometry & geom, std::false_type)
    {
        using conv_tag = typename boost::mpl::deref<Iter>::type;
        using conv_type = typename detail::converter_traits<Geometry,conv_tag>::conv_type;
        using Next = typename boost::mpl::next<Iter>::type;

        std::size_t index = boost::mpl::distance<Iter,End>::value - 1;
        if (vec_[index] == 1)
        {
            converter_fwd<std::is_same<Geometry,conv_type>::value>::
                template forward<this_type,Geometry,args_type,conv_tag,Next,End>(*this,geom,args_);
        }
        else
        {
            dispatch<Next,End>(geom, typename std::is_same<Next,End>::type());
        }
    }

    template <typename Geometry>
    void apply(Geometry & geom)
    {
        using begin = typename boost::mpl::begin<conv_types>::type;
        using end = typename boost::mpl::end  <conv_types>::type;
        dispatch<begin,end,Geometry>(geom, std::false_type());
    }

    std::array<unsigned, boost::mpl::size<conv_types>::value> vec_;
    args_type args_;
};

template <typename Dispatcher, typename Converter, typename... ConverterTypes>
struct converter_setter;

template <typename Dispatcher, typename Converter, typename Current, typename... ConverterTypes>
struct converter_setter<Dispatcher,Converter,Current,ConverterTypes...>
{
    static void set(Dispatcher & disp, int state)
    {
        if (std::is_same<Converter,Current>::value)
        {
            std::size_t index = sizeof...(ConverterTypes) ;
            disp.vec_[index] = state;
        }
        else
        {
            converter_setter<Dispatcher,Converter,ConverterTypes...>::set(disp, state);
        }
    }
};

template <typename Dispatcher, typename Converter>
struct converter_setter<Dispatcher,Converter>
{
    static void set(Dispatcher & disp, int state) {}
};

}

template <typename R, typename... C >
struct vertex_converter : private mapnik::noncopyable
{
    using conv_types = boost::mpl::vector<C...>;
    using bbox_type = box2d<double>;
    using rasterizer_type = R;
    using symbolizer_type = symbolizer_base;
    using trans_type = view_transform;
    using proj_trans_type = proj_transform;
    using affine_trans_type = agg::trans_affine;
    using feature_type = feature_impl;
    using args_type = typename boost::fusion::vector<
    bbox_type const&,
    rasterizer_type&,
    symbolizer_type const&,
    trans_type const&,
    proj_trans_type const&,
    affine_trans_type const&,
    feature_type const&,
    attributes const&,
    double //scale-factor
    >;

    using dispatcher_type = detail::dispatcher<args_type,conv_types>;

    vertex_converter(bbox_type const& b,
                     rasterizer_type & ras,
                     symbolizer_type const& sym,
                     trans_type const& tr,
                     proj_trans_type const& prj_trans,
                     affine_trans_type const& affine_trans,
                     feature_type const& feature,
                     attributes const& vars,
                     double scale_factor)
        : disp_(args_type(std::cref(b),
                          std::ref(ras),
                          std::cref(sym),
                          std::cref(tr),
                          std::cref(prj_trans),
                          std::cref(affine_trans),
                          std::cref(feature),
                          std::cref(vars),
                          scale_factor)) {}

    template <typename Geometry>
    void apply(Geometry & geom)
    {
        using geometry_type = Geometry;
        disp_.template apply<geometry_type>(geom);
    }

    template <typename Conv>
    void set()
    {
        detail::converter_setter<dispatcher_type, Conv,C...>::set(disp_, 1);
    }

    template <typename Conv>
    void unset()
    {
        detail::converter_setter<dispatcher_type, Conv, C...>::set(disp_, 0);
    }

    dispatcher_type disp_;
};

}

#endif // MAPNIK_VERTEX_CONVERTERS_HPP
