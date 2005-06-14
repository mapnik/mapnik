/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//  Credits:
//  I gratefully acknowledge the inspiring work of Maxim Shemanarev (McSeem), 
//  author of Anti-Grain Geometry (http://www.antigrain.com), and also the developers 
//  of the FreeType library (http://www.freetype.org). I have slightly modified the polygon 
//  rasterizing algorithm to work with my library, but render_line and 
//  render_hline remain intact. 


//$Id: scanline_aa.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef SCANLINE_AA_HPP
#define SCANLINE_AA_HPP

#include "envelope.hpp"
#include "geometry.hpp"
#include "graphics.hpp"
#include "style.hpp"

namespace mapnik
{
    enum path_commands_e
	{
	    path_cmd_stop     = 0,                    //----path_cmd_stop
	    path_cmd_move_to  = 1,                    //----path_cmd_move_to
	    path_cmd_line_to  = 2,                    //----path_cmd_line_to
	    path_cmd_curve3   = 3,                    //----path_cmd_curve3
	    path_cmd_curve4   = 4,                    //----path_cmd_curve4
	    path_cmd_end_poly = 6,                    //----path_cmd_end_poly
	    path_cmd_mask     = 0x0F                  //----path_cmd_mask
	};

    //------------------------------------------------------------path_flags_e
    enum path_flags_e
	{
	    path_flags_none  = 0,                     //----path_flags_none
	    path_flags_ccw   = 0x10,                  //----path_flags_ccw
	    path_flags_cw    = 0x20,                  //----path_flags_cw
	    path_flags_close = 0x40,                  //----path_flags_close
	    path_flags_mask  = 0xF0                   //----path_flags_mask
	};

    inline bool is_vertex(unsigned c)
    {
        return c >= path_cmd_move_to && c < path_cmd_end_poly;
    }

    inline bool is_move_to(unsigned c)
    {
        return c == path_cmd_move_to;
    }

    inline bool is_close(unsigned c)
    {
        return (c & ~(path_flags_cw | path_flags_ccw)) ==
            (path_cmd_end_poly | path_flags_close);
    }

    inline unsigned clipping_flags(int x, int y, const Envelope<int>& clip_box)
    {
        return  (x > clip_box.maxx()) |
            ((y > clip_box.maxy()) << 1) |
            ((x < clip_box.minx()) << 2) |
            ((y < clip_box.miny()) << 3);
    }

    template<class T>
    inline unsigned clip_liang_barsky(T x1, T y1, T x2, T y2,
				      const Envelope<T>& clip_box,
				      T* x, T* y)
    {
        const double nearzero = 1e-30;

        double deltax = x2 - x1;
        double deltay = y2 - y1;
        double xin;
        double xout;
        double yin;
        double yout;
        double tinx;
        double tiny;
        double toutx;
        double touty;
        double tin1;
        double tin2;
        double tout1;
        unsigned np = 0;

        if(deltax == 0.0)
        {
            // bump off of the vertical
            deltax = (x1 > clip_box.minx()) ? -nearzero : nearzero;
        }

        if(deltay == 0.0)
        {
            // bump off of the horizontal
            deltay = (y1 > clip_box.miny()) ? -nearzero : nearzero;
        }

        if(deltax > 0.0)
        {
            // points to right
            xin  = clip_box.minx();
            xout = clip_box.maxx();
        }
        else
        {
            xin  = clip_box.maxx();
            xout = clip_box.minx();
        }

        if(deltay > 0.0)
        {
            // points up
            yin  = clip_box.miny();
            yout = clip_box.maxy();
        }
        else
        {
            yin  = clip_box.maxy();
            yout = clip_box.miny();
        }

        tinx = (xin - x1) / deltax;
        tiny = (yin - y1) / deltay;

        if (tinx < tiny)
        {
            // hits x first
            tin1 = tinx;
            tin2 = tiny;
        }
        else
        {
            // hits y first
            tin1 = tiny;
            tin2 = tinx;
        }

        if(tin1 <= 1.0)
        {
            if(0.0 < tin1)
            {
                *x++ = (T)xin;
                *y++ = (T)yin;
                ++np;
            }

            if(tin2 <= 1.0)
            {
                toutx = (xout - x1) / deltax;
                touty = (yout - y1) / deltay;

                tout1 = (toutx < touty) ? toutx : touty;

                if(tin2 > 0.0 || tout1 > 0.0)
                {
                    if(tin2 <= tout1)
                    {
                        if(tin2 > 0.0)
                        {
                            if(tinx > tiny)
                            {
                                *x++ = (T)xin;
                                *y++ = (T)(y1 + tinx * deltay);
                            }
                            else
                            {
                                *x++ = (T)(x1 + tiny * deltax);
                                *y++ = (T)yin;
                            }
                            ++np;
                        }

                        if(tout1 < 1.0)
                        {
                            if(toutx < touty)
                            {
                                *x++ = (T)xout;
                                *y++ = (T)(y1 + toutx * deltay);
                            }
                            else
                            {
                                *x++ = (T)(x1 + touty * deltax);
                                *y++ = (T)yout;
                            }
                        }
                        else
                        {
                            *x++ = x2;
                            *y++ = y2;
                        }
                        ++np;
                    }
                    else
                    {
                        if(tinx > tiny)
                        {
                            *x++ = (T)xin;
                            *y++ = (T)yout;
                        }
                        else
                        {
                            *x++ = (T)xout;
                            *y++ = (T)yin;
                        }
                        ++np;
                    }
                }
            }
        }
        return np;
    }

    enum
	{
	    poly_base_shift = 8,
	    poly_base_size  = 1 << poly_base_shift,
	    poly_base_mask  = poly_base_size - 1
	};

    inline int poly_coord(double c)
    {
        return int(c * poly_base_size);
    }

    struct cell_aa
    {
        short x;
        short y;
        int   packed_coord;
        int   cover;
        int   area;

        void set(int x, int y, int c, int a);
        void set_coord(int x, int y);
        void set_cover(int c, int a);
        void add_cover(int c, int a);
    };

    class outline_aa
    {
        enum {
	    cell_block_shift = 12,
	    cell_block_size  = 1 << cell_block_shift,
	    cell_block_mask  = cell_block_size - 1,
	    cell_block_pool  = 256,
	    cell_block_limit = 1024
	};

    public:

	~outline_aa();
	outline_aa();

	void reset();

	void move_to(int x, int y);
	void line_to(int x, int y);

	int min_x() const { return m_min_x; }
	int min_y() const { return m_min_y; }
	int max_x() const { return m_max_x; }
	int max_y() const { return m_max_y; }

	const cell_aa* const* cells();
	unsigned num_cells() { cells(); return m_num_cells; }
	bool sorted() const { return m_sorted; }

    private:
	outline_aa(const outline_aa&);
	const outline_aa& operator = (const outline_aa&);

	void set_cur_cell(int x, int y);
	void add_cur_cell();
	void sort_cells();
	void render_hline(int ey, int x1, int y1, int x2, int y2);
	void render_line(int x1, int y1, int x2, int y2);
	void allocate_block();

	static void qsort_cells(cell_aa** start, unsigned num);

    private:
	unsigned  m_num_blocks;
	unsigned  m_max_blocks;
	unsigned  m_cur_block;
	unsigned  m_num_cells;
	cell_aa** m_cells;
	cell_aa*  m_cur_cell_ptr;
	cell_aa** m_sorted_cells;
	unsigned  m_sorted_size;
	cell_aa   m_cur_cell;
	int       m_cur_x;
	int       m_cur_y;
	int       m_min_x;
	int       m_min_y;
	int       m_max_x;
	int       m_max_y;
	bool      m_sorted;
    };

    enum filling_rule_e
	{
	    fill_non_zero,
	    fill_even_odd
	};

    template <typename PixBuffer> class ScanlineRasterizerAA
    {
        enum status
	    {
		status_initial,
		status_line_to,
		status_closed
	    };

        struct iterator
        {
            const cell_aa* const* cells;
            int                   cover;
            int                   last_y;
        };
        enum
	    {
		aa_shift = 8,
		aa_num   = 1 << aa_shift,
		aa_mask  = aa_num - 1,
		aa_2num  = aa_num * 2,
		aa_2mask = aa_2num - 1
	    };
    private:
	PixBuffer*     pixbuf_;
	outline_aa     m_outline;
	int            m_gamma[aa_num];
	filling_rule_e m_filling_rule;
	int            m_clipped_start_x;
	int            m_clipped_start_y;
	int            m_start_x;
	int            m_start_y;
	int            m_prev_x;
	int            m_prev_y;
	unsigned       m_prev_flags;
	unsigned       m_status;
	Envelope<int>  m_clip_box;
	bool           m_clipping;
	iterator       m_iterator;
    public:
	ScanlineRasterizerAA(PixBuffer& pixbuf)
	    :pixbuf_(&pixbuf),
	     m_filling_rule(fill_non_zero),
	     m_clipped_start_x(0),
	     m_clipped_start_y(0),
	     m_start_x(0),
	     m_start_y(0),
	     m_prev_x(0),
	     m_prev_y(0),
	     m_prev_flags(0),
	     m_status(status_initial),
	     m_clipping(false)
	{
	    for(int i = 0; i < aa_num; i++) m_gamma[i] = i;
	}
	
	template <typename Transform>
	void render(const geometry_type& geom,const Color& c);

    private:
	ScanlineRasterizerAA(const ScanlineRasterizerAA&);
	ScanlineRasterizerAA& operator=(const ScanlineRasterizerAA&);
	void render_hline(int x0,int x1,int y,unsigned rgba);
	void blend_hline(int x0,int x1,int y,const unsigned char* cover,
			 unsigned rgba);
	int min_x() const { return m_outline.min_x(); }
	int min_y() const { return m_outline.min_y(); }
	int max_x() const { return m_outline.max_x(); }
	int max_y() const { return m_outline.max_y(); }
	void reset();
	void filling_rule(filling_rule_e filling_rule);
	void clip_box(double x1, double y1, double x2, double y2);
	void reset_clipping();
	template<class GammaF> void gamma(const GammaF& gamma_function)
	{
	    int i;
	    for(i = 0; i < aa_num; i++)
	    {
		m_gamma[i] = int(floor(gamma_function(double(i) / aa_mask) * aa_mask + 0.5));
	    }
	}
	unsigned apply_gamma(unsigned cover) const
	{
	    return m_gamma[cover];
	}
	void add_vertex(double x, double y, unsigned cmd);
	void move_to(int x, int y);
	void line_to(int x, int y);
	void close_polygon();

	void move_to_no_clip(int x, int y);
	void line_to_no_clip(int x, int y);
	void close_polygon_no_clip();
	void clip_segment(int x, int y);
	unsigned calculate_alpha(int area) const
	{
	    int cover = area >> (poly_base_shift*2 + 1 - aa_shift);

	    if(cover < 0) cover = -cover;
	    if(m_filling_rule == fill_even_odd)
	    {
		cover &= aa_2mask;
		if(cover > aa_num)
		{
		    cover = aa_2num - cover;
		}
	    }
	    if(cover > aa_mask) cover = aa_mask;
	    return m_gamma[cover];
	}


	void sort()
	{
	    m_outline.cells();
	}


	bool rewind_scanlines()
	{
	    close_polygon();
	    m_iterator.cells = m_outline.cells();
	    if(m_outline.num_cells() == 0)
	    {
		return false;
	    }
	    m_iterator.cover  = 0;
	    m_iterator.last_y = (*m_iterator.cells)->y;
	    return true;
	}


	template<class Scanline> bool sweep_scanline(Scanline& sl)
	{
	    sl.reset_spans();
	    for(;;)
	    {
		const cell_aa* cur_cell = *m_iterator.cells;
		if(cur_cell == 0) return false;
		++m_iterator.cells;
		m_iterator.last_y = cur_cell->y;

		for(;;)
		{
		    int coord  = cur_cell->packed_coord;
		    int area   = cur_cell->area;
		    int last_x = cur_cell->x;

		    m_iterator.cover += cur_cell->cover;

		    //accumulate all cells with the same coordinates
		    for(; (cur_cell = *m_iterator.cells) != 0; ++m_iterator.cells)
		    {
			if(cur_cell->packed_coord != coord) break;
			area             += cur_cell->area;
			m_iterator.cover += cur_cell->cover;
		    }

		    int alpha;
		    if(cur_cell == 0 || cur_cell->y != m_iterator.last_y)
		    {

			if(area)
			{
			    alpha = calculate_alpha((m_iterator.cover << (poly_base_shift + 1)) - area);
			    if(alpha)
			    {
				sl.add_cell(last_x, alpha);
			    }
			    ++last_x;
			}
			break;
		    }

		    ++m_iterator.cells;

		    if(area)
		    {
			alpha = calculate_alpha((m_iterator.cover << (poly_base_shift + 1)) - area);
			if(alpha)
			{
			    sl.add_cell(last_x, alpha);
			}
			++last_x;
		    }

		    if(cur_cell->x > last_x)
		    {
			alpha = calculate_alpha(m_iterator.cover << (poly_base_shift + 1));
			if(alpha)
			{
			    sl.add_span(last_x, cur_cell->x - last_x, alpha);
			}
		    }
		}
		if(sl.num_spans())
		{
		    sl.finalize(m_iterator.last_y);
		    break;
		}
	    }
	    return true;
	}
    };

    template<typename PixBuffer>
    void  ScanlineRasterizerAA<PixBuffer>::reset()
    {
        m_outline.reset();
        m_status = status_initial;
    }

    template<typename PixBuffer>
    void ScanlineRasterizerAA<PixBuffer>::clip_box(double x1, double y1, double x2, double y2)
    {
        //reset();
        m_clip_box = Envelope<int>(poly_coord(x1), poly_coord(y1),
				   poly_coord(x2), poly_coord(y2));
        //m_clip_box.normalize();
        m_clipping = true;
    }

    template<typename PixBuffer>
    void ScanlineRasterizerAA<PixBuffer>::move_to_no_clip(int x, int y)
    {
        if(m_status == status_line_to)
        {
            close_polygon_no_clip();
        }
        m_outline.move_to(x,y);
        m_clipped_start_x = x;
        m_clipped_start_y = y;
        m_status = status_line_to;
    }

    template<typename PixBuffer>
    void ScanlineRasterizerAA<PixBuffer>::line_to_no_clip(int x, int y)
    {
        if(m_status != status_initial)
        {
            m_outline.line_to(x , y);
            m_status = status_line_to;
        }
    }

    template<typename PixBuffer>
    void ScanlineRasterizerAA<PixBuffer>::close_polygon_no_clip()
    {
        if(m_status == status_line_to)
        {
            m_outline.line_to(m_clipped_start_x, m_clipped_start_y);
            m_status = status_closed;
        }
    }

    template<typename PixBuffer>
    void ScanlineRasterizerAA<PixBuffer>::clip_segment(int x, int y)
    {
        unsigned flags = clipping_flags(x, y, m_clip_box);
        if(m_prev_flags == flags)
        {
            if(flags == 0)
            {
                if(m_status == status_initial)
                {
                    move_to_no_clip(x, y);
                }
                else
                {
                    line_to_no_clip(x, y);
                }
            }
        }
        else
        {

            int cx[4];
            int cy[4];
            unsigned n = clip_liang_barsky<int>(m_prev_x, m_prev_y,
						x, y,
						m_clip_box,
						cx, cy);
            const int* px = cx;
            const int* py = cy;
            while(n--)
            {
                if(m_status == status_initial)
                {
                    move_to_no_clip(*px++, *py++);
                }
                else
                {
                    line_to_no_clip(*px++, *py++);
                }
            }
        }

        m_prev_flags = flags;
        m_prev_x = x;
        m_prev_y = y;
    }

    template<typename PixBuffer>
    void ScanlineRasterizerAA<PixBuffer>::add_vertex(double x, double y, unsigned cmd)
    {
        if(is_close(cmd))
        {
            close_polygon();
        }
        else
        {
            if(is_move_to(cmd))
            {
                move_to(poly_coord(x), poly_coord(y));
            }
            else
            {
                if(is_vertex(cmd))
                {
                    line_to(poly_coord(x), poly_coord(y));
                }
            }
        }
    }

    template<typename PixBuffer>
    void ScanlineRasterizerAA<PixBuffer>::move_to(int x, int y)
    {
        if(m_clipping)
        {
            if(m_outline.sorted())
            {
                reset();
            }
            if(m_status == status_line_to)
            {
                close_polygon();
            }
            m_prev_x = m_start_x = x;
            m_prev_y = m_start_y = y;
            m_status = status_initial;
            m_prev_flags = clipping_flags(x, y, m_clip_box);
            if(m_prev_flags == 0)
            {
                move_to_no_clip(x, y);
            }
        }
        else
        {
            move_to_no_clip(x, y);
        }
    }

    template<typename PixBuffer>
    void ScanlineRasterizerAA<PixBuffer>::line_to(int x, int y)
    {
        if(m_clipping)
        {
            clip_segment(x, y);
        }
        else
        {
            line_to_no_clip(x, y);
        }
    }

    template<typename PixBuffer>
    void ScanlineRasterizerAA<PixBuffer>::close_polygon()
    {
        if(m_clipping)
        {
            clip_segment(m_start_x, m_start_y);
        }
        close_polygon_no_clip();
    }


    template<class T> class scanline_u
    {
    public:
	typedef T cover_type;
	struct span
	{
	    short x;
	    short len;
	    cover_type* covers;
	};

	typedef span* iterator;
	typedef const span* const_iterator;

	~scanline_u();
	scanline_u();

	void     reset(int min_x, int max_x);
	void     add_cell(int x, unsigned cover);
	void     add_cells(int x, unsigned len, const T* covers);
	void     add_span(int x, unsigned len, unsigned cover);
	void     finalize(int y) { m_y = y; }
	void     reset_spans();

	int      y()           const { return m_y; }
	unsigned num_spans()   const { return unsigned(m_cur_span - m_spans); }
	const_iterator begin() const { return m_spans + 1; }
	iterator       begin()       { return m_spans + 1; }

    private:
	scanline_u<T>(const scanline_u<T>&);
	const scanline_u<T>& operator = (const scanline_u<T>&);

    private:
	int           m_min_x;
	unsigned      m_max_len;
	int           m_last_x;
	int           m_y;
	cover_type*   m_covers;
	span*         m_spans;
	span*         m_cur_span;
    };

    template<class T> scanline_u<T>::~scanline_u()
    {
        delete [] m_spans;
        delete [] m_covers;
    }

    template<class T> scanline_u<T>::scanline_u() :
	m_min_x(0),
        m_max_len(0),
        m_last_x(0x7FFFFFF0),
        m_covers(0),
        m_spans(0),
        m_cur_span(0)
    {
    }

    template<class T> void scanline_u<T>::reset(int min_x, int max_x)
    {
        unsigned max_len = max_x - min_x + 2;
        if(max_len > m_max_len)
        {
            delete [] m_spans;
            delete [] m_covers;
            m_covers  = new cover_type [max_len];
            m_spans   = new span       [max_len];
            m_max_len = max_len;
        }
        m_last_x        = 0x7FFFFFF0;
        m_min_x         = min_x;
        m_cur_span      = m_spans;
    }

    template<class T> inline void scanline_u<T>::reset_spans()
    {
        m_last_x    = 0x7FFFFFF0;
        m_cur_span  = m_spans;
    }

    template<class T> inline void scanline_u<T>::add_cell(int x, unsigned cover)
    {
        x -= m_min_x;
        m_covers[x] = (unsigned char)cover;
        if(x == m_last_x+1)
        {
            m_cur_span->len++;
        }
        else
        {
            m_cur_span++;
            m_cur_span->x      = (short)(x + m_min_x);
            m_cur_span->len    = 1;
            m_cur_span->covers = m_covers + x;
        }
        m_last_x = x;
    }

    template<class T> void scanline_u<T>::add_cells(int x, unsigned len, const T* covers)
    {
        x -= m_min_x;
        memcpy(m_covers + x, covers, len * sizeof(T));
        if(x == m_last_x+1)
        {
            m_cur_span->len += (short)len;
        }
        else
        {
            m_cur_span++;
            m_cur_span->x      = (short)(x + m_min_x);
            m_cur_span->len    = (short)len;
            m_cur_span->covers = m_covers + x;
        }
        m_last_x = x + len - 1;
    }

    template<class T> void scanline_u<T>::add_span(int x, unsigned len, unsigned cover)
    {
        x -= m_min_x;
        memset(m_covers + x, cover, len);
        if(x == m_last_x+1)
        {
            m_cur_span->len += (short)len;
        }
        else
        {
            m_cur_span++;
            m_cur_span->x      = (short)(x + m_min_x);
            m_cur_span->len    = (short)len;
            m_cur_span->covers = m_covers + x;
        }
        m_last_x = x + len - 1;
    }

    typedef scanline_u<unsigned char> scanline_u8;
    typedef scanline_u<unsigned short> scanline_u16;
    typedef scanline_u<unsigned int> scanline_u32;
}

#endif //SCANLINE_AA_HPP
