/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/extend_converter.hpp>
#include <mapnik/adaptive_smooth.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_math_stroke.h"
#include "agg_trans_affine.h"
#include "agg_conv_clip_polygon.h"
#include "agg_conv_clip_polyline.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_conv_transform.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <type_traits>
#include <stdexcept>
#include <array>

namespace mapnik {

struct transform_tag
{};
struct clip_line_tag
{};
struct clip_poly_tag
{};
struct smooth_tag
{};
struct simplify_tag
{};
struct stroke_tag
{};
struct dash_tag
{};
struct affine_transform_tag
{};
struct offset_transform_tag
{};
struct extend_tag
{};

namespace detail {

template<typename T0, typename T1>
struct converter_traits
{};

template<typename T>
struct converter_traits<T, mapnik::smooth_tag>
{
    using geometry_type = T;
    using conv_type = smooth_converter<geometry_type>;

    template<typename Args>
    static void setup(geometry_type& geom, Args const& args)
    {
        geom.algorithm(get<smooth_algorithm_enum, keys::smooth_algorithm>(args.sym, args.feature, args.vars));
        geom.smooth_value(get<value_double, keys::smooth>(args.sym, args.feature, args.vars));
    }
};

template<typename T>
struct converter_traits<T, mapnik::simplify_tag>
{
    using geometry_type = T;
    using conv_type = simplify_converter<geometry_type>;

    template<typename Args>
    static void setup(geometry_type& geom, Args const& args)
    {
        geom.set_simplify_algorithm(
          get<simplify_algorithm_e, keys::simplify_algorithm>(args.sym, args.feature, args.vars));
        geom.set_simplify_tolerance(get<value_double, keys::simplify_tolerance>(args.sym, args.feature, args.vars));
    }
};

template<typename T>
struct converter_traits<T, mapnik::clip_line_tag>
{
    using geometry_type = T;
    using conv_type = typename agg::conv_clip_polyline<geometry_type>;

    template<typename Args>
    static void setup(geometry_type& geom, Args const& args)
    {
        auto const& box = args.bbox;
        geom.clip_box(box.minx(), box.miny(), box.maxx(), box.maxy());
    }
};

template<typename T>
struct converter_traits<T, mapnik::dash_tag>
{
    using geometry_type = T;
    using conv_type = typename agg::conv_dash<geometry_type>;

    template<typename Args>
    static void setup(geometry_type& geom, Args const& args)
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
                geom.add_dash(d.first * scale_factor, d.second * scale_factor);
            }
        }
    }
};

template<typename Symbolizer, typename PathType, typename Feature>
void set_join_caps(Symbolizer const& sym, PathType& stroke, Feature const& feature, attributes const& vars)
{
    const line_join_enum join = get<line_join_enum, keys::stroke_linejoin>(sym, feature, vars);
    switch (join)
    {
        case line_join_enum::MITER_JOIN:
            stroke.generator().line_join(agg::miter_join);
            break;
        case line_join_enum::MITER_REVERT_JOIN:
            stroke.generator().line_join(agg::miter_join);
            break;
        case line_join_enum::ROUND_JOIN:
            stroke.generator().line_join(agg::round_join);
            break;
        default:
            stroke.generator().line_join(agg::bevel_join);
    }

    const line_cap_enum cap = get<line_cap_enum, keys::stroke_linecap>(sym, feature, vars);
    switch (cap)
    {
        case line_cap_enum::BUTT_CAP:
            stroke.generator().line_cap(agg::butt_cap);
            break;
        case line_cap_enum::SQUARE_CAP:
            stroke.generator().line_cap(agg::square_cap);
            break;
        default:
            stroke.generator().line_cap(agg::round_cap);
    }
}

template<typename T>
struct converter_traits<T, mapnik::stroke_tag>
{
    using geometry_type = T;
    using conv_type = typename agg::conv_stroke<geometry_type>;

    template<typename Args>
    static void setup(geometry_type& geom, Args const& args)
    {
        auto const& sym = args.sym;
        auto const& feat = args.feature;
        auto const& vars = args.vars;
        set_join_caps(sym, geom, feat, vars);
        double miterlimit = get<value_double, keys::stroke_miterlimit>(sym, feat, vars);
        geom.generator().miter_limit(miterlimit);
        double scale_factor = args.scale_factor;
        double width = get<value_double, keys::stroke_width>(sym, feat, vars);
        geom.generator().width(width * scale_factor);
    }
};

template<typename T>
struct converter_traits<T, mapnik::clip_poly_tag>
{
    using geometry_type = T;
    using conv_type = typename agg::conv_clip_polygon<geometry_type>;
    template<typename Args>
    static void setup(geometry_type& geom, Args const& args)
    {
        auto const& box = args.bbox;
        geom.clip_box(box.minx(), box.miny(), box.maxx(), box.maxy());
    }
};

template<typename T>
struct converter_traits<T, mapnik::transform_tag>
{
    using geometry_type = T;
    using conv_type = transform_path_adapter<view_transform, geometry_type>;

    template<typename Args>
    static void setup(geometry_type& geom, Args const& args)
    {
        geom.set_proj_trans(args.prj_trans);
        geom.set_trans(args.tr);
    }
};

template<typename T>
struct converter_traits<T, mapnik::affine_transform_tag>
{
    using geometry_type = T;
    using conv_base_type = agg::conv_transform<geometry_type, agg::trans_affine const>;
    struct conv_type : public conv_base_type
    {
        conv_type(geometry_type& geom)
            : conv_base_type(geom, agg::trans_affine::identity)
        {}
    };

    template<typename Args>
    static void setup(geometry_type& geom, Args& args)
    {
        geom.transformer(args.affine_trans);
    }
};

template<typename T>
struct converter_traits<T, mapnik::offset_transform_tag>
{
    using geometry_type = T;
    using conv_type = offset_converter<geometry_type>;

    template<typename Args>
    static void setup(geometry_type& geom, Args const& args)
    {
        auto const& sym = args.sym;
        auto const& feat = args.feature;
        auto const& vars = args.vars;
        double offset = get<value_double, keys::offset>(sym, feat, vars);
        geom.set_offset(offset * args.scale_factor);
    }
};

template<typename T>
struct converter_traits<T, mapnik::extend_tag>
{
    using geometry_type = T;
    using conv_type = extend_converter<geometry_type>;

    template<typename Args>
    static void setup(geometry_type& geom, Args const& args)
    {
        auto const& sym = args.sym;
        auto const& feat = args.feature;
        auto const& vars = args.vars;
        double extend = get<value_double, keys::extend>(sym, feat, vars);
        geom.set_extend(extend * args.scale_factor);
    }
};

template<typename T0, typename T1>
struct is_switchable
{
    static constexpr bool value = true;
};

template<typename T>
struct is_switchable<T, transform_tag>
{
    static constexpr bool value = false;
};

template<typename T>
struct is_switchable<T, stroke_tag>
{
    static constexpr bool value = false;
};

template<typename Dispatcher, typename... ConverterTypes>
struct converters_helper;

template<typename Dispatcher, typename Current, typename... ConverterTypes>
struct converters_helper<Dispatcher, Current, ConverterTypes...>
{
    template<typename Converter>
    static void set(Dispatcher& disp, std::size_t state)
    {
        if (std::is_same<Converter, Current>::value)
        {
            constexpr std::size_t index = sizeof...(ConverterTypes);
            disp.vec_[index] = state;
        }
        else
        {
            converters_helper<Dispatcher, ConverterTypes...>::template set<Converter>(disp, state);
        }
    }

    template<typename Geometry, typename Processor>
    static void forward(Dispatcher& disp,
                        Geometry& geom,
                        Processor& proc,
                        typename std::enable_if<detail::is_switchable<Geometry, Current>::value>::type* = 0)
    {
        constexpr std::size_t index = sizeof...(ConverterTypes);
        if (disp.vec_[index] == 1)
        {
            using conv_type = typename detail::converter_traits<Geometry, Current>::conv_type;
            conv_type conv(geom);
            detail::converter_traits<conv_type, Current>::setup(conv, disp.args_);
            converters_helper<Dispatcher, ConverterTypes...>::forward(disp, conv, proc);
        }
        else
        {
            converters_helper<Dispatcher, ConverterTypes...>::forward(disp, geom, proc);
        }
    }
    template<typename Geometry, typename Processor>
    static void forward(Dispatcher& disp,
                        Geometry& geom,
                        Processor& proc,
                        typename std::enable_if<!detail::is_switchable<Geometry, Current>::value>::type* = 0)
    {
        using conv_type = typename detail::converter_traits<Geometry, Current>::conv_type;
        conv_type conv(geom);
        detail::converter_traits<conv_type, Current>::setup(conv, disp.args_);
        converters_helper<Dispatcher, ConverterTypes...>::forward(disp, conv, proc);
    }
};

template<typename Dispatcher>
struct converters_helper<Dispatcher>
{
    template<typename Converter>
    static void set(Dispatcher&, std::size_t)
    {}
    template<typename Geometry, typename Processor>
    static void forward(Dispatcher&, Geometry& geom, Processor& proc)
    {
        proc.add_path(geom);
    }
};

template<typename Args, std::size_t NUM_CONV>
struct dispatcher : util::noncopyable
{
    using this_type = dispatcher;
    using args_type = Args;

    dispatcher(box2d<double> const& bbox,
               symbolizer_base const& sym,
               view_transform const& tr,
               proj_transform const& prj_trans,
               agg::trans_affine const& affine_trans,
               feature_impl const& feature,
               attributes const& vars,
               double scale_factor)
        : args_(bbox, sym, tr, prj_trans, affine_trans, feature, vars, scale_factor)
    {
        std::fill(vec_.begin(), vec_.end(), 0);
    }

    std::array<std::size_t, NUM_CONV> vec_;
    args_type args_;
};

struct arguments : util::noncopyable
{
    arguments(box2d<double> const& _bbox,
              symbolizer_base const& _sym,
              view_transform const& _tr,
              proj_transform const& _prj_trans,
              agg::trans_affine const& _affine_trans,
              feature_impl const& _feature,
              attributes const& _vars,
              double _scale_factor)
        : bbox(_bbox)
        , sym(_sym)
        , tr(_tr)
        , prj_trans(_prj_trans)
        , affine_trans(_affine_trans)
        , feature(_feature)
        , vars(_vars)
        , scale_factor(_scale_factor)
    {}

    box2d<double> const& bbox;
    symbolizer_base const& sym;
    view_transform const& tr;
    proj_transform const& prj_trans;
    agg::trans_affine const& affine_trans;
    feature_impl const& feature;
    attributes const& vars;
    double scale_factor;
};

} // namespace detail

template<typename... ConverterTypes>
struct vertex_converter : private util::noncopyable
{
    using bbox_type = box2d<double>;
    using symbolizer_type = symbolizer_base;
    using trans_type = view_transform;
    using proj_trans_type = proj_transform;
    using affine_trans_type = agg::trans_affine;
    using feature_type = feature_impl;
    using args_type = detail::arguments;
    using dispatcher_type = detail::dispatcher<args_type, sizeof...(ConverterTypes)>;

    vertex_converter(bbox_type const& bbox,
                     symbolizer_type const& sym,
                     trans_type const& tr,
                     proj_trans_type const& prj_trans,
                     affine_trans_type const& affine_trans,
                     feature_type const& feature,
                     attributes const& vars,
                     double scale_factor)
        : disp_(bbox, sym, tr, prj_trans, affine_trans, feature, vars, scale_factor)
    {}

    template<typename VertexAdapter, typename Processor>
    void apply(VertexAdapter& geom, Processor& proc)
    {
        detail::converters_helper<dispatcher_type, ConverterTypes...>::template forward<VertexAdapter, Processor>(disp_,
                                                                                                                  geom,
                                                                                                                  proc);
    }

    template<typename Converter>
    void set()
    {
        detail::converters_helper<dispatcher_type, ConverterTypes...>::template set<Converter>(disp_, 1);
    }

    template<typename Converter>
    void unset()
    {
        detail::converters_helper<dispatcher_type, ConverterTypes...>::template set<Converter>(disp_, 0);
    }

    dispatcher_type disp_;
};

} // namespace mapnik

#endif // MAPNIK_VERTEX_CONVERTERS_HPP
