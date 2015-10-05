/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_QUAD_TREE_HPP
#define MAPNIK_QUAD_TREE_HPP

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/make_unique.hpp>
// stl
#include <algorithm>
#include <vector>
#include <type_traits>

#include <cstring>

namespace mapnik
{
template <typename T>
class quad_tree : util::noncopyable
{
    using value_type = T;
    struct node
    {
        using cont_type = std::vector<T>;
        using iterator = typename cont_type::iterator;
        using const_iterator = typename cont_type::const_iterator;
        box2d<double> extent_;
        cont_type cont_;
        node * children_[4];

        explicit node(box2d<double> const& ext)
            : extent_(ext)
        {
            std::fill(children_, children_ + 4, nullptr);
        }

        box2d<double> const& extent() const
        {
            return extent_;
        }

        iterator begin()
        {
            return cont_.begin();
        }

        const_iterator begin() const
        {
            return cont_.begin();
        }

        iterator end()
        {
            return cont_.end();
        }

        const_iterator end() const
        {
            return cont_.end();
        }

        int num_subnodes() const
        {
            int count = 0;
            for (int i = 0; i < 4; ++i)
            {
                if (children_[i]) ++count;
            }
            return count;
        }
        ~node () {}
    };

    using nodes_type = std::vector<std::unique_ptr<node> >;
    using cont_type = typename node::cont_type;
    using node_data_iterator = typename cont_type::iterator;

public:
    using iterator = typename nodes_type::iterator;
    using const_iterator = typename nodes_type::const_iterator;
    using result_type = typename std::vector<std::reference_wrapper<T> >;
    using query_iterator = typename result_type::iterator;

    explicit quad_tree(box2d<double> const& ext,
                       unsigned int max_depth = 8,
                       double ratio = 0.55)
        : max_depth_(max_depth),
          ratio_(ratio),
          query_result_(),
          nodes_()
    {
        nodes_.push_back(std::make_unique<node>(ext));
        root_ = nodes_[0].get();
    }

    void insert(T data, box2d<double> const& box)
    {
        unsigned int depth=0;
        do_insert_data(data,box,root_,depth);
    }

    query_iterator query_in_box(box2d<double> const& box)
    {
        query_result_.clear();
        query_node(box, query_result_, root_);
        return query_result_.begin();
    }

    query_iterator query_end()
    {
        return query_result_.end();
    }

    const_iterator begin() const
    {
        return nodes_.begin();
    }


    const_iterator end() const
    {
        return  nodes_.end();
    }

    void clear ()
    {
        box2d<double> ext = root_->extent_;
        nodes_.clear();
        nodes_.push_back(std::make_unique<node>(ext));
        root_ = nodes_[0].get();
    }

    box2d<double> const& extent() const
    {
        return root_->extent_;
    }

    int count() const
    {
        return count_nodes(root_);
    }

    int count_items() const
    {
        int count = 0;
        count_items(root_, count);
        return count;
    }
    void trim()
    {
        trim_tree(root_);
    }

    template <typename OutputStream>
    void write(OutputStream & out)
    {
        static_assert(std::is_standard_layout<value_type>::value,
                      "Values stored in quad-tree must be standard layout types to allow serialisation");
        char header[16];
        std::memset(header,0,16);
        header[0]='m';
        header[1]='a';
        header[2]='p';
        header[3]='n';
        header[4]='i';
        header[5]='k';
        out.write(header,16);
        write_node(out,root_);
    }
private:

    void query_node(box2d<double> const& box, result_type & result, node * node_) const
    {
        if (node_)
        {
            box2d<double> const& node_extent = node_->extent();
            if (box.intersects(node_extent))
            {
                for (auto & n : *node_)
                {
                    result.push_back(std::ref(n));
                }
                for (int k = 0; k < 4; ++k)
                {
                    query_node(box,result,node_->children_[k]);
                }
            }
        }
    }

    void do_insert_data(T data, box2d<double> const& box, node * n, unsigned int& depth)
    {
        if (++depth >= max_depth_)
        {
            n->cont_.push_back(data);
        }
        else
        {
            box2d<double> const& node_extent = n->extent();
            box2d<double> ext[4];
            split_box(node_extent,ext);
            for (int i = 0; i < 4; ++i)
            {
                if (ext[i].contains(box))
                {
                    if (!n->children_[i])
                    {
                        nodes_.push_back(std::make_unique<node>(ext[i]));
                        n->children_[i]=nodes_.back().get();
                    }
                    do_insert_data(data,box,n->children_[i],depth);
                    return;
                }
            }
            n->cont_.push_back(data);
        }
    }

    void split_box(box2d<double> const& node_extent,box2d<double> * ext)
    {
        double width=node_extent.width();
        double height=node_extent.height();

        double lox=node_extent.minx();
        double loy=node_extent.miny();
        double hix=node_extent.maxx();
        double hiy=node_extent.maxy();

        ext[0]=box2d<double>(lox,loy,lox + width * ratio_,loy + height * ratio_);
        ext[1]=box2d<double>(hix - width * ratio_,loy,hix,loy + height * ratio_);
        ext[2]=box2d<double>(lox,hiy - height*ratio_,lox + width * ratio_,hiy);
        ext[3]=box2d<double>(hix - width * ratio_,hiy - height*ratio_,hix,hiy);
    }

    void trim_tree(node *& n)
    {
        if (n)
        {
            for (int i = 0; i < 4; ++i)
            {
                trim_tree(n->children_[i]);
            }
            if (n->num_subnodes() == 1 && n->cont_.size() == 0)
            {
                for (int i = 0; i < 4; ++i)
                {
                    if (n->children_[i])
                    {
                        n = n->children_[i];
                        break;
                    }
                }
            }
        }

    }

    int count_nodes(node const* n) const
    {
        if (!n) return 0;
        else
        {
            int count = 1;
            for (int i = 0; i < 4; ++i)
            {
                count += count_nodes(n->children_[i]);
            }
            return count;
        }
    }

    void count_items(node const* n,int& count) const
    {
        if (n)
        {
            count += n->cont_.size();
            for (int i = 0; i < 4; ++i)
            {
                count_items(n->children_[i],count);
            }
        }
    }

    int subnode_offset(node const* n) const
    {
        int offset = 0;
        for (int i = 0; i < 4; i++)
        {
            if (n->children_[i])
            {
                offset +=sizeof(box2d<double>) + (n->children_[i]->cont_.size() * sizeof(value_type)) + 3 * sizeof(int);
                offset +=subnode_offset(n->children_[i]);
            }
        }
        return offset;
    }

    template <typename OutputStream>
    void write_node(OutputStream & out, node const* n) const
    {
        if (n)
        {
            int offset=subnode_offset(n);
            int shape_count=n->cont_.size();
            int recsize=sizeof(box2d<double>) + 3 * sizeof(int) + shape_count * sizeof(value_type);
            std::unique_ptr<char[]> node_record(new char[recsize]);
            std::memset(node_record.get(), 0, recsize);
            std::memcpy(node_record.get(), &offset, 4);
            std::memcpy(node_record.get() + 4, &n->extent_, sizeof(box2d<double>));
            std::memcpy(node_record.get() + 36, &shape_count, 4);
            for (int i=0; i < shape_count; ++i)
            {
                memcpy(node_record.get() + 40 + i * sizeof(value_type), &(n->cont_[i]),sizeof(value_type));
            }
            int num_subnodes=0;
            for (int i = 0; i < 4; ++i)
            {
                if (n->children_[i])
                {
                    ++num_subnodes;
                }
            }
            std::memcpy(node_record.get() + 40 + shape_count * sizeof(value_type),&num_subnodes,4);
            out.write(node_record.get(),recsize);
            for (int i = 0; i < 4; ++i)
            {
                write_node(out, n->children_[i]);
            }
        }
    }

    const unsigned int max_depth_;
    const double ratio_;
    result_type query_result_;
    nodes_type nodes_;
    node * root_;

};
}

#endif // MAPNIK_QUAD_TREE_HPP
