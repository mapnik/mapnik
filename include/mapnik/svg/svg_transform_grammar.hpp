/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_SVG_TRANSFORM_GRAMMAR_HPP
#define MAPNIK_SVG_TRANSFORM_GRAMMAR_HPP

// mapnik
#include <mapnik/global.hpp>

// agg
#include <agg_trans_affine.h>

// spirit
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

namespace mapnik { namespace svg {

    using namespace boost::spirit;
    using namespace boost::fusion;
    using namespace boost::phoenix;

    inline double deg2rad(double d)
    {
        return M_PI * d / 180.0;
    }

    template <typename TransformType>
    struct process_matrix
    {
#ifdef BOOST_SPIRIT_USE_PHOENIX_V3
        template <typename T0>
#else
        template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
#endif
        struct result
        {
            typedef void type;
        };

        explicit process_matrix( TransformType & tr)
            :tr_(tr) {}

        void operator () (double a, double b, double c, double d, double e, double f) const
        {
            tr_ = agg::trans_affine(a,b,c,d,e,f) * tr_;
        }

        TransformType & tr_;
    };

    template <typename TransformType>
    struct process_rotate
    {
#ifdef BOOST_SPIRIT_USE_PHOENIX_V3
        template <typename T0>
#else
        template <typename T0, typename T1, typename T2>
#endif
        struct result
        {
            typedef void type;
        };

        explicit process_rotate( TransformType & tr)
            :tr_(tr) {}

        template <typename T0,typename T1,typename T2>
        void operator () (T0 a, T1 cx, T2 cy) const
        {
            if (cx == 0.0 && cy == 0.0)
            {
                tr_ = agg::trans_affine_rotation(deg2rad(a)) * tr_;
            }
            else
            {
                agg::trans_affine t = agg::trans_affine_translation(-cx,-cy);
                t *= agg::trans_affine_rotation(deg2rad(a));
                t *= agg::trans_affine_translation(cx, cy);
                tr_ = t * tr_;
            }
        }

        TransformType & tr_;
    };

    template <typename TransformType>
    struct process_translate
    {
#ifdef BOOST_SPIRIT_USE_PHOENIX_V3
        template <typename T0>
#else
        template <typename T0, typename T1>
#endif
        struct result
        {
            typedef void type;
        };

        explicit process_translate( TransformType & tr)
            :tr_(tr) {}

        template <typename T0,typename T1>
        void operator () (T0 tx, T1 ty) const
        {
            if (ty) tr_ = agg::trans_affine_translation(tx,*ty) * tr_;
            else tr_ = agg::trans_affine_translation(tx,0.0) * tr_;
        }

        TransformType & tr_;
    };

    template <typename TransformType>
    struct process_scale
    {
#ifdef BOOST_SPIRIT_USE_PHOENIX_V3
        template <typename T0>
#else
        template <typename T0, typename T1>
#endif
        struct result
        {
            typedef void type;
        };

        explicit process_scale( TransformType & tr)
            :tr_(tr) {}

        template <typename T0,typename T1>
        void operator () (T0 sx, T1 sy) const
        {
            if (sy) tr_ = agg::trans_affine_scaling(sx,*sy) * tr_;
            else tr_ = agg::trans_affine_scaling(sx,sx) * tr_;
        }

        TransformType & tr_;
    };


    template <typename TransformType>
    struct process_skew
    {
#ifdef BOOST_SPIRIT_USE_PHOENIX_V3
        template <typename T0>
#else
        template <typename T0, typename T1>
#endif
        struct result
        {
            typedef void type;
        };

        explicit process_skew( TransformType & tr)
            :tr_(tr) {}

        template <typename T0,typename T1>
        void operator () (T0 skew_x, T1 skew_y) const
        {
            tr_ = agg::trans_affine_skewing(deg2rad(skew_x),deg2rad(skew_y)) * tr_;
        }

        TransformType & tr_;
    };

    // commented as this does not appear used and crashes clang when used with pch
    /*
      struct print_action
      {
      template <typename T>
      void operator()(T const& c, qi::unused_type, qi::unused_type) const
      {
        MAPNIK_LOG_DEBUG(svg) << typeid(c).name();
      }
      };
    */

    template <typename Iterator, typename SkipType, typename TransformType>
    struct svg_transform_grammar : qi::grammar<Iterator,SkipType>
    {
        explicit svg_transform_grammar(TransformType & tr)
            : svg_transform_grammar::base_type(start),
              matrix_action(process_matrix<TransformType>(tr)),
              rotate_action(process_rotate<TransformType>(tr)),
              translate_action(process_translate<TransformType>(tr)),
              scale_action(process_scale<TransformType>(tr)),
              skew_action(process_skew<TransformType>(tr))
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
            qi::lit_type lit;
            qi::double_type double_;
            qi::no_case_type no_case;

            start =  +transform_ ;

            transform_ = matrix | rotate | translate | scale | rotate | skewX | skewY ;

            matrix = no_case[lit("matrix")]
                >> lit('(')
                >> (
                    double_ >> -lit(',')
                    >> double_ >> -lit(',')
                    >> double_ >> -lit(',')
                    >> double_ >> -lit(',')
                    >> double_ >> -lit(',')
                    >> double_) [ matrix_action(_1,_2,_3,_4,_5,_6) ]
                >>  lit(')')
                ;

            translate = no_case[lit("translate")]
                >> lit('(')
                >> (double_ >> -lit(',')
                    >> -double_) [ translate_action(_1,_2) ]
                >> lit(')');

            scale = no_case[lit("scale")]
                >> lit('(')
                >> (double_ >> -lit(',')
                    >> -double_ )[ scale_action(_1,_2)]
                >>  lit(')');

            rotate = no_case[lit("rotate")]
                >> lit('(')
                >> double_[_a = _1] >> -lit(',')
                >> -(double_ [_b = _1] >> -lit(',') >> double_[_c = _1])
                >> lit(')') [ rotate_action(_a,_b,_c)];

            skewX = no_case[lit("skewX")] >> lit('(') >> double_ [ skew_action(_1, 0.0)] >> lit(')');

            skewY = no_case[lit("skewY")] >> lit('(') >> double_ [ skew_action(0.0, _1)] >> lit(')');

        }

        // rules
        qi::rule<Iterator,SkipType> start;
        qi::rule<Iterator,SkipType> transform_;
        qi::rule<Iterator,SkipType> matrix;
        qi::rule<Iterator,SkipType> translate;
        qi::rule<Iterator,SkipType> scale;
        qi::rule<Iterator,qi::locals<double,double,double>, SkipType> rotate;
        qi::rule<Iterator,SkipType> skewX;
        qi::rule<Iterator,SkipType> skewY;

        // actions
        function<process_matrix<TransformType> > matrix_action;
        function<process_rotate<TransformType> > rotate_action;
        function<process_translate<TransformType> > translate_action;
        function<process_scale<TransformType> > scale_action;
        function<process_skew<TransformType> > skew_action;
    };

    }}

#endif // MAPNIK_SVG_TRANSFORM_GRAMMAR_HPP
