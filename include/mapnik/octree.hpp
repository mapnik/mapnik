/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_OCTREE_HPP
#define MAPNIK_OCTREE_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/palette.hpp>
#include <mapnik/noncopyable.hpp>

// stl
#include <vector>
#include <cstring>
#include <deque>
#include <algorithm>

namespace mapnik {

struct RGBPolicy
{
    const static unsigned MAX_LEVELS = 6;
    const static unsigned MIN_LEVELS = 2;
    inline static unsigned index_from_level(unsigned level, rgb const& c)
    {
        unsigned shift = 7 - level;
        return (((c.r >> shift) & 1) << 2)
            | (((c.g >> shift) & 1) << 1)
            | ((c.b >> shift) & 1);
    }
};

template <typename T, typename InsertPolicy = RGBPolicy >
class octree : private mapnik::noncopyable
{
    struct node
    {
        node()
            :  reds(0),
               greens(0),
               blues(0),
               count(0),
               count_cum(0),
               children_count(0),
               index(0)
        {
            std::memset(&children_[0],0,sizeof(children_));
        }

        ~node()
        {
            for (unsigned i = 0;i < 8; ++i)
            {
                if (children_[i] != 0)
                {
                    delete children_[i];
                    children_[i]=0;
                }
            }
        }

        bool is_leaf() const
        {
            return count == 0;
        }
        node * children_[8];
        boost::uint64_t reds;
        boost::uint64_t greens;
        boost::uint64_t blues;
        unsigned count;
        double reduce_cost;
        unsigned count_cum;
        byte children_count;
        byte index;
    };
    struct node_cmp
    {
        bool operator() ( const node * lhs,const node* rhs) const
        {
            return lhs->reduce_cost < rhs->reduce_cost;
        }
    };

    std::deque<node*> reducible_[InsertPolicy::MAX_LEVELS];
    unsigned max_colors_;
    unsigned colors_;
    unsigned offset_;
    unsigned leaf_level_;

public:
    explicit octree(unsigned max_colors=256)
        : max_colors_(max_colors),
          colors_(0),
          offset_(0),
          leaf_level_(InsertPolicy::MAX_LEVELS),
          root_(new node())
    {}

    ~octree()
    {
        delete root_;
    }

    unsigned colors()
    {
        return colors_;
    }

    void setMaxColors(unsigned max_colors)
    {
        max_colors_ = max_colors;
    }

    void setOffset(unsigned offset)
    {
        offset_ = offset;
    }

    unsigned getOffset()
    {
        return offset_;
    }

    void insert(T const& data)
    {
        unsigned level = 0;
        node * cur_node = root_;
        while (true)
        {
            cur_node->count_cum++;
            cur_node->reds   += data.r;
            cur_node->greens += data.g;
            cur_node->blues  += data.b;

            if ( cur_node->count > 0 || level == leaf_level_)
            {
                cur_node->count  += 1;
                if (cur_node->count == 1) ++colors_;
                //if (colors_ >= max_colors_ - 1)
                //reduce();
                break;
            }

            unsigned idx = InsertPolicy::index_from_level(level,data);
            if (cur_node->children_[idx] == 0)
            {
                cur_node->children_count++;
                cur_node->children_[idx] = new node();
                if (level < leaf_level_-1)
                {
                    reducible_[level+1].push_back(cur_node->children_[idx]);
                }
            }
            cur_node = cur_node->children_[idx];
            ++level;
        }
    }

    int quantize(unsigned val) const
    {
        unsigned level = 0;
        rgb c(val);
        node * cur_node = root_;
        while (cur_node)
        {
            if (cur_node->children_count == 0)
            {
                return cur_node->index + offset_;
            }
            unsigned idx = InsertPolicy::index_from_level(level,c);
            cur_node = cur_node->children_[idx];
            ++level;
        }
        return -1;
    }

    void create_palette(std::vector<rgb> & palette)
    {
        reduce();
        palette.reserve(colors_);
        create_palette(palette, root_);
    }

    void computeCost(node *r)
    {
        r->reduce_cost = 0;
        if (r->children_count==0)
        {
            return;
        }

        double mean_r = static_cast<double>(r->reds / r->count_cum);
        double mean_g = static_cast<double>(r->greens / r->count_cum);
        double mean_b = static_cast<double>(r->blues / r->count_cum);
        for (unsigned idx=0; idx < 8; ++idx)
        {
            if (r->children_[idx] != 0)
            {
                double dr,dg,db;
                computeCost(r->children_[idx]);
                dr = r->children_[idx]->reds   / r->children_[idx]->count_cum - mean_r;
                dg = r->children_[idx]->greens / r->children_[idx]->count_cum - mean_g;
                db = r->children_[idx]->blues  / r->children_[idx]->count_cum - mean_b;
                r->reduce_cost += r->children_[idx]->reduce_cost;
                r->reduce_cost += (dr*dr + dg*dg + db*db) * r->children_[idx]->count_cum;
            }
        }
    }

    void reduce()
    {
        computeCost(root_);
        reducible_[0].push_back(root_);

        // sort reducible by reduce_cost
        for (unsigned i=0;i<InsertPolicy::MAX_LEVELS;++i)
        {
            std::sort(reducible_[i].begin(), reducible_[i].end(),node_cmp());
        }
        while (colors_ > max_colors_ && colors_ > 1)
        {
            while (leaf_level_ >0  && reducible_[leaf_level_-1].size() == 0)
            {
                --leaf_level_;
            }

            if (leaf_level_ <= 0)
            {
                return;
            }

            // select best of all reducible:
            unsigned red_idx = leaf_level_-1;
            unsigned bestv = static_cast<unsigned>((*reducible_[red_idx].begin())->reduce_cost);
            for(unsigned i=red_idx; i>=InsertPolicy::MIN_LEVELS; i--)
            {
                if (!reducible_[i].empty())
                {
                    node *nd = *reducible_[i].begin();
                    unsigned gch = 0;
                    for(unsigned idx=0; idx<8; idx++)
                    {
                        if (nd->children_[idx])
                            gch += nd->children_[idx]->children_count;
                    }
                    if (gch==0 && nd->reduce_cost < bestv)
                    {
                        bestv = static_cast<unsigned>(nd->reduce_cost);
                        red_idx = i;
                    }
                }
            }

            typename std::deque<node*>::iterator pos = reducible_[red_idx].begin();
            node * cur_node = *pos;
            unsigned num_children = 0;
            for (unsigned idx=0; idx < 8; ++idx)
            {
                if (cur_node->children_[idx] != 0)
                {
                    cur_node->children_count--;
                    ++num_children;
                    cur_node->count  += cur_node->children_[idx]->count;
                    //todo: case of nonleaf children, if someday sorting by reduce_cost doesn't handle it
                    delete cur_node->children_[idx], cur_node->children_[idx]=0;
                }
            }

            reducible_[red_idx].erase(pos);
            if (num_children > 0 )
            {
                colors_ -= (num_children - 1);
            }
        }
    }

    void create_palette(std::vector<rgb> & palette, node * itr) const
    {
        if (itr->count != 0)
        {
            unsigned count = itr->count;
            palette.push_back(rgb(byte(itr->reds/float(count)),
                                  byte(itr->greens/float(count)),
                                  byte(itr->blues/float(count))));
            itr->index = static_cast<unsigned>(palette.size()) - 1;
        }
        for (unsigned i=0; i < 8 ;++i)
        {
            if (itr->children_[i] != 0)
            {
                create_palette(palette, itr->children_[i]);
            }
        }
    }
private:
    node * root_;
};

} // namespace mapnik

#endif // MAPNIK_OCTREE_HPP
