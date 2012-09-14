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

#ifndef MAPNIK_TRANSFORM_PROCESSOR_HPP
#define MAPNIK_TRANSFORM_PROCESSOR_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/value.hpp>
#include <mapnik/transform_expression.hpp>
#include <mapnik/expression_evaluator.hpp>

// boost
#include <boost/foreach.hpp>

// agg
#include <agg_trans_affine.h>

namespace mapnik {

template <typename Container> struct expression_attributes;

template <typename T>
struct transform_processor
{
    typedef T feature_type;
    typedef agg::trans_affine transform_type;

    template <typename Container>
    struct attribute_collector : boost::static_visitor<void>
    {
        expression_attributes<Container> collect_;

        attribute_collector(Container& names)
            : collect_(names) {}

        void operator() (identity_node const& node) const
        {
            boost::ignore_unused_variable_warning(node);
        }

        void operator() (matrix_node const& node) const
        {
            boost::apply_visitor(collect_, node.a_);
            boost::apply_visitor(collect_, node.b_);
            boost::apply_visitor(collect_, node.c_);
            boost::apply_visitor(collect_, node.d_);
            boost::apply_visitor(collect_, node.e_);
            boost::apply_visitor(collect_, node.f_);
        }

        void operator() (translate_node const& node) const
        {
            boost::apply_visitor(collect_, node.tx_);
            boost::apply_visitor(collect_, node.ty_);
        }

        void operator() (scale_node const& node) const
        {
            boost::apply_visitor(collect_, node.sx_);
            boost::apply_visitor(collect_, node.sy_);
        }

        void operator() (rotate_node const& node) const
        {
            boost::apply_visitor(collect_, node.angle_);
            boost::apply_visitor(collect_, node.cx_);
            boost::apply_visitor(collect_, node.cy_);
        }

        void operator() (skewX_node const& node) const
        {
            boost::apply_visitor(collect_, node.angle_);
        }

        void operator() (skewY_node const& node) const
        {
            boost::apply_visitor(collect_, node.angle_);
        }
    };

    struct node_evaluator : boost::static_visitor<void>
    {
        node_evaluator(transform_type& tr, feature_type const& feat)
            : transform_(tr), feature_(feat) {}

        void operator() (identity_node const& node)
        {
            boost::ignore_unused_variable_warning(node);
        }

        void operator() (matrix_node const& node)
        {
            double a = eval(node.a_);
            double b = eval(node.b_);
            double c = eval(node.c_);
            double d = eval(node.d_);
            double e = eval(node.e_);
            double f = eval(node.f_);
            transform_.multiply(agg::trans_affine(a, b, c, d, e, f));
        }

        void operator() (translate_node const& node)
        {
            double tx = eval(node.tx_);
            double ty = eval(node.ty_, 0.0);
            transform_.translate(tx, ty);
        }

        void operator() (scale_node const& node)
        {
            double sx = eval(node.sx_);
            double sy = eval(node.sy_, sx);
            transform_.scale(sx, sy);
        }

        void operator() (rotate_node const& node)
        {
            double angle = deg2rad(eval(node.angle_));
            double cx = eval(node.cx_, 0.0);
            double cy = eval(node.cy_, 0.0);
            transform_.translate(-cx, -cy);
            transform_.rotate(angle);
            transform_.translate(cx, cy);
        }

        void operator() (skewX_node const& node)
        {
            double angle = deg2rad(eval(node.angle_));
            transform_.multiply(agg::trans_affine_skewing(angle, 0.0));
        }

        void operator() (skewY_node const& node)
        {
            double angle = deg2rad(eval(node.angle_));
            transform_.multiply(agg::trans_affine_skewing(0.0, angle));
        }

    private:

        static double deg2rad(double d)
        {
            return d * M_PI / 180.0;
        }

        double eval(expr_node const& x) const
        {
            mapnik::evaluate<feature_type, value_type> e(feature_);
            return boost::apply_visitor(e, x).to_double();
        }

        double eval(expr_node const& x, double def) const
        {
            return is_null(x) ? def : eval(x);
        }

        transform_type& transform_;
        feature_type const& feature_;
    };

    template <typename Container>
    static void collect_attributes(Container& names,
                                   transform_list const& list)
    {
        attribute_collector<Container> collect(names);

        BOOST_FOREACH (transform_node const& node, list)
        {
            boost::apply_visitor(collect, *node);
        }
    }

    static void evaluate(transform_type& tr, feature_type const& feat,
                         transform_list const& list)
    {
        node_evaluator eval(tr, feat);

        #ifdef MAPNIK_LOG
        MAPNIK_LOG_DEBUG(transform) << "transform: begin with " << to_string(matrix_node(tr));
        #endif

        BOOST_REVERSE_FOREACH (transform_node const& node, list)
        {
            boost::apply_visitor(eval, *node);
            #ifdef MAPNIK_LOG
            MAPNIK_LOG_DEBUG(transform) << "transform: apply " << to_string(*node);
            MAPNIK_LOG_DEBUG(transform) << "transform: result " << to_string(matrix_node(tr));
            #endif
        }

        #ifdef MAPNIK_LOG
        MAPNIK_LOG_DEBUG(transform) << "transform: end";
        #endif
    }

    static std::string to_string(transform_node const& node)
    {
        return to_expression_string(node);
    }

    static std::string to_string(transform_list const& list)
    {
        return to_expression_string(list);
    }
};

typedef mapnik::transform_processor<Feature> transform_processor_type;

} // namespace mapnik

#endif // MAPNIK_TRANSFORM_PROCESSOR_HPP
