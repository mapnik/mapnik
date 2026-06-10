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

#ifndef MAPNIK_TEXT_CLUSTERS_CONTAINER_HPP
#define MAPNIK_TEXT_CLUSTERS_CONTAINER_HPP

#include <cstddef>
#include <cassert>
#include <iterator>
#include <utility>
#include <vector>

namespace mapnik {
namespace detail {

template<typename T>
class clusters_container
{
  public:
    using value_type = T;
    using container_type = std::vector<value_type>;
    using reference = typename container_type::reference;

    class iterator
    {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        iterator() = default;

        reference operator*() const { return owner_->slots_[owner_->physical_index(offset_)]; }
        pointer operator->() const { return &owner_->slots_[owner_->physical_index(offset_)]; }

        iterator& operator++()
        {
            ++offset_;
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(iterator const& lhs, iterator const& rhs)
        {
            return lhs.owner_ == rhs.owner_ && lhs.offset_ == rhs.offset_;
        }

        friend bool operator!=(iterator const& lhs, iterator const& rhs) { return !(lhs == rhs); }

      private:
        friend class clusters_container;

        iterator(clusters_container* owner, std::size_t offset)
            : owner_(owner),
              offset_(offset)
        {}

        clusters_container* owner_ = nullptr;
        std::size_t offset_ = 0;
    };

    bool empty() const noexcept { return size_ == 0; }
    std::size_t size() const noexcept { return size_; }

    void reset(std::size_t count)
    {
        head_ = 0;
        size_ = 0;
        if (count > slots_.size())
            slots_.resize(count);
    }

    reference front() { return slots_[head_]; }
    reference back() { return slots_[physical_index(size_ - 1)]; }

    template<typename U>
    void push_back(U&& value)
    {
        assert(size_ < slots_.size());
        slots_[physical_index(size_)] = std::forward<U>(value);
        ++size_;
    }

    template<typename U>
    void push_front(U&& value)
    {
        assert(size_ < slots_.size());
        head_ = (head_ + slots_.size() - 1) % slots_.size();
        slots_[head_] = std::forward<U>(value);
        ++size_;
    }

    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, size_); }

  private:
    std::size_t physical_index(std::size_t logical_index) const { return (head_ + logical_index) % slots_.size(); }

    container_type slots_;
    std::size_t head_ = 0;
    std::size_t size_ = 0;
};

} // namespace detail
} // namespace mapnik

#endif // MAPNIK_TEXT_CLUSTERS_CONTAINER_HPP
