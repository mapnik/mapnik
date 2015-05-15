/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_null.hpp>
#include <mapnik/image_impl.hpp>
#include <mapnik/pixel_types.hpp>

namespace mapnik
{

namespace detail
{

// BUFFER
buffer::buffer(std::size_t size)
    : size_(size),
      data_(static_cast<unsigned char*>(size_ != 0 ? ::operator new(size_) : nullptr))
{}

buffer::buffer(buffer && rhs) noexcept
: size_(std::move(rhs.size_)),
    data_(std::move(rhs.data_))
{
    rhs.size_ = 0;
    rhs.data_ = nullptr;
}

buffer::buffer(buffer const& rhs)
    : size_(rhs.size_),
      data_(static_cast<unsigned char*>(size_ != 0 ? ::operator new(size_) : nullptr))
{
    if (data_) std::copy(rhs.data_, rhs.data_ + rhs.size_, data_);
}

buffer::~buffer()
{
    ::operator delete(data_);
}

buffer& buffer::operator=(buffer rhs)
{
    swap(rhs);
    return *this;
}

void buffer::swap(buffer & rhs)
{
    std::swap(size_, rhs.size_);
    std::swap(data_, rhs.data_);
}

unsigned char* buffer::data()
{
    return data_;
}

unsigned char const* buffer::data() const
{
    return data_;
}

std::size_t buffer::size() const
{
    return size_;
}

template struct MAPNIK_DECL image_dimensions<65535>;

} // end ns detail

template class MAPNIK_DECL image<null_t>;
template class MAPNIK_DECL image<rgba8_t>;
template class MAPNIK_DECL image<gray8_t>;
template class MAPNIK_DECL image<gray8s_t>;
template class MAPNIK_DECL image<gray16_t>;
template class MAPNIK_DECL image<gray16s_t>;
template class MAPNIK_DECL image<gray32_t>;
template class MAPNIK_DECL image<gray32s_t>;
template class MAPNIK_DECL image<gray32f_t>;
template class MAPNIK_DECL image<gray64_t>;
template class MAPNIK_DECL image<gray64s_t>;
template class MAPNIK_DECL image<gray64f_t>;

} // end ns
