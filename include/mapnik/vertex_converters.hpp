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
#include <mapnik/geometry.hpp>
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
#include <type_traits>
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
};

template <typename T>
struct converter_traits<T,mapnik::smooth_tag>
{
    using geometry_type = T;
    using conv_type = typename agg::conv_smooth_poly1_curve<geometry_type>;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        geom.smooth_value(get<value_double, keys::smooth>(args.sym, args.feature, args.vars));
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
        geom.set_simplify_algorithm(get<simplify_algorithm_e,keys::simplify_algorithm>(args.sym, args.feature, args.vars));
        geom.set_simplify_tolerance(get<value_double,keys::simplify_tolerance>(args.sym,args.feature, args.vars));
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
        auto const& box = args.bbox;
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
        auto const& sym = args.sym;
        auto const& feat = args.feature;
        auto const& vars = args.vars;
        double scale_factor = args.scale_factor;
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
    line_join_enum join = get<line_join_enum,keys::stroke_linejoin>(sym, feature, vars);
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

    line_cap_enum cap = get<line_cap_enum,keys::stroke_linecap>(sym, feature, vars);

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
        auto const& sym = args.sym;
        auto const& feat = args.feature;
        auto const& vars = args.vars;
        set_join_caps(sym, geom, feat, vars);
        double miterlimit = get<value_double,keys::stroke_miterlimit>(sym, feat, vars);
        geom.generator().miter_limit(miterlimit);
        double scale_factor = args.scale_factor;
        double width = get<value_double,keys::stroke_width>(sym, feat, vars);
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
        auto const& box = args.bbox;
        geom.clip_box(box.minx(),box.miny(),box.maxx(),box.maxy());
    }
};

template <typename T>
struct converter_traits<T,mapnik::close_poly_tag>
{
    using geometry_type = T;
    using conv_type = typename agg::conv_close_polygon<geometry_type>;
    template <typename Args>
    static void setup(geometry_type & , Args const&) {}
};

template <typename T>
struct converter_traits<T,mapnik::transform_tag>
{
    using geometry_type = T;
    using conv_type = transform_path_adapter<view_transform, geometry_type>;

    template <typename Args>
    static void setup(geometry_type & geom, Args const& args)
    {
        geom.set_proj_trans(args.prj_trans);
        geom.set_trans(args.tr);
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
        geom.transformer(args.affine_trans);
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
        auto const& sym = args.sym;
        auto const& feat = args.feature;
        auto const& vars = args.vars;
        double offset = get<value_double, keys::offset>(sym, feat, vars);
        geom.set_offset(offset * args.scale_factor);
    }
};

template <typename Dispatcher, typename... ConverterTypes>
struct converters_helper;

template <typename Dispatcher, typename Current, typename... ConverterTypes>
struct converters_helper<Dispatcher,Current,ConverterTypes...>
{
    template <typename Converter>
    static void set(Dispatcher & disp, int state)
    {
        if (std::is_same<Converter,Current>::value)
        {
            constexpr std::size_t index = sizeof...(ConverterTypes) ;
            disp.vec_[index] = state;
        }
        else
        {
            converters_helper<Dispatcher,ConverterTypes...>:: template set<Converter>(disp, state);
        }
    }

    template <typename Geometry>
    static void forward(Dispatcher & disp, Geometry & geom)
    {
        constexpr std::size_t index = sizeof...(ConverterTypes);
        if (disp.vec_[index] == 1)
        {
            using conv_type = typename detail::converter_traits<Geometry,Current>::conv_type;
            conv_type conv(geom);
            detail::converter_traits<conv_type,Current>::setup(conv,disp.args_);
            converters_helper<Dispatcher, ConverterTypes...>::forward(disp, conv);
        }
        else
        {
            converters_helper<Dispatcher,ConverterTypes...>::forward(disp, geom);
        }
    }
};

template <typename Dispatcher>
struct converters_helper<Dispatcher>
{
    template <typename Converter>
    static void set(Dispatcher &, int) {}
    template <typename Geometry>
    static void forward(Dispatcher & disp, Geometry & geom)
    {
        disp.args_.proc.add_path(geom);
    }
};

template <typename Args, typename... ConverterTypes>
struct dispatcher : mapnik::noncopyable
{
    using this_type = dispatcher;
    using args_type = Args;

    dispatcher(typename Args::processor_type & proc, box2d<double> const& bbox, symbolizer_base const& sym, view_transform const& tr,
               proj_transform const& prj_trans, agg::trans_affine const& affine_trans, feature_impl const& feature,
               attributes const& vars, double scale_factor)
        : args_(proc,bbox,sym,tr,prj_trans,affine_trans,feature,vars,scale_factor)
    {
        std::fill(vec_.begin(), vec_.end(), 0);
    }

    std::array<unsigned, sizeof...(ConverterTypes)> vec_;
    args_type args_;
};

template <typename Processor>
struct arguments : mapnik::noncopyable
{
    using processor_type = Processor;
    arguments(Processor & proc, box2d<double> const& bbox, symbolizer_base const& sym, view_transform const& tr,
              proj_transform const& prj_trans, agg::trans_affine const& affine_trans, feature_impl const& feature,
              attributes const& vars, double scale_factor)
        : proc(proc),
          bbox(bbox),
          sym(sym),
          tr(tr),
          prj_trans(prj_trans),
          affine_trans(affine_trans),
          feature(feature),
          vars(vars),
          scale_factor(scale_factor) {}

    Processor & proc;
    box2d<double> const& bbox;
    symbolizer_base const& sym;
    view_transform const& tr;
    proj_transform const& prj_trans;
    agg::trans_affine const& affine_trans;
    feature_impl const& feature;
    attributes const& vars;
    double scale_factor;
};

}

template <typename Processor, typename... ConverterTypes >
struct vertex_converter : private mapnik::noncopyable
{
    using bbox_type = box2d<double>;
    using processor_type = Processor;
    using symbolizer_type = symbolizer_base;
    using trans_type = view_transform;
    using proj_trans_type = proj_transform;
    using affine_trans_type = agg::trans_affine;
    using feature_type = feature_impl;
    using args_type = detail::arguments<Processor>;
    using dispatcher_type = detail::dispatcher<args_type,ConverterTypes...>;

    vertex_converter(bbox_type const& bbox,
                     processor_type & proc,
                     symbolizer_type const& sym,
                     trans_type const& tr,
                     proj_trans_type const& prj_trans,
                     affine_trans_type const& affine_trans,
                     feature_type const& feature,
                     attributes const& vars,
                     double scale_factor)
        : disp_(proc,bbox,sym,tr,prj_trans,affine_trans,feature,vars,scale_factor) {}

    void apply(geometry_type & geom)
    {
        detail::converters_helper<dispatcher_type, ConverterTypes...>:: template forward<geometry_type>(disp_, geom);
    }

    template <typename Converter>
    void set()
    {
        detail::converters_helper<dispatcher_type, ConverterTypes...>:: template set<Converter>(disp_, 1);
    }

    template <typename Converter>
    void unset()
    {
        detail::converters_helper<dispatcher_type, ConverterTypes...>:: template set<Converter>(disp_, 0);
    }

    dispatcher_type disp_;
};

}

#endif // MAPNIK_VERTEX_CONVERTERS_HPP
