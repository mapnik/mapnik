/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_GRID_PIXFMT_HPP
#define MAPNIK_GRID_PIXFMT_HPP

#include <string>
#include <mapnik/grid/grid_rendering_buffer.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include "agg_basics.h"
#include <mapnik/grid/grid_pixel.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik
{

//============================================================blender_gray
template<typename ColorT> struct blender_gray
{
    using color_type = ColorT;
    using value_type = typename color_type::value_type;
    using calc_type = typename color_type::calc_type;
    enum base_scale_e { base_shift = color_type::base_shift };

    static AGG_INLINE void blend_pix(value_type* p, unsigned cv,
                                     unsigned alpha, unsigned /*cover*/=0)
    {
        *p = (value_type)((((cv - calc_type(*p)) * alpha) + (calc_type(*p) << base_shift)) >> base_shift);
    }
};



//=====================================================apply_gamma_dir_gray
template<typename ColorT, class GammaLut> class apply_gamma_dir_gray
{
public:
    using value_type = typename ColorT::value_type;

    apply_gamma_dir_gray(const GammaLut& gamma) : m_gamma(gamma) {}

    AGG_INLINE void operator () (value_type* p)
    {
        *p = m_gamma.dir(*p);
    }

private:
    const GammaLut& m_gamma;
};



//=====================================================apply_gamma_inv_gray
template<typename ColorT, class GammaLut> class apply_gamma_inv_gray
{
public:
    using value_type = typename ColorT::value_type;

    apply_gamma_inv_gray(const GammaLut& gamma) : m_gamma(gamma) {}

    AGG_INLINE void operator () (value_type* p)
    {
        *p = m_gamma.inv(*p);
    }

private:
    const GammaLut& m_gamma;
};



//=================================================pixfmt_alpha_blend_gray
template<typename Blender, class RenBuf, unsigned Step=1, unsigned Offset=0>
class pixfmt_alpha_blend_gray
{
public:
    using rbuf_type = RenBuf  ;
    using row_data = typename rbuf_type::row_data;
    using blender_type = Blender ;
    using color_type = typename blender_type::color_type;
    using order_type = int                              ; // A fake one
    using value_type = typename color_type::value_type  ;
    using calc_type = typename color_type::calc_type   ;
    enum base_scale_e
    {
        base_shift = color_type::base_shift,
        base_scale = color_type::base_scale,
        base_mask  = color_type::base_mask,
        pix_width  = sizeof(value_type),
        pix_step   = Step,
        pix_offset = Offset
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
    explicit pixfmt_alpha_blend_gray(rbuf_type& rb) :
        m_rbuf(&rb)
    {}
    void attach(rbuf_type& rb) { m_rbuf = &rb; }
    //--------------------------------------------------------------------

    template<typename PixFmt>
    bool attach(PixFmt& pixf, int x1, int y1, int x2, int y2)
    {
        agg::rect_i r(x1, y1, x2, y2);
        if(r.clip(agg::rect_i(0, 0, pixf.width()-1, pixf.height()-1)))
        {
            int stride = pixf.stride();
            m_rbuf->attach(pixf.pix_ptr(r.x1, stride < 0 ? r.y2 : r.y1),
                           (r.x2 - r.x1) + 1,
                           (r.y2 - r.y1) + 1,
                           stride);
            return true;
        }
        return false;
    }

    //--------------------------------------------------------------------
    AGG_INLINE unsigned width()  const { return m_rbuf->width();  }
    AGG_INLINE unsigned height() const { return m_rbuf->height(); }
    AGG_INLINE int      stride() const { return m_rbuf->stride(); }

    //--------------------------------------------------------------------
    agg::int8u* row_ptr(int y)       { return m_rbuf->row_ptr(y); }
    const agg::int8u* row_ptr(int y) const { return m_rbuf->row_ptr(y); }
    row_data     row(int y)     const { return m_rbuf->row(y); }

    const agg::int8u* pix_ptr(int x, int y) const
    {
        return m_rbuf->row_ptr(y) + x * Step + Offset;
    }

    agg::int8u* pix_ptr(int x, int y)
    {
        return m_rbuf->row_ptr(y) + x * Step + Offset;
    }

    //--------------------------------------------------------------------
    AGG_INLINE static void make_pix(agg::int8u* p, const color_type& c)
    {
        *(value_type*)p = c.v;
    }

    //--------------------------------------------------------------------
    AGG_INLINE color_type pixel(int x, int y) const
    {
        value_type* p = (value_type*)m_rbuf->row_ptr(y) + x * Step + Offset;
        return color_type(*p);
    }

    //--------------------------------------------------------------------
    AGG_INLINE void copy_pixel(int x, int y, const color_type& c)
    {
        *((value_type*)m_rbuf->row_ptr(x, y, 1) + x * Step + Offset) = c.v;
    }

    //--------------------------------------------------------------------
    AGG_INLINE void blend_pixel(int x, int y, const color_type& c, agg::int8u cover)
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
                     agg::int8u /*cover*/)
    {
        value_type* p = (value_type*)
            m_rbuf->row_ptr(x, y, len) + x * Step + Offset;
        do
        {
            *p = c.v;
            p += Step;
        }
        while(--len);
        // We ignore alpha since grid_renderer is a binary renderer for now
        /*if (c.a)
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
        }*/
    }


    //--------------------------------------------------------------------
    void blend_vline(int x, int y,
                     unsigned len,
                     const color_type& c,
                     agg::int8u cover)
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
                           const agg::int8u* covers)
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
                           const agg::int8u* covers)
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
            *p = colors->v;
            p += Step;
            ++colors;
        }
        while(--len);
    }


    //--------------------------------------------------------------------
    void copy_color_vspan(int x, int y,
                          unsigned len,
                          const color_type* colors)
    {
        do
        {
            value_type* p = (value_type*)
                m_rbuf->row_ptr(x, y++, 1) + x * Step + Offset;
            *p = colors->v;
            ++colors;
        }
        while(--len);
    }


    //--------------------------------------------------------------------
    void blend_color_hspan(int x, int y,
                           unsigned len,
                           const color_type* colors,
                           const agg::int8u* covers,
                           agg::int8u cover)
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
                           const agg::int8u* covers,
                           agg::int8u cover)
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
    template <typename Function>
    void for_each_pixel(Function f)
    {
        for(unsigned y = 0; y < height(); ++y)
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
    template<typename GammaLut> void apply_gamma_dir(const GammaLut& g)
    {
        for_each_pixel(apply_gamma_dir_gray<color_type, GammaLut>(g));
    }

    //--------------------------------------------------------------------
    template<typename GammaLut> void apply_gamma_inv(const GammaLut& g)
    {
        for_each_pixel(apply_gamma_inv_gray<color_type, GammaLut>(g));
    }

    //--------------------------------------------------------------------
    template<typename RenBuf2>
    void copy_from(const RenBuf2& from,
                   int xdst, int ydst,
                   int xsrc, int ysrc,
                   unsigned len)
    {
        const agg::int8u* p = from.row_ptr(ysrc);
        if(p)
        {
            memmove(m_rbuf->row_ptr(xdst, ydst, len) + xdst * pix_width,
                    p + xsrc * pix_width,
                    len * pix_width);
        }
    }

    //--------------------------------------------------------------------
    template<typename SrcPixelFormatRenderer>
    void blend_from_color(const SrcPixelFormatRenderer& from,
                          const color_type& color,
                          int xdst, int ydst,
                          int /*xsrc*/, int ysrc,
                          unsigned len,
                          agg::int8u cover)
    {
        using src_value_type = typename SrcPixelFormatRenderer::value_type;
        const src_value_type* psrc = (src_value_type*)from.row_ptr(ysrc);
        if(psrc)
        {
            value_type* pdst =
                (value_type*)m_rbuf->row_ptr(xdst, ydst, len) + xdst;
            do
            {
                copy_or_blend_pix(pdst,
                                  color,
                                  (*psrc * cover + base_mask) >> base_shift);
                ++psrc;
                ++pdst;
            }
            while(--len);
        }
    }

    //--------------------------------------------------------------------
    template<typename SrcPixelFormatRenderer>
    void blend_from_lut(const SrcPixelFormatRenderer& from,
                        const color_type* color_lut,
                        int xdst, int ydst,
                        int /*xsrc*/, int ysrc,
                        unsigned len,
                        agg::int8u cover)
    {
        using src_value_type = typename SrcPixelFormatRenderer::value_type;
        const src_value_type* psrc = (src_value_type*)from.row_ptr(ysrc);
        if(psrc)
        {
            value_type* pdst =
                (value_type*)m_rbuf->row_ptr(xdst, ydst, len) + xdst;
            do
            {
                copy_or_blend_pix(pdst, color_lut[*psrc], cover);
                ++psrc;
                ++pdst;
            }
            while(--len);
        }
    }

private:
    rbuf_type* m_rbuf;
};

using blender_gray16 = blender_gray<gray16>;

using pixfmt_gray16 =  pixfmt_alpha_blend_gray<blender_gray16,
                                               mapnik::grid_rendering_buffer>;     //----pixfmt_gray16

using blender_gray32 = blender_gray<gray32>;

using  pixfmt_gray32 = pixfmt_alpha_blend_gray<blender_gray32,
                                               mapnik::grid_rendering_buffer>;     //----pixfmt_gray32

using blender_gray64 = blender_gray<gray64>;

using pixfmt_gray64 = pixfmt_alpha_blend_gray<blender_gray64,
                                              mapnik::grid_rendering_buffer>;     //----pixfmt_gray64

}

#endif
