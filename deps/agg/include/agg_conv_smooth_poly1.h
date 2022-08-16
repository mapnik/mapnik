//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// Smooth polygon generator
//
//----------------------------------------------------------------------------
#ifndef AGG_CONV_SMOOTH_POLY1_INCLUDED
#define AGG_CONV_SMOOTH_POLY1_INCLUDED

#include "agg_basics.h"
#include "agg_vcgen_smooth_poly1.h"
#include "agg_conv_adaptor_vcgen.h"
#include "agg_conv_curve.h"


namespace agg
{

    //-------------------------------------------------------conv_smooth
    template<class VertexSource, class VertexGenerator>
    struct conv_smooth :
    public conv_adaptor_vcgen<VertexSource, VertexGenerator>
    {
        typedef conv_adaptor_vcgen<VertexSource, VertexGenerator> base_type;

        conv_smooth(VertexSource& vs) :
            conv_adaptor_vcgen<VertexSource, VertexGenerator>(vs)
        {
        }

        conv_smooth(conv_smooth<VertexSource, VertexGenerator> &&) = default;

        conv_smooth(const conv_smooth<VertexSource, VertexGenerator>&) = delete;
        const conv_smooth<VertexSource, VertexGenerator>&
            operator = (const conv_smooth<VertexSource, VertexGenerator>&) = delete;

        void   smooth_value(double v) { base_type::generator().smooth_value(v); }
        double smooth_value() const { return base_type::generator().smooth_value(); }
        unsigned type() const { return base_type::type(); }
    };

    template<class VertexSource>
    using conv_smooth_poly1 = conv_smooth<VertexSource, vcgen_smooth_poly1>;

    //-------------------------------------------------conv_smooth_curve
    template<class VertexSource, class VertexGenerator>
    struct conv_smooth_curve :
    public conv_curve<conv_smooth<VertexSource, VertexGenerator>>
    {
        conv_smooth_curve(VertexSource& vs) :
            conv_curve<conv_smooth<VertexSource, VertexGenerator>>(m_smooth),
            m_smooth(vs)
        {
        }

        conv_smooth_curve(conv_smooth_curve<VertexSource, VertexGenerator> && rhs) :
            conv_curve<conv_smooth<VertexSource, VertexGenerator>>(std::move(rhs)),
            m_smooth(std::move(rhs.m_smooth))
        {
            this->attach(m_smooth);
        }

        conv_smooth_curve(const conv_smooth_curve<VertexSource, VertexGenerator>&) = delete;
        const conv_smooth_curve<VertexSource, VertexGenerator>&
            operator = (const conv_smooth_curve<VertexSource, VertexGenerator>&) = delete;

        void   smooth_value(double v) { m_smooth.generator().smooth_value(v); }
        double smooth_value() const { return m_smooth.generator().smooth_value(); }
        unsigned type() const { return m_smooth.type(); }

    private:
        conv_smooth<VertexSource, VertexGenerator> m_smooth;
    };

    template<class VertexSource>
    using conv_smooth_poly1_curve = conv_smooth_curve<VertexSource, vcgen_smooth_poly1>;
}


#endif

