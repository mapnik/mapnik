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
//  Credits:
//  I gratefully acknowledge the inspiring work of Maxim Shemanarev (McSeem),
//  author of Anti-Grain Geometry (http://www.antigrain.com). I have used
//  the datastructure from AGG as a template for my own.

#ifndef MAPNIK_VERTEX_VECTOR_HPP
#define MAPNIK_VERTEX_VECTOR_HPP

// mapnik
#include <mapnik/vertex.hpp>
#include <mapnik/util/noncopyable.hpp>

// stl
#include <algorithm>
#include <tuple>
#include <cstdint>

namespace mapnik {

template<typename T>
class vertex_vector : private util::noncopyable
{
    using coordinate_type = T;
    enum block_e { block_shift = 8, block_size = 1 << block_shift, block_mask = block_size - 1, grow_by = 256 };

  public:
    // required for iterators support
    using value_type = std::tuple<unsigned, coordinate_type, coordinate_type>;
    using size_type = std::size_t;
    using command_size = std::uint8_t;

  private:
    unsigned num_blocks_;
    unsigned max_blocks_;
    coordinate_type** vertices_;
    command_size** commands_;
    size_type pos_;

  public:

    vertex_vector()
        : num_blocks_(0),
          max_blocks_(0),
          vertices_(0),
          commands_(0),
          pos_(0)
    {}

    ~vertex_vector()
    {
        if (num_blocks_)
        {
            coordinate_type** vertices = vertices_ + num_blocks_ - 1;
            while (num_blocks_--)
            {
                ::operator delete(*vertices);
                --vertices;
            }
            ::operator delete(vertices_);
        }
    }
    size_type size() const { return pos_; }

    void push_back(coordinate_type x, coordinate_type y, command_size command)
    {
        size_type block = pos_ >> block_shift;
        if (block >= num_blocks_)
        {
            allocate_block(block);
        }
        coordinate_type* vertex = vertices_[block] + ((pos_ & block_mask) << 1);
        command_size* cmd = commands_[block] + (pos_ & block_mask);

        *cmd = static_cast<command_size>(command);
        *vertex++ = x;
        *vertex = y;
        ++pos_;
    }
    unsigned get_vertex(unsigned pos, coordinate_type* x, coordinate_type* y) const
    {
        if (pos >= pos_)
            return SEG_END;
        size_type block = pos >> block_shift;
        const coordinate_type* vertex = vertices_[block] + ((pos & block_mask) << 1);
        *x = (*vertex++);
        *y = (*vertex);
        return commands_[block][pos & block_mask];
    }

    void set_command(unsigned pos, unsigned command)
    {
        if (pos < pos_)
        {
            size_type block = pos >> block_shift;
            commands_[block][pos & block_mask] = command;
        }
    }

  private:
    void allocate_block(size_type block)
    {
        if (block >= max_blocks_)
        {
            coordinate_type** new_vertices =
              static_cast<coordinate_type**>(::operator new(sizeof(coordinate_type*) * ((max_blocks_ + grow_by) * 2)));
            command_size** new_commands = (command_size**)(new_vertices + max_blocks_ + grow_by);
            if (vertices_)
            {
                std::copy(vertices_, vertices_ + max_blocks_, new_vertices);
                std::copy(commands_, commands_ + max_blocks_, new_commands);
                ::operator delete(vertices_);
            }
            vertices_ = new_vertices;
            commands_ = new_commands;
            max_blocks_ += grow_by;
        }
        vertices_[block] = static_cast<coordinate_type*>(
          ::operator new(sizeof(coordinate_type) * (block_size * 2 + block_size / (sizeof(coordinate_type)))));

        commands_[block] = (command_size*)(vertices_[block] + block_size * 2);
        ++num_blocks_;
    }
};

} // namespace mapnik

#endif // MAPNIK_VERTEX_VECTOR_HPP
