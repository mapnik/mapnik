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
// Adaptation for high precision colors has been sponsored by 
// Liberty Technology Systems, Inc., visit http://lib-sys.com
//
// Liberty Technology Systems, Inc. is the provider of
// PostScript and PDF technology for software developers.
// 
//----------------------------------------------------------------------------

#ifndef AGG_PIXFMT_GRAY_INCLUDED
#define AGG_PIXFMT_GRAY_INCLUDED

#include <string.h>
#include "agg_basics.h"
#include "agg_color_gray.h"
#include "agg_rendering_buffer.h"

namespace agg
{
 
    //============================================================blender_gray
    template<class ColorT> struct blender_gray
    {
        typedef ColorT color_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e { base_shift = color_type::base_shift };

        static AGG_INLINE void blend_pix(value_type* p, unsigned cv, 
                                         unsigned alpha, unsigned cover=0)
        {
            *p = (value_type)((((cv - calc_type(*p)) * alpha) + (calc_type(*p) << base_shift)) >> base_shift);
        }
    };


    //======================================================blender_gray_pre
    template<class ColorT> struct blender_gray_pre
    {
        typedef ColorT color_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e { base_shift = color_type::base_shift };

        static AGG_INLINE void blend_pix(value_type* p, unsigned cv,
                                         unsigned alpha, unsigned cover)
        {
            alpha = color_type::base_mask - alpha;
            cover = (cover + 1) << (base_shift - 8);
            *p = (value_type)((*p * alpha + cv * cover) >> base_shift);
        }

        static AGG_INLINE void blend_pix(value_type* p, unsigned cv,
                                         unsigned alpha)
        {
            *p = (value_type)(((*p * (color_type::base_mask - alpha)) >> base_shift) + cv);
        }
    };
    


    //=====================================================apply_gamma_dir_gray
    template<class ColorT, class GammaLut> class apply_gamma_dir_gray
    {
    public:
        typedef typename ColorT::value_type value_type;

        apply_gamma_dir_gray(const GammaLut& gamma) : m_gamma(gamma) {}

        AGG_INLINE void operator () (value_type* p)
        {
            *p = m_gamma.dir(*p);
        }

    private:
        const GammaLut& m_gamma;
    };



    //=====================================================apply_gamma_inv_gray
    template<class ColorT, class GammaLut> class apply_gamma_inv_gray
    {
    public:
        typedef typename ColorT::value_type value_type;

        apply_gamma_inv_gray(const GammaLut& gamma) : m_gamma(gamma) {}

        AGG_INLINE void operator () (value_type* p)
        {
            *p = m_gamma.inv(*p);
        }

    private:
        const GammaLut& m_gamma;
    };



    //=================================================pixfmt_alpha_blend_gray
    template<class Blender, class RenBuf, unsigned Step=1, unsigned Offset=0>
    class pixfmt_alpha_blend_gray
    {
    public:
        typedef RenBuf   rbuf_type;
        typedef typename rbuf_type::row_data row_data;
        typedef Blender  blender_type;
        typedef typename blender_type::color_type color_type;
        typedef int                               order_type; // A fake one
        typedef typename color_type::value_type   value_type;
        typedef typename color_type::calc_type    calc_type;
        enum base_scale_e 
        {
            base_shift = color_type::base_shift,
            base_scale = color_type::base_scale,
            base_mask  = color_type::base_mask,
            pix_width  = sizeof(value_type)
        };

    private:
        //--------------------------------------------------------------------
        static AGG_INLINE void copy_or_blend_pix(value_type* p, 
                                                 const color_type& c, 
                                                 unsigned cover)
        {
            if (c.a)
            {
                calc_type alpha = (calc_type(c.a) * (cover + 1)) >> 8;
                if(alpha == base_mask)
                {
                    *p = c.v;
                }
                else
                {
                    Blender::blend_pix(p, c.v, alpha, cover);
                }
            }
        }


        static AGG_INLINE void copy_or_blend_pix(value_type* p, 
                                                 const color_type& c)
        {
            if (c.a)
            {
                if(c.a == base_mask)
                {
                    *p = c.v;
                }
                else
                {
                    Blender::blend_pix(p, c.v, c.a);
                }
            }
        }


    public:
        //--------------------------------------------------------------------
        pixfmt_alpha_blend_gray(rbuf_type& rb) :
            m_rbuf(&rb)
        {}

        //--------------------------------------------------------------------
        AGG_INLINE unsigned width()  const { return m_rbuf->width();  }
        AGG_INLINE unsigned height() const { return m_rbuf->height(); }

        //--------------------------------------------------------------------
        const int8u* row_ptr(int y) const
        {
            return m_rbuf->row_ptr(y);
        }

        //--------------------------------------------------------------------
        const int8u* pix_ptr(int x, int y) const
        {
            return m_rbuf->row_ptr(y) + x * pix_width;
        }

        //--------------------------------------------------------------------
        row_data row(int x, int y) const
        {
            return m_rbuf->row(y);
        }

        //--------------------------------------------------------------------
        AGG_INLINE static void make_pix(int8u* p, const color_type& c)
        {
            *(value_type*)p = c.v;
        }

        //--------------------------------------------------------------------
        AGG_INLINE color_type pixel(int x, int y) const
        {
            value_type* p = (value_type*)m_rbuf->row(y) + x * Step + Offset;
            return color_type(*p);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void copy_pixel(int x, int y, const color_type& c)
        {
            *((value_type*)m_rbuf->row_ptr(x, y, 1) + x * Step + Offset) = c.v;
        }

        //--------------------------------------------------------------------
        AGG_INLINE void blend_pixel(int x, int y, const color_type& c, int8u cover)
        {
            copy_or_blend_pix((value_type*)
                               m_rbuf->row_ptr(x, y, 1) + x * Step + Offset, 
                               c, 
                               cover);
        }


        //--------------------------------------------------------------------
        AGG_INLINE void copy_hline(int x, int y, 
                                   unsigned len, 
                                   const color_type& c)
        {
            value_type* p = (value_type*)
                m_rbuf->row_ptr(x, y, len) + x * Step + Offset;

            do
            {
                *p = c.v; 
                p += Step;
            }
            while(--len);
        }


        //--------------------------------------------------------------------
        AGG_INLINE void copy_vline(int x, int y,
                                   unsigned len, 
                                   const color_type& c)
        {
            do
            {
                value_type* p = (value_type*)
                    m_rbuf->row_ptr(x, y++, 1) + x * Step + Offset;

                *p = c.v;
            }
            while(--len);
        }


        //--------------------------------------------------------------------
        void blend_hline(int x, int y,
                         unsigned len, 
                         const color_type& c,
                         int8u cover)
        {
            if (c.a)
            {
                value_type* p = (value_type*)
                    m_rbuf->row_ptr(x, y, len) + x * Step + Offset;

                calc_type alpha = (calc_type(c.a) * (cover + 1)) >> 8;
                if(alpha == base_mask)
                {
                    do
                    {
                        *p = c.v; 
                        p += Step;
                    }
                    while(--len);
                }
                else
                {
                    do
                    {
                        Blender::blend_pix(p, c.v, alpha, cover);
                        p += Step;
                    }
                    while(--len);
                }
            }
        }


        //--------------------------------------------------------------------
        void blend_vline(int x, int y,
                         unsigned len, 
                         const color_type& c,
                         int8u cover)
        {
            if (c.a)
            {
                value_type* p;
                calc_type alpha = (calc_type(c.a) * (cover + 1)) >> 8;
                if(alpha == base_mask)
                {
                    do
                    {
                        p = (value_type*)
                            m_rbuf->row_ptr(x, y++, 1) + x * Step + Offset;

                        *p = c.v; 
                    }
                    while(--len);
                }
                else
                {
                    do
                    {
                        p = (value_type*)
                            m_rbuf->row_ptr(x, y++, 1) + x * Step + Offset;

                        Blender::blend_pix(p, c.v, alpha, cover);
                    }
                    while(--len);
                }
            }
        }


        //--------------------------------------------------------------------
        void blend_solid_hspan(int x, int y,
                               unsigned len, 
                               const color_type& c,
                               const int8u* covers)
        {
            if (c.a)
            {
                value_type* p = (value_type*)
                    m_rbuf->row_ptr(x, y, len) + x * Step + Offset;

                do 
                {
                    calc_type alpha = (calc_type(c.a) * (calc_type(*covers) + 1)) >> 8;
                    if(alpha == base_mask)
                    {
                        *p = c.v;
                    }
                    else
                    {
                        Blender::blend_pix(p, c.v, alpha, *covers);
                    }
                    p += Step;
                    ++covers;
                }
                while(--len);
            }
        }


        //--------------------------------------------------------------------
        void blend_solid_vspan(int x, int y,
                               unsigned len, 
                               const color_type& c,
                               const int8u* covers)
        {
            if (c.a)
            {
                do 
                {
                    calc_type alpha = (calc_type(c.a) * (calc_type(*covers) + 1)) >> 8;

                    value_type* p = (value_type*)
                        m_rbuf->row_ptr(x, y++, 1) + x * Step + Offset;

                    if(alpha == base_mask)
                    {
                        *p = c.v;
                    }
                    else
                    {
                        Blender::blend_pix(p, c.v, alpha, *covers);
                    }
                    ++covers;
                }
                while(--len);
            }
        }


        //--------------------------------------------------------------------
        void copy_color_hspan(int x, int y,
                              unsigned len, 
                              const color_type* colors)
        {
            value_type* p = (value_type*)
                m_rbuf->row_ptr(x, y, len) + x * Step + Offset;

            do 
            {
                *p++ = colors->v;
                ++colors;
            }
            while(--len);
        }



        //--------------------------------------------------------------------
        void blend_color_hspan(int x, int y,
                               unsigned len, 
                               const color_type* colors,
                               const int8u* covers,
                               int8u cover)
        {
            value_type* p = (value_type*)
                m_rbuf->row_ptr(x, y, len) + x * Step + Offset;

            if(covers)
            {
                do 
                {
                    copy_or_blend_pix(p, *colors++, *covers++);
                    p += Step;
                }
                while(--len);
            }
            else
            {
                if(cover == 255)
                {
                    do 
                    {
                        if(colors->a == base_mask)
                        {
                            *p = colors->v;
                        }
                        else
                        {
                            copy_or_blend_pix(p, *colors);
                        }
                        p += Step;
                        ++colors;
                    }
                    while(--len);
                }
                else
                {
                    do 
                    {
                        copy_or_blend_pix(p, *colors++, cover);
                        p += Step;
                    }
                    while(--len);
                }
            }
        }



        //--------------------------------------------------------------------
        void blend_color_vspan(int x, int y,
                               unsigned len, 
                               const color_type* colors,
                               const int8u* covers,
                               int8u cover)
        {
            value_type* p;
            if(covers)
            {
                do 
                {
                    p = (value_type*)
                        m_rbuf->row_ptr(x, y++, 1) + x * Step + Offset;

                    copy_or_blend_pix(p, *colors++, *covers++);
                }
                while(--len);
            }
            else
            {
                if(cover == 255)
                {
                    do 
                    {
                        p = (value_type*)
                            m_rbuf->row_ptr(x, y++, 1) + x * Step + Offset;

                        if(colors->a == base_mask)
                        {
                            *p = colors->v;
                        }
                        else
                        {
                            copy_or_blend_pix(p, *colors);
                        }
                        ++colors;
                    }
                    while(--len);
                }
                else
                {
                    do 
                    {
                        p = (value_type*)
                            m_rbuf->row_ptr(x, y++, 1) + x * Step + Offset;

                        copy_or_blend_pix(p, *colors++, cover);
                    }
                    while(--len);
                }
            }
        }

        //--------------------------------------------------------------------
        template<class Function> void for_each_pixel(Function f)
        {
            unsigned y;
            for(y = 0; y < height(); ++y)
            {
                row_data r = m_rbuf->row(y);
                if(r.ptr)
                {
                    unsigned len = r.x2 - r.x1 + 1;

                    value_type* p = (value_type*)
                        m_rbuf->row_ptr(r.x1, y, len) + r.x1 * Step + Offset;

                    do
                    {
                        f(p);
                        p += Step;
                    }
                    while(--len);
                }
            }
        }

        //--------------------------------------------------------------------
        template<class GammaLut> void apply_gamma_dir(const GammaLut& g)
        {
            for_each_pixel(apply_gamma_dir_gray<color_type, GammaLut>(g));
        }

        //--------------------------------------------------------------------
        template<class GammaLut> void apply_gamma_inv(const GammaLut& g)
        {
            for_each_pixel(apply_gamma_inv_gray<color_type, GammaLut>(g));
        }

        //--------------------------------------------------------------------
        template<class RenBuf2>
        void copy_from(const RenBuf2& from, 
                       int xdst, int ydst,
                       int xsrc, int ysrc,
                       unsigned len)
        {
            const int8u* p = from.row_ptr(ysrc);
            if(p)
            {
                memmove(m_rbuf->row_ptr(xdst, ydst, len) + xdst * pix_width, 
                        p + xsrc * pix_width, 
                        len * pix_width);
            }
        }

    private:
        rbuf_type* m_rbuf;
    };

    typedef blender_gray<gray8>      blender_gray8;
    typedef blender_gray_pre<gray8>  blender_gray8_pre;
    typedef blender_gray<gray16>     blender_gray16;
    typedef blender_gray_pre<gray16> blender_gray16_pre;

    typedef pixfmt_alpha_blend_gray<blender_gray8,      rendering_buffer> pixfmt_gray8;      //----pixfmt_gray8
    typedef pixfmt_alpha_blend_gray<blender_gray8_pre,  rendering_buffer> pixfmt_gray8_pre;  //----pixfmt_gray8_pre
    typedef pixfmt_alpha_blend_gray<blender_gray16,     rendering_buffer> pixfmt_gray16;     //----pixfmt_gray16
    typedef pixfmt_alpha_blend_gray<blender_gray16_pre, rendering_buffer> pixfmt_gray16_pre; //----pixfmt_gray16_pre
}

#endif

