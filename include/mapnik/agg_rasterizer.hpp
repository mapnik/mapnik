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

#ifndef MAPNIK_AGG_RASTERIZER_HPP
#define MAPNIK_AGG_RASTERIZER_HPP

// mapnik
#include <mapnik/util/noncopyable.hpp>

// stl
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

struct rasterizer : agg::rasterizer_scanline_aa<agg::rasterizer_sl_clip_int_sat>,
                    util::noncopyable
{
    // gamma(agg::gamma_power(g)), keeping the tables it computed: each takes aa_scale pow calls, and styles
    // switch between a few gamma values from one symbolizer to the next
    void gamma_power(double g)
    {
        for (auto const& entry : power_tables_)
        {
            if (entry.first == g)
            {
                gamma_table(entry.second.data());
                return;
            }
        }
        gamma(agg::gamma_power(g));
        if (power_tables_.size() < 16)
        {
            std::array<int, aa_scale> table;
            std::copy(gamma_table(), gamma_table() + aa_scale, table.begin());
            power_tables_.emplace_back(g, table);
        }
    }

    // Whether rewind_scanlines() and sweep_scanline() note what they find (see rendered()): only
    // the buffers of composited styles need it.
    void track_rendered(bool track) { track_ = track; }

    // The base's, noting the bounds of the cells it found: agg::render_scanlines and the like
    // call it before they render anything, and render only pixels within those bounds.
    bool rewind_scanlines()
    {
        bool const any = agg::rasterizer_scanline_aa<agg::rasterizer_sl_clip_int_sat>::rewind_scanlines();
        if (any && track_)
        {
            rendered_ = true;
            rendered_x1_ = std::min(rendered_x1_, min_x());
            rendered_y1_ = std::min(rendered_y1_, min_y());
            rendered_x2_ = std::max(rendered_x2_, max_x());
            rendered_y2_ = std::max(rendered_y2_, max_y());
        }
        return any;
    }

    // The base's, noting the columns of the scanline it swept: the row's pixels lie between its
    // first and its last span (they come in x order). The row is cleared first when the buffer
    // being drawn on is cleared lazily (only while tracking).
    template<class Scanline>
    bool sweep_scanline(Scanline& sl)
    {
        bool const any = agg::rasterizer_scanline_aa<agg::rasterizer_sl_clip_int_sat>::sweep_scanline(sl);
        if (any && track_)
        {
            int const y = sl.y();
            if (y >= 0)
            {
                if (lazy_clean_ && std::size_t(y) < lazy_rows_ && !lazy_clean_[y])
                {
                    std::memset(lazy_bytes_ + y * lazy_row_size_, 0, lazy_row_size_);
                    lazy_clean_[y] = 1;
                }
                auto const* first = sl.begin();
                auto const& last = first[sl.num_spans() - 1];
                // packed scanlines give a solid run of -len pixels
                int const len = last.len < 0 ? -last.len : last.len;
                note_row(y, first->x, last.x + len - 1);
            }
        }
        return any;
    }

    // The rows of the buffer the scanlines are rendered into are to be cleared as they are first
    // swept: clean[y] says whether row y is transparent already.
    void clear_rows_lazily(unsigned char* bytes, std::size_t row_size, std::uint8_t* clean, std::size_t rows)
    {
        lazy_bytes_ = bytes;
        lazy_row_size_ = row_size;
        lazy_clean_ = clean;
        lazy_rows_ = rows;
    }
    void clear_rows_eagerly() { lazy_clean_ = nullptr; }

    // A scanline to render with, kept from one symbolizer to the next: one made for each would
    // allocate its spans and covers again (resetting it for a render is all it needs).
    agg::scanline_u8& scanline() { return scanline_; }

    // Whether rewind_scanlines() found cells since the last forget_rendered(), and the pixels
    // (inclusive) all of them lie within.
    bool rendered() const { return rendered_; }
    int rendered_x1() const { return rendered_x1_; }
    int rendered_y1() const { return rendered_y1_; }
    int rendered_x2() const { return rendered_x2_; }
    int rendered_y2() const { return rendered_y2_; }
    // The columns (inclusive) within which row y was swept since the last forget_rendered(),
    // or false when it was not.
    bool rendered_row(int y, int& x1, int& x2) const
    {
        if (y < 0 || std::size_t(y) >= row_x1_.size() || row_x1_[y] > row_x2_[y])
        {
            return false;
        }
        x1 = row_x1_[y];
        x2 = row_x2_[y];
        return true;
    }
    void forget_rendered()
    {
        if (rendered_)
        {
            int const y2 = std::min(rendered_y2_, static_cast<int>(row_x1_.size()) - 1);
            for (int y = std::max(rendered_y1_, 0); y <= y2; ++y)
            {
                row_x1_[y] = std::numeric_limits<int>::max();
                row_x2_[y] = std::numeric_limits<int>::min();
            }
        }
        rendered_ = false;
        rendered_x1_ = std::numeric_limits<int>::max();
        rendered_y1_ = std::numeric_limits<int>::max();
        rendered_x2_ = std::numeric_limits<int>::min();
        rendered_y2_ = std::numeric_limits<int>::min();
    }

  private:
    void note_row(int y, int x1, int x2)
    {
        if (std::size_t(y) >= row_x1_.size())
        {
            row_x1_.resize(y + 1, std::numeric_limits<int>::max());
            row_x2_.resize(y + 1, std::numeric_limits<int>::min());
        }
        row_x1_[y] = std::min(row_x1_[y], x1);
        row_x2_[y] = std::max(row_x2_[y], x2);
    }

    std::vector<std::pair<double, std::array<int, aa_scale>>> power_tables_;
    agg::scanline_u8 scanline_;
    bool rendered_ = false;
    int rendered_x1_ = std::numeric_limits<int>::max();
    int rendered_y1_ = std::numeric_limits<int>::max();
    int rendered_x2_ = std::numeric_limits<int>::min();
    int rendered_y2_ = std::numeric_limits<int>::min();
    // per row, within rendered_y1_..rendered_y2_
    std::vector<int> row_x1_;
    std::vector<int> row_x2_;
    unsigned char* lazy_bytes_ = nullptr;
    std::size_t lazy_row_size_ = 0;
    std::uint8_t* lazy_clean_ = nullptr;
    std::size_t lazy_rows_ = 0;
    // see track_rendered()
    bool track_ = false;
};

} // namespace mapnik

#endif // MAPNIK_AGG_RASTERIZER_HPP
