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

namespace mapnik
{
template <typename T>
class quad_tree : util::noncopyable
{
    struct node
    {
        using value_t = T;
        using cont_t = std::vector<T>;
        using iterator = typename cont_t::iterator;
        using const_iterator = typename cont_t::const_iterator;
        box2d<double> extent_;
        cont_t cont_;
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
        ~node () {}
    };

    using nodes_t = std::vector<std::unique_ptr<node> >;
    using cont_t = typename node::cont_t;
    using node_data_iterator = typename cont_t::iterator;

public:
    using iterator = typename nodes_t::iterator;
    using const_iterator = typename nodes_t::const_iterator;
    using result_t = typename std::vector<std::reference_wrapper<T> >;
    using query_iterator = typename result_t::iterator;

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

private:

    void query_node(box2d<double> const& box, result_t & result, node * node_) const
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

    const unsigned int max_depth_;
    const double ratio_;
    result_t query_result_;
    nodes_t nodes_;
    node * root_;

};
}

#endif // MAPNIK_QUAD_TREE_HPP
