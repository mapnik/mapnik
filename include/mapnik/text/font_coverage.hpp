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

#ifndef MAPNIK_TEXT_FONT_COVERAGE_HPP
#define MAPNIK_TEXT_FONT_COVERAGE_HPP

#include <mapnik/text/face.hpp>

#include <harfbuzz/hb.h>

#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

namespace mapnik {
namespace detail {

struct glyph_face_info
{
    face_ptr face;
    hb_glyph_info_t glyph;
    hb_glyph_position_t position;
};

struct font_coverage
{
    static constexpr std::size_t uncovered_ = std::numeric_limits<std::size_t>::max();

    struct range
    {
        unsigned start;
        unsigned end;
    };

    struct covering_iterator
    {
        using value_type = std::size_t;

        covering_iterator() = default;
        covering_iterator(font_coverage const* owner, bool reverse)
            : owner_(owner),
              emit_reverse_(reverse),
              next_coverage_offset_(reverse && owner ? owner->coverage_.size() : 0)
        {
            advance();
        }

        value_type operator*() const { return current_covering_index_; }
        covering_iterator& operator++()
        {
            advance();
            return *this;
        }
        bool operator==(covering_iterator const& rhs) const
        {
            return owner_ == rhs.owner_ && next_coverage_offset_ == rhs.next_coverage_offset_;
        }
        bool operator!=(covering_iterator const& rhs) const { return !(*this == rhs); }

      private:
        void advance()
        {
            if (!owner_)
                return;

            auto const coverage_size = owner_->coverage_.size();
            if (!emit_reverse_)
            {
                while (next_coverage_offset_ < coverage_size)
                {
                    auto const covering_index = owner_->coverage_[next_coverage_offset_];
                    ++next_coverage_offset_;
                    if (covering_index == uncovered_)
                        continue;
                    // coverage_ stores one glyphinfos index per covered UTF-16 code unit.
                    // Skip duplicate entries so iteration emits each contiguous covering
                    // glyph cluster exactly once. For example, [7, 7, x, 9, 9, 9, 12]
                    // yields 7, 9, 12 in forward order; `x` is the uncovered marker.
                    if (next_coverage_offset_ > 1 &&
                        owner_->coverage_[next_coverage_offset_ - 2] == covering_index)
                        continue;
                    current_covering_index_ = covering_index;
                    return;
                }
            }
            else
            {
                while (next_coverage_offset_ > 0)
                {
                    --next_coverage_offset_;
                    auto const covering_index = owner_->coverage_[next_coverage_offset_];
                    if (covering_index == uncovered_)
                        continue;
                    // The same deduplication rule applies in reverse. The same example
                    // [7, 7, x, 9, 9, 9, 12] yields 12, 9, 7. After stepping left, look
                    // one slot to the right; if it carries the same covering index then
                    // this is another code unit from the same contiguous cluster, so skip it.
                    if (next_coverage_offset_ + 1 < coverage_size &&
                        owner_->coverage_[next_coverage_offset_ + 1] == covering_index)
                        continue;
                    current_covering_index_ = covering_index;
                    return;
                }
            }

            owner_ = nullptr;
            // Forward iteration exhausts with a non-zero offset, so normalize it
            // back to covering_end(); reverse iteration already finishes at 0.
            next_coverage_offset_ = 0;
        }

        font_coverage const* owner_ = nullptr;
        bool emit_reverse_ = false;
        std::size_t next_coverage_offset_ = 0;
        value_type current_covering_index_ = uncovered_;
    };

    font_coverage(unsigned start, unsigned end)
        : start_(start),
          end_(end),
          coverage_(end_ - start_, uncovered_)
    {
        if (start_ < end_)
            current_uncovered_ranges_.push_back({start_, end_});
    }

    bool fully_covered() const
    {
        return next_uncovered_ranges_.empty() && current_uncovered_index_ == current_uncovered_ranges_.size();
    }

    void begin_try_cover_pass()
    {
        next_uncovered_ranges_.clear();
        current_uncovered_index_ = 0;
    }

    void finish_try_cover_pass()
    {
        for (; current_uncovered_index_ < current_uncovered_ranges_.size(); ++current_uncovered_index_)
        {
            next_uncovered_ranges_.push_back(current_uncovered_ranges_[current_uncovered_index_]);
        }
        current_uncovered_ranges_.swap(next_uncovered_ranges_);
        next_uncovered_ranges_.clear();
        current_uncovered_index_ = 0;
    }

    void cover(unsigned start, unsigned end, std::size_t glyphinfo_index)
    {
        if (start >= end)
            return;
        assert(glyphinfo_index != uncovered_);
        unsigned clamped_start = std::max(start, start_);
        unsigned clamped_end = std::min(end, end_);
        for (unsigned i = clamped_start; i < clamped_end; ++i)
        {
            auto const coverage_index = i - start_;
            assert(coverage_[coverage_index] == uncovered_);
            coverage_[coverage_index] = glyphinfo_index;
        }
    }

    bool try_cover(unsigned start, unsigned end, std::size_t glyphinfo_index)
    {
        if (start >= end)
            return false;
        assert(glyphinfo_index != uncovered_);

        unsigned clamped_start = std::max(start, start_);
        unsigned clamped_end = std::min(end, end_);
        if (clamped_start >= clamped_end)
            return false;

        while (current_uncovered_index_ < current_uncovered_ranges_.size() &&
               current_uncovered_ranges_[current_uncovered_index_].end <= clamped_start)
        {
            next_uncovered_ranges_.push_back(current_uncovered_ranges_[current_uncovered_index_]);
            ++current_uncovered_index_;
        }

        if (current_uncovered_index_ >= current_uncovered_ranges_.size())
            return false;

        auto& uncovered_range = current_uncovered_ranges_[current_uncovered_index_];
        if (clamped_start < uncovered_range.start || clamped_end > uncovered_range.end)
            return false;

        if (uncovered_range.start < clamped_start)
            next_uncovered_ranges_.push_back({uncovered_range.start, clamped_start});

        cover(clamped_start, clamped_end, glyphinfo_index);

        if (clamped_end < uncovered_range.end)
        {
            uncovered_range.start = clamped_end;
        }
        else
        {
            ++current_uncovered_index_;
        }
        return true;
    }

    covering_iterator covering_begin(bool rtl) const { return covering_iterator(this, rtl); }
    covering_iterator covering_end() const { return covering_iterator(); }

  private:
    unsigned start_;
    unsigned end_;
    std::vector<std::size_t> coverage_;
    std::vector<range> current_uncovered_ranges_;
    std::vector<range> next_uncovered_ranges_;
    std::size_t current_uncovered_index_ = 0;
};

} // namespace detail
} // namespace mapnik

#endif // MAPNIK_TEXT_FONT_COVERAGE_HPP
