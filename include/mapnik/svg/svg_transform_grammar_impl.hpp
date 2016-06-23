/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

// NOTE: This is an implementation header file and is only meant to be included
//    from implementation files. It therefore doesn't have an include guard.
// mapnik
#include <mapnik/svg/svg_transform_grammar.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include <agg_trans_affine.h>
#pragma GCC diagnostic pop

namespace mapnik { namespace svg {

using namespace boost::spirit;
using namespace boost::fusion;
using namespace boost::phoenix;

inline double deg2rad(double d)
{
    return M_PI * d / 180.0;
}

struct process_matrix
{
    using result_type = void;
    template <typename TransformType>
    void operator () (TransformType & tr, double a, double b, double c, double d, double e, double f) const
    {
        tr = agg::trans_affine(a,b,c,d,e,f) * tr;
    }
};

struct process_rotate
{
    using result_type = void;

    template <typename TransformType, typename T0,typename T1,typename T2>
    void operator () (TransformType & tr, T0 a, T1 cx, T2 cy) const
    {
        if (cx == 0.0 && cy == 0.0)
        {
            tr = agg::trans_affine_rotation(deg2rad(a)) * tr;
        }
        else
        {
            agg::trans_affine t = agg::trans_affine_translation(-cx,-cy);
            t *= agg::trans_affine_rotation(deg2rad(a));
            t *= agg::trans_affine_translation(cx, cy);
            tr = t * tr;
        }
    }
};

struct process_translate
{
    using result_type = void;
    template <typename TransformType, typename T0,typename T1>
    void operator () (TransformType & tr, T0 tx, T1 ty) const
    {
        if (ty) tr = agg::trans_affine_translation(tx,*ty) * tr;
        else tr = agg::trans_affine_translation(tx,0.0) * tr;
    }
};

struct process_scale
{
    using result_type = void;
    template <typename TransformType, typename T0,typename T1>
    void operator () (TransformType & tr, T0 sx, T1 sy) const
    {
        if (sy) tr = agg::trans_affine_scaling(sx,*sy) * tr;
        else tr = agg::trans_affine_scaling(sx,sx) * tr;
    }
};


struct process_skew
{
    using result_type = void;

    template <typename TransformType, typename T0,typename T1>
    void operator () (TransformType & tr, T0 skew_x, T1 skew_y) const
    {
        tr = agg::trans_affine_skewing(deg2rad(skew_x),deg2rad(skew_y)) * tr;
    }
};

template <typename Iterator, typename TransformType, typename SkipType>
svg_transform_grammar<Iterator, TransformType, SkipType>::svg_transform_grammar()
    : svg_transform_grammar::base_type(start)
{
    qi::_1_type _1;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::_5_type _5;
    qi::_6_type _6;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_c_type _c;
    qi::_r1_type _r1;
    qi::lit_type lit;
    qi::double_type double_;
    qi::no_case_type no_case;

    // actions
    function<process_matrix> matrix_action;
    function<process_rotate> rotate_action;
    function<process_translate> translate_action;
    function<process_scale> scale_action;
    function<process_skew> skew_action;

    start =  +transform_(_r1) ;

    transform_ = matrix(_r1) | rotate(_r1) | translate(_r1) | scale(_r1) | rotate(_r1) | skewX(_r1) | skewY (_r1) ;

    matrix = no_case[lit("matrix")] >> lit('(')
                                    >> (double_ >> -lit(',')
                                        >> double_ >> -lit(',')
                                        >> double_ >> -lit(',')
                                        >> double_ >> -lit(',')
                                        >> double_ >> -lit(',')
                                        >> double_)[matrix_action(_r1, _1, _2, _3, _4, _5, _6)] >> lit(')');

    translate = no_case[lit("translate")]
        >> lit('(') >> (double_ >> -lit(',') >> -double_)[translate_action(_r1, _1, _2)] >> lit(')');

    scale = no_case[lit("scale")]
        >> lit('(') >> (double_ >> -lit(',') >> -double_)[scale_action(_r1, _1, _2)] >> lit(')');

    rotate = no_case[lit("rotate")]
        >> lit('(')
        >> double_[_a = _1] >> -lit(',')
        >> -(double_ [_b = _1] >> -lit(',') >> double_[_c = _1])
        >> lit(')') [ rotate_action(_r1, _a,_b,_c)];

    skewX = no_case[lit("skewX")] >> lit('(') >> double_ [ skew_action(_r1, _1, 0.0)] >> lit(')');

    skewY = no_case[lit("skewY")] >> lit('(') >> double_ [ skew_action(_r1, 0.0, _1)] >> lit(')');

}

}}
