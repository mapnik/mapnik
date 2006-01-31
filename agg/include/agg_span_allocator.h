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

#ifndef AGG_SPAN_ALLOCATOR_INCLUDED
#define AGG_SPAN_ALLOCATOR_INCLUDED

#include "agg_basics.h"

namespace agg
{
    //----------------------------------------------------------span_allocator
    template<class ColorT> class span_allocator
    {
    public:
        typedef ColorT color_type;

        //--------------------------------------------------------------------
        ~span_allocator()
        {
            delete [] m_span;
        }

        //--------------------------------------------------------------------
        span_allocator() :
            m_max_span_len(0),
            m_span(0)
        {
        }

        //--------------------------------------------------------------------
        AGG_INLINE color_type* allocate(unsigned span_len)
        {
            if(span_len > m_max_span_len)
            {
                // To reduce the number of reallocs we align the 
                // span_len to 256 color elements. 
                // Well, I just like this number and it looks reasonable.
                //-----------------------
                delete [] m_span;
                span_len = ((span_len + 255) >> 8) << 8;
                m_span = new color_type[m_max_span_len = span_len];
            }
            return m_span;
        }

        AGG_INLINE color_type* span()               { return m_span; }
        AGG_INLINE unsigned    max_span_len() const { return m_max_span_len; }

    private:
        //--------------------------------------------------------------------
        span_allocator(const span_allocator<ColorT>&);
        const span_allocator<ColorT>& operator = (const span_allocator<ColorT>&);

        unsigned    m_max_span_len;
        color_type* m_span;
    };
}


#endif


