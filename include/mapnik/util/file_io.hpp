/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_FILE_IO_HPP
#define MAPNIK_FILE_IO_HPP

// mapnik
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/util/utf_conv_win.hpp>

// stl
#include <cstdio>
#include <memory>
#include <string>

namespace mapnik {
namespace util {

class file : public util::noncopyable
{
  public:
    using file_ptr = std::unique_ptr<std::FILE, int (*)(std::FILE*)>;
    using data_type = std::unique_ptr<char[]>;

    explicit file(std::string const& filename)
#ifdef _WIN32
        : file_(_wfopen(mapnik::utf8_to_utf16(filename).c_str(), L"rb"), std::fclose)
        ,
#else
        : file_(std::fopen(filename.c_str(), "rb"), std::fclose)
        ,
#endif
        size_(0)

    {
        if (file_)
        {
            std::fseek(file_.get(), 0, SEEK_END);
            size_ = std::ftell(file_.get());
            std::fseek(file_.get(), 0, SEEK_SET);
        }
    }

    inline bool is_open() const
    {
        return file_ ? true : false;
    }

    explicit operator bool() const
    {
        return this->is_open();
    }

    inline std::FILE* get() const
    {
        return file_.get();
    }

    inline std::size_t size() const
    {
        return size_;
    }

    inline data_type data() const
    {
        if (!size_)
            return nullptr;
        std::fseek(file_.get(), 0, SEEK_SET);
        data_type buffer(new char[size_]);
        auto count = std::fread(buffer.get(), size_, 1, file_.get());
        if (count != 1)
            return nullptr;
        return buffer;
    }

  private:
    file_ptr file_;
    std::size_t size_;
};

} // namespace util
} // namespace mapnik

#endif // FILE_IO
