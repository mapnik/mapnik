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

#ifndef MAPNIK_QUAD_TREE_HPP
#define MAPNIK_QUAD_TREE_HPP

// mapnik
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/util/noncopyable.hpp>

// stl
#include <memory>
#include <new>
#include <vector>
#include <type_traits>

#include <cstring>

namespace mapnik {
template<typename T0, typename T1 = box2d<double>>
class quad_tree : util::noncopyable
{
    using value_type = T0;
    using bbox_type = T1;
    struct node
    {
        using cont_type = std::vector<T0>;
        using iterator = typename cont_type::iterator;
        using const_iterator = typename cont_type::const_iterator;
        bbox_type extent_;
        cont_type cont_;
        node* children_[4];

        explicit node(bbox_type const& ext)
            : extent_(ext)
        {
            std::fill(children_, children_ + 4, nullptr);
        }

        bbox_type const& extent() const { return extent_; }

        iterator begin() { return cont_.begin(); }

        const_iterator begin() const { return cont_.begin(); }

        iterator end() { return cont_.end(); }

        const_iterator end() const { return cont_.end(); }

        int num_subnodes() const
        {
            int _count = 0;
            for (int i = 0; i < 4; ++i)
            {
                if (children_[i])
                    ++_count;
            }
            return _count;
        }
        ~node() {}
    };

    using nodes_type = std::vector<std::unique_ptr<node>>;
    using cont_type = typename node::cont_type;
    using node_data_iterator = typename cont_type::iterator;

  public:
    using iterator = typename nodes_type::iterator;
    using const_iterator = typename nodes_type::const_iterator;
    using result_type = typename std::vector<std::reference_wrapper<value_type>>;
    using query_iterator = typename result_type::iterator;

    explicit quad_tree(bbox_type const& ext, unsigned int max_depth = 8, double ratio = 0.55)
        : max_depth_(max_depth),
          ratio_(ratio),
          query_result_(),
          nodes_()
    {
        nodes_.push_back(std::make_unique<node>(ext));
        root_ = nodes_[0].get();
    }

    void insert(value_type const& data, bbox_type const& box)
    {
        unsigned int depth = 0;
        do_insert_data(data, box, root_, depth);
    }

    query_iterator query_in_box(bbox_type const& box)
    {
        query_result_.clear();
        query_node(box, query_result_, root_);
        return query_result_.begin();
    }

    query_iterator query_end() { return query_result_.end(); }

    const_iterator begin() const { return nodes_.begin(); }

    const_iterator end() const { return nodes_.end(); }

    void clear()
    {
        bbox_type ext = root_->extent_;
        nodes_.clear();
        nodes_.push_back(std::make_unique<node>(ext));
        root_ = nodes_[0].get();
    }

    bbox_type const& extent() const { return root_->extent_; }

    int count() const { return count_nodes(root_); }

    int count_items() const
    {
        int _count = 0;
        count_items(root_, _count);
        return _count;
    }
    void trim() { trim_tree(root_); }

    template<typename OutputStream>
    void write(OutputStream& out)
    {
        static_assert(std::is_standard_layout<value_type>::value,
                      "Values stored in quad-tree must be standard layout types to allow serialisation");
        char header[16];
        std::memset(header, 0, 16);
        std::strcpy(header, "mapnik-index");
        out.write(header, 16);
        write_node(out, root_);
    }

  private:

    void query_node(bbox_type const& box, result_type& result, node* node_) const
    {
        if (node_)
        {
            bbox_type const& node_extent = node_->extent();
            if (box.intersects(node_extent))
            {
                for (auto& n : *node_)
                {
                    result.push_back(std::ref(n));
                }
                for (int k = 0; k < 4; ++k)
                {
                    query_node(box, result, node_->children_[k]);
                }
            }
        }
    }

    void do_insert_data(value_type const& data, bbox_type const& box, node* n, unsigned int& depth)
    {
        if (++depth < max_depth_)
        {
            bbox_type const& node_extent = n->extent();
            bbox_type ext[4];
            split_box(node_extent, ext);
            for (int i = 0; i < 4; ++i)
            {
                if (ext[i].contains(box))
                {
                    if (!n->children_[i])
                    {
                        nodes_.push_back(std::make_unique<node>(ext[i]));
                        n->children_[i] = nodes_.back().get();
                    }
                    do_insert_data(data, box, n->children_[i], depth);
                    return;
                }
            }
        }
        n->cont_.push_back(data);
    }

    void split_box(bbox_type const& node_extent, bbox_type* ext)
    {
        typename bbox_type::value_type width = node_extent.width();
        typename bbox_type::value_type height = node_extent.height();
        typename bbox_type::value_type lox = node_extent.minx();
        typename bbox_type::value_type loy = node_extent.miny();
        typename bbox_type::value_type hix = node_extent.maxx();
        typename bbox_type::value_type hiy = node_extent.maxy();

        ext[0] = bbox_type(lox, loy, lox + width * ratio_, loy + height * ratio_);
        ext[1] = bbox_type(hix - width * ratio_, loy, hix, loy + height * ratio_);
        ext[2] = bbox_type(lox, hiy - height * ratio_, lox + width * ratio_, hiy);
        ext[3] = bbox_type(hix - width * ratio_, hiy - height * ratio_, hix, hiy);
    }

    void trim_tree(node*& n)
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
        if (!n)
            return 0;
        else
        {
            int _count = 1;
            for (int i = 0; i < 4; ++i)
            {
                _count += count_nodes(n->children_[i]);
            }
            return _count;
        }
    }

    void count_items(node const* n, int& _count) const
    {
        if (n)
        {
            _count += n->cont_.size();
            for (int i = 0; i < 4; ++i)
            {
                count_items(n->children_[i], _count);
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
                offset += sizeof(bbox_type) + (n->children_[i]->cont_.size() * sizeof(value_type)) + 3 * sizeof(int);
                offset += subnode_offset(n->children_[i]);
            }
        }
        return offset;
    }

    template<typename OutputStream>
    void write_node(OutputStream& out, node const* n) const
    {
        if (n)
        {
            int offset = subnode_offset(n);
            int shape_count = n->cont_.size();
            int recsize = sizeof(bbox_type) + 3 * sizeof(int) + shape_count * sizeof(value_type);
            std::unique_ptr<char[]> node_record(new char[recsize]);
            std::memset(node_record.get(), 0, recsize);
            std::memcpy(node_record.get(), &offset, 4);
            std::memcpy(node_record.get() + 4, &n->extent_, sizeof(bbox_type));
            std::memcpy(node_record.get() + 4 + sizeof(bbox_type), &shape_count, 4);
            for (int i = 0; i < shape_count; ++i)
            {
                memcpy(node_record.get() + 8 + sizeof(bbox_type) + i * sizeof(value_type),
                       &(n->cont_[i]),
                       sizeof(value_type));
            }
            int num_subnodes = 0;
            for (int i = 0; i < 4; ++i)
            {
                if (n->children_[i])
                {
                    ++num_subnodes;
                }
            }
            std::memcpy(node_record.get() + 8 + sizeof(bbox_type) + shape_count * sizeof(value_type), &num_subnodes, 4);
            out.write(node_record.get(), recsize);
            for (int i = 0; i < 4; ++i)
            {
                write_node(out, n->children_[i]);
            }
        }
    }

    unsigned int const max_depth_;
    double const ratio_;
    result_type query_result_;
    nodes_type nodes_;
    node* root_;
};
} // namespace mapnik

#endif // MAPNIK_QUAD_TREE_HPP
