/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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


//$Id: vertex_vector.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef VERTEX_VECTOR_HPP
#define VERTEX_VECTOR_HPP
// mapnik
#include <mapnik/vertex.hpp>
#include <mapnik/ctrans.hpp>
// boost
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>
// stl
#include <vector>
#include <cstring>

namespace mapnik
{
    template <typename T>
    class vertex_vector : private boost::noncopyable
    {
        typedef typename T::type value_type;
        typedef vertex<value_type,2> vertex_type;
        enum block_e {
            block_shift = 8,
            block_size  = 1<<block_shift,
            block_mask  = block_size - 1,
            grow_by     = 256
        };

    private:
        unsigned num_blocks_;
        unsigned max_blocks_;
        value_type** vertexs_;
        unsigned char** commands_;
        unsigned pos_;
    public:
	
        vertex_vector() 
            : num_blocks_(0),
              max_blocks_(0),
              vertexs_(0),
              commands_(0),
              pos_(0) {}

        ~vertex_vector()
        {
            if ( num_blocks_ )
            {
                value_type** vertexs=vertexs_ + num_blocks_ - 1;
                while ( num_blocks_-- )
                {
                    delete [] *vertexs;
                    --vertexs;
                }
                delete [] vertexs_;
            }
        }
        unsigned size() const 
        {
            return pos_;
        }
	
        void push_back (value_type x,value_type y,unsigned command)
        {
            unsigned block = pos_ >> block_shift;
            if (block >= num_blocks_)
            {
                allocate_block(block);
            }
            value_type* vertex = vertexs_[block] + ((pos_ & block_mask) << 1);
            unsigned char* cmd= commands_[block] + (pos_ & block_mask);
	    
            *cmd = static_cast<unsigned char>(command);
            *vertex++ = x;
            *vertex   = y;
            ++pos_;
        }
        unsigned get_vertex(unsigned pos,value_type* x,value_type* y) const
        {
            if (pos >= pos_) return SEG_END;
            unsigned block = pos >> block_shift;
            const value_type* vertex = vertexs_[block] + (( pos & block_mask) << 1);
            *x = (*vertex++);
            *y = (*vertex);
            return commands_[block] [pos & block_mask];
        }
        	
        void set_capacity(size_t)
        {
            //do nothing
        }
        
    private:
        void allocate_block(unsigned block)
        {
            if (block >= max_blocks_)
            {
                value_type** new_vertexs = new value_type* [(max_blocks_ + grow_by) * 2];
                unsigned char** new_commands = (unsigned char**)(new_vertexs + max_blocks_ + grow_by);
                if (vertexs_)
                {
                    std::memcpy(new_vertexs,vertexs_,max_blocks_ * sizeof(value_type*));
                    std::memcpy(new_commands,commands_,max_blocks_ * sizeof(unsigned char*));
                    delete [] vertexs_;
                }
                vertexs_ = new_vertexs;
                commands_ = new_commands;
                max_blocks_ += grow_by;
            }
            vertexs_[block] = new value_type [block_size * 2 + block_size / (sizeof(value_type))];
            commands_[block] = (unsigned char*)(vertexs_[block] + block_size*2);
            ++num_blocks_;
        }
    };

    template <typename T>
    struct vertex_vector2 : boost::noncopyable
    {
        typedef typename T::type value_type;
        typedef boost::tuple<value_type,value_type,char> vertex_type;
        typedef typename std::vector<vertex_type>::const_iterator const_iterator;
        vertex_vector2() {}
        unsigned size() const 
        {
            return cont_.size();
        }

        void push_back (value_type x,value_type y,unsigned command)
        {
            cont_.push_back(vertex_type(x,y,command));
        }
        unsigned get_vertex(unsigned pos,value_type* x,value_type* y) const
        {
            if (pos >= cont_.size()) return SEG_END;
            vertex_type const& c = cont_[pos];
            *x = boost::get<0>(c);
            *y = boost::get<1>(c);
            return boost::get<2>(c);
        }
        
        const_iterator begin() const
        {
            return cont_.begin();
        }
        
        const_iterator end() const
        {
            return cont_.end();
        }

        void set_capacity(size_t size)
        {
            cont_.reserve(size);
        }
    private:
        std::vector<vertex_type> cont_;
    };
}

#endif //VERTEX_VECTOR_HPP
