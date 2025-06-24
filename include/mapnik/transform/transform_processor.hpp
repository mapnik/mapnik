/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_TRANSFORM_PROCESSOR_HPP
#define MAPNIK_TRANSFORM_PROCESSOR_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/value.hpp>
#include <mapnik/transform/transform_expression.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/util/math.hpp>
#include <mapnik/util/variant.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include <agg_trans_affine.h>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <cmath>

namespace mapnik {

class feature_impl;

template<typename Container>
struct expression_attributes;

template<typename T, typename T1>
struct transform_processor
{
    using feature_type = T;
    using variable_type = T1;
    using transform_type = agg::trans_affine;

    template<typename Container>
    struct attribute_collector
    {
        expression_attributes<Container> collect_;

        attribute_collector(Container& names)
            : collect_(names)
        {}

        void operator()(identity_node const&) const {}

        void operator()(matrix_node const& node) const
        {
            util::apply_visitor(collect_, node.a_);
            util::apply_visitor(collect_, node.b_);
            util::apply_visitor(collect_, node.c_);
            util::apply_visitor(collect_, node.d_);
            util::apply_visitor(collect_, node.e_);
            util::apply_visitor(collect_, node.f_);
        }

        void operator()(translate_node const& node) const
        {
            util::apply_visitor(collect_, node.tx_);
            util::apply_visitor(collect_, node.ty_);
        }

        void operator()(scale_node const& node) const
        {
            util::apply_visitor(collect_, node.sx_);
            util::apply_visitor(collect_, node.sy_);
        }

        void operator()(rotate_node const& node) const
        {
            util::apply_visitor(collect_, node.angle_);
            util::apply_visitor(collect_, node.cx_);
            util::apply_visitor(collect_, node.cy_);
        }

        void operator()(skewX_node const& node) const { util::apply_visitor(collect_, node.angle_); }

        void operator()(skewY_node const& node) const { util::apply_visitor(collect_, node.angle_); }
    };

    struct node_evaluator
    {
        node_evaluator(transform_type& tr, feature_type const& feat, variable_type const& v, double scale_factor)
            : transform_(tr)
            , feature_(feat)
            , vars_(v)
            , scale_factor_(scale_factor)
        {}

        void operator()(identity_node const&) const {}

        void operator()(matrix_node const& node) const
        {
            double a = eval(node.a_); // scale x;
            double b = eval(node.b_);
            double c = eval(node.c_);
            double d = eval(node.d_);                 // scale y;
            double e = eval(node.e_) * scale_factor_; // translate x
            double f = eval(node.f_) * scale_factor_; // translate y
            transform_.multiply(agg::trans_affine(a, b, c, d, e, f));
        }

        void operator()(translate_node const& node) const
        {
            double tx = eval(node.tx_) * scale_factor_;
            double ty = eval(node.ty_, 0.0) * scale_factor_;
            transform_.translate(tx, ty);
        }

        void operator()(scale_node const& node) const
        {
            double sx = eval(node.sx_);
            double sy = eval(node.sy_, sx);
            transform_.scale(sx, sy);
        }

        void operator()(rotate_node const& node) const
        {
            double angle = agg::deg2rad(eval(node.angle_));
            double cx = eval(node.cx_, 0.0);
            double cy = eval(node.cy_, 0.0);
            transform_.translate(-cx, -cy);
            transform_.rotate(angle);
            transform_.translate(cx, cy);
        }

        void operator()(skewX_node const& node) const
        {
            auto degrees = std::fmod(eval(node.angle_), 90.0);
            auto angle = agg::deg2rad(util::clamp(degrees, -89.0, 89.0));
            transform_.multiply(agg::trans_affine_skewing(angle, 0.0));
        }

        void operator()(skewY_node const& node) const
        {
            auto degrees = std::fmod(eval(node.angle_), 90.0);
            auto angle = agg::deg2rad(util::clamp(degrees, -89.0, 89.0));
            transform_.multiply(agg::trans_affine_skewing(0.0, angle));
        }

      private:

        double eval(expr_node const& x) const
        {
            mapnik::evaluate<feature_type, value_type, variable_type> e(feature_, vars_);
            return util::apply_visitor(e, x).to_double();
        }

        double eval(expr_node const& x, double def) const { return detail::is_null_node(x) ? def : eval(x); }

        transform_type& transform_;
        feature_type const& feature_;
        variable_type const& vars_;
        double scale_factor_;
    };

    template<typename Container>
    static void collect_attributes(Container& names, transform_list const& list)
    {
        attribute_collector<Container> collect(names);

        for (transform_node const& node : list)
        {
            util::apply_visitor(collect, *node);
        }
    }

    static void evaluate(transform_type& tr,
                         feature_type const& feat,
                         variable_type const& vars,
                         transform_list const& list,
                         double scale_factor)
    {
        node_evaluator eval(tr, feat, vars, scale_factor);

        transform_list::const_reverse_iterator rit;
        for (rit = list.rbegin(); rit != list.rend(); ++rit)
        {
            util::apply_visitor(eval, *(*rit));
        }
    }

    static std::string to_string(transform_node const& node) { return to_expression_string(node); }

    static std::string to_string(transform_list const& list) { return to_expression_string(list); }
};

using transform_processor_type = mapnik::transform_processor<feature_impl, attributes>;

} // namespace mapnik

#endif // MAPNIK_TRANSFORM_PROCESSOR_HPP
