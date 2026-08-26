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

#include <harfbuzz/hb.h>

#include <cassert>
#include <limits>
#include <utility>
#include <vector>

namespace mapnik {
namespace detail {

struct font_coverage
{
    static constexpr std::size_t uncovered_sentinel_ = std::numeric_limits<std::size_t>::max();

    struct range
    {
        unsigned start;
        unsigned end;
    };

    struct covering_iterator
    {
        using value_type = std::size_t;

        covering_iterator() = default;

        static covering_iterator make_end(font_coverage const* owner, bool reverse)
        {
            covering_iterator itr;
            itr.emit_reverse_ = reverse;
            itr.next_coverage_offset_ = (!reverse && owner) ? owner->coverage_.size() : 0;
            return itr;
        }

        covering_iterator(font_coverage const* owner, bool reverse)
            : owner_(owner),
              emit_reverse_(reverse),
              next_coverage_offset_((reverse && owner) ? owner->coverage_.size() : 0)
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
            return owner_ == rhs.owner_ && emit_reverse_ == rhs.emit_reverse_ &&
                   next_coverage_offset_ == rhs.next_coverage_offset_;
        }

        bool operator!=(covering_iterator const& rhs) const { return !(*this == rhs); }

      private:
        void advance()
        {
            if (!owner_)
                return;

            auto const coverage_size = owner_->coverage_.size();
            // The current shaping flow should not leave uncovered slots behind:
            // the last font is used for any still-uncovered ranges. Keep the
            // sentinel checks below anyway so iteration does not depend on that
            // invariant.
            if (!emit_reverse_)
            {
                while (next_coverage_offset_ < coverage_size)
                {
                    auto const covering_index = owner_->coverage_[next_coverage_offset_];
                    ++next_coverage_offset_;
                    if (covering_index == uncovered_sentinel_)
                        continue;
                    // coverage_ stores one glyphinfos index per covered UTF-16 code unit.
                    // Skip duplicate entries so iteration emits each contiguous covering
                    // glyph cluster exactly once. For example, [7, 7, x, 9, 9, 9, 12]
                    // yields 7, 9, 12 in forward order; `x` is the uncovered marker.
                    if (next_coverage_offset_ > 1 && owner_->coverage_[next_coverage_offset_ - 2] == covering_index)
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
                    if (covering_index == uncovered_sentinel_)
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
        }

        font_coverage const* owner_ = nullptr;
        bool emit_reverse_ = false;
        std::size_t next_coverage_offset_ = 0;
        value_type current_covering_index_ = uncovered_sentinel_;
    };

    font_coverage(unsigned start, unsigned end)
        : start_(start),
          end_(end),
          coverage_(end_ - start_, uncovered_sentinel_)
    {
        if (start_ < end_)
            current_uncovered_ranges_.push_back({start_, end_});
    }

    bool fully_covered() const
    {
        return current_uncovered_offset_ >= current_uncovered_ranges_.size() && next_uncovered_ranges_.empty();
    }

    bool has_current_uncovered() const { return current_uncovered_offset_ < current_uncovered_ranges_.size(); }

    range pop_current_uncovered_front()
    {
        assert(has_current_uncovered());
        auto range = current_uncovered_ranges_[current_uncovered_offset_++];
        return range;
    }

    void advance_uncovered_ranges()
    {
        current_uncovered_ranges_.swap(next_uncovered_ranges_);
        current_uncovered_offset_ = 0;
        next_uncovered_ranges_.clear();
    }

    void push_uncovered(unsigned start, unsigned end)
    {
        if (start >= end)
            return;

        unsigned clamped_start = std::max(start, start_);
        unsigned clamped_end = std::min(end, end_);
        if (clamped_start >= clamped_end)
            return;

        range new_range = {clamped_start, clamped_end};
        // Callers must append uncovered ranges in text order. Given that
        // monotonic invariant we only need to coalesce with the most recently
        // appended range.
        if (!next_uncovered_ranges_.empty())
        {
            auto& last_range = next_uncovered_ranges_.back();
            assert(last_range.start <= new_range.start);
            if (last_range.end >= new_range.start)
            {
                last_range.end = std::max(last_range.end, new_range.end);
                return;
            }
        }
        next_uncovered_ranges_.push_back(new_range);
    }

    void reserve(std::size_t uncovered_range_count) { next_uncovered_ranges_.reserve(uncovered_range_count); }

    void cover(unsigned start, unsigned end, std::size_t glyphinfo_index)
    {
        if (start >= end)
            return;
        assert(glyphinfo_index != uncovered_sentinel_);
        unsigned clamped_start = std::max(start, start_);
        unsigned clamped_end = std::min(end, end_);
        for (unsigned i = clamped_start; i < clamped_end; ++i)
        {
            assert(coverage_[i - start_] == uncovered_sentinel_);
            coverage_[i - start_] = glyphinfo_index;
        }
    }

    covering_iterator covering_begin(bool rtl) const { return covering_iterator(this, rtl); }
    covering_iterator covering_end(bool rtl) const { return covering_iterator::make_end(this, rtl); }

  private:
    unsigned start_;
    unsigned end_;
    std::vector<range> current_uncovered_ranges_;
    std::size_t current_uncovered_offset_ = 0;
    std::vector<range> next_uncovered_ranges_;
    std::vector<std::size_t> coverage_;
};

} // namespace detail
} // namespace mapnik

#endif // MAPNIK_TEXT_FONT_COVERAGE_HPP
