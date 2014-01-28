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

#ifndef MAPNIK_HEXTREE_HPP
#define MAPNIK_HEXTREE_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/palette.hpp>
#include <mapnik/noncopyable.hpp>

// boost
#include <boost/range/algorithm.hpp>

// stl
#include <vector>
#include <cstring>
#include <set>
#include <algorithm>
#include <cmath>

namespace mapnik {

struct RGBAPolicy
{
    const static unsigned MAX_LEVELS = 6;
    const static unsigned MIN_ALPHA  = 5;
    const static unsigned MAX_ALPHA  = 250;
    inline static unsigned index_from_level(unsigned level, rgba const& c)
    {
        unsigned shift = 7 - level;
        return (((c.a >> shift) & 1) << 3)
            | (((c.r >> shift) & 1) << 2)
            | (((c.g >> shift) & 1) << 1)
            | ((c.b >> shift) & 1);
    }
};

template <typename T, typename InsertPolicy = RGBAPolicy >
class hextree : private mapnik::noncopyable
{
    struct node
    {
        node ()
            : reds(0),
              greens(0),
              blues(0),
              alphas(0),
              count(0),
              pixel_count(0),
              children_count(0)
        {
            std::memset(&children_[0],0,sizeof(children_));
        }

        ~node ()
        {
            for (unsigned i = 0; i < 16; ++i)
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
            return (children_count == 0);
        }
        node * children_[16];
        // sum of values for computing mean value using count or pixel_count
        double reds;
        double greens;
        double blues;
        double alphas;
        // if count!=0, then node represents color in output palette
        int count;
        // number of pixels represented by this subtree
        unsigned pixel_count;
        // penalty of using this node as color
        double reduce_cost;
        // number of !=0 positions in children_ array
        byte children_count;
    };

    // highest reduce_cost first
    struct node_rev_cmp
    {
        bool operator() (const node * lhs, const node* rhs) const
        {
            if (lhs->reduce_cost != rhs->reduce_cost)
            {
                return (lhs->reduce_cost > rhs->reduce_cost);
            }
            return (lhs > rhs);
        }
    };

    unsigned max_colors_;
    unsigned colors_;
    // flag indicating existance of invisible pixels (a < InsertPolicy::MIN_ALPHA)
    bool has_holes_;
    const std::unique_ptr<node> root_;
    // working palette for quantization, sorted on mean(r,g,b,a) for easier searching NN
    std::vector<rgba> sorted_pal_;
    // index remaping of sorted_pal_ indexes to indexes of returned image palette
    std::vector<unsigned> pal_remap_;
    // rgba hashtable for quantization
    mutable rgba_hash_table color_hashmap_;
    // gamma correction to prioritize dark colors (>1.0)
    double gamma_;
    // look up table for gamma correction
    double gammaLUT_[256];
    // transparency handling
    enum transparency_mode_t {NO_TRANSPARENCY=0, BINARY_TRANSPARENCY=1, FULL_TRANSPARENCY=2};
    unsigned trans_mode_;

    inline double gamma(double b, double g) const
    {
        return 255 * std::pow(b/255, g);
    }

public:
    explicit hextree(unsigned max_colors=256, double g=2.0)
        : max_colors_(max_colors),
          colors_(0),
          has_holes_(false),
          root_(new node()),
#ifdef USE_DENSE_HASH_MAP
          // TODO - test for any benefit to initializing at a larger size
          color_hashmap_(),
#endif
          trans_mode_(FULL_TRANSPARENCY)
    {
        setGamma(g);
#ifdef USE_DENSE_HASH_MAP
        color_hashmap_.set_empty_key(0);
#endif
    }

    ~hextree()
    {}

    void setMaxColors(unsigned max_colors)
    {
        max_colors_ = max_colors;
    }

    void setGamma(double g)
    {
        gamma_ = g;
        for (unsigned i=0; i<256; i++)
        {
            gammaLUT_[i] = gamma(i, 1/gamma_);
        }
    }

    void setTransMode(unsigned t)
    {
        trans_mode_ = t;
    }

    transparency_mode_t getTransMode() const
    {
        return trans_mode_;
    }

    // process alpha value based on trans_mode_
    byte preprocessAlpha(byte a) const
    {
        switch(trans_mode_)
        {
        case NO_TRANSPARENCY:
            return 255;
        case BINARY_TRANSPARENCY:
            return a<127?0:255;
        default:
            return a;
        }
    }

    void insert(T const& data)
    {
        byte a = preprocessAlpha(data.a);
        unsigned level = 0;
        node * cur_node = root_.get();
        if (a < InsertPolicy::MIN_ALPHA)
        {
            has_holes_ = true;
            return;
        }
        while (true)
        {
            cur_node->pixel_count++;
            cur_node->reds   += gammaLUT_[data.r];
            cur_node->greens += gammaLUT_[data.g];
            cur_node->blues  += gammaLUT_[data.b];
            cur_node->alphas += a;

            if (level == InsertPolicy::MAX_LEVELS)
            {
                if (cur_node->pixel_count == 1)
                {
                    ++colors_;
                }
                break;
            }

            unsigned idx = InsertPolicy::index_from_level(level,data);
            if (cur_node->children_[idx] == 0)
            {
                cur_node->children_count++;
                cur_node->children_[idx] = new node();
            }
            cur_node = cur_node->children_[idx];
            ++level;
        }
    }

    // return color index in returned earlier palette
    int quantize(unsigned val) const
    {
        byte a = preprocessAlpha(U2ALPHA(val));
        unsigned ind=0;
        if (a < InsertPolicy::MIN_ALPHA || colors_ == 0)
        {
            return 0;
        }
        if (colors_ == 1)
        {
            return pal_remap_[has_holes_?1:0];
        }

        rgba_hash_table::iterator it = color_hashmap_.find(val);
        if (it == color_hashmap_.end())
        {
            rgba c(val);
            int dr, dg, db, da;
            int dist, newdist;

            // find closest match based on mean of r,g,b,a
            std::vector<rgba>::const_iterator pit =
                boost::lower_bound(sorted_pal_, c, rgba::mean_sort_cmp());
            ind = pit-sorted_pal_.begin();
            if (ind == sorted_pal_.size())
                ind--;
            dr = sorted_pal_[ind].r - c.r;
            dg = sorted_pal_[ind].g - c.g;
            db = sorted_pal_[ind].b - c.b;
            da = sorted_pal_[ind].a - a;
            dist = dr*dr + dg*dg + db*db + da*da;
            int poz = ind;

            // search neighbour positions in both directions for better match
            for (int i = poz - 1; i >= 0; i--)
            {
                dr = sorted_pal_[i].r - c.r;
                dg = sorted_pal_[i].g - c.g;
                db = sorted_pal_[i].b - c.b;
                da = sorted_pal_[i].a - a;
                // stop criteria based on properties of used sorting
                if (((dr+db+dg+da) * (dr+db+dg+da) / 4 > dist))
                {
                    break;
                }
                newdist = dr*dr + dg*dg + db*db + da*da;
                if (newdist < dist)
                {
                    ind = i;
                    dist = newdist;
                }
            }
            for (unsigned i = poz + 1; i < sorted_pal_.size(); i++)
            {
                dr = sorted_pal_[i].r - c.r;
                dg = sorted_pal_[i].g - c.g;
                db = sorted_pal_[i].b - c.b;
                da = sorted_pal_[i].a - a;
                // stop criteria based on properties of used sorting
                if ((dr+db+dg+da) * (dr+db+dg+da) / 4 > dist)
                {
                    break;
                }
                newdist = dr*dr + dg*dg + db*db + da*da;
                if (newdist < dist)
                {
                    ind = i;
                    dist = newdist;
                }
            }
            //put found index in hash map
            color_hashmap_[val] = ind;
        }
        else
        {
            ind = it->second;
        }

        return pal_remap_[ind];
    }

    void create_palette(std::vector<rgba> & palette)
    {
        sorted_pal_.clear();
        if (has_holes_)
        {
            max_colors_--;
            sorted_pal_.push_back(rgba(0,0,0,0));
        }
        assign_node_colors();

        sorted_pal_.reserve(colors_);
        create_palette_rek(sorted_pal_, root_.get());

        // sort palette for binary searching in quantization
        boost::sort(sorted_pal_, rgba::mean_sort_cmp());
        // returned palette is rearanged, so that colors with a<255 are at the begining
        pal_remap_.resize(sorted_pal_.size());
        palette.clear();
        palette.reserve(sorted_pal_.size());
        for (unsigned i=0; i<sorted_pal_.size(); ++i)
        {
            if (sorted_pal_[i].a<255)
            {
                pal_remap_[i] = static_cast<unsigned>(palette.size());
                palette.push_back(sorted_pal_[i]);
            }
        }
        for (unsigned i=0; i<sorted_pal_.size(); ++i)
        {
            if (sorted_pal_[i].a==255)
            {
                pal_remap_[i] = static_cast<unsigned>(palette.size());
                palette.push_back(sorted_pal_[i]);
            }
        }
    }

private:

    void print_tree(node *r, int d=0, int id=0) const
    {
        for (int i=0; i<d; i++)
        {
            printf("\t");
        }
        if (r->count>0)
        {
            printf("%d: (+%d/%d/%.5f) (%d %d %d %d)\n",
                   id, (int)r->count, (int)r->pixel_count, r->reduce_cost,
                   (int)round(gamma(r->reds / r->count, gamma_)),
                   (int)round(gamma(r->greens / r->count, gamma_)),
                   (int)round(gamma(r->blues / r->count, gamma_)),
                   (int)(r->alphas / r->count));
        }
        else
        {
            printf("%d: (%d/%d/%.5f) (%d %d %d %d)\n", id,
                   (int)r->count, (int)r->pixel_count, r->reduce_cost,
                   (int)round(gamma(r->reds / r->pixel_count, gamma_)),
                   (int)round(gamma(r->greens / r->pixel_count, gamma_)),
                   (int)round(gamma(r->blues / r->pixel_count, gamma_)),
                   (int)(r->alphas / r->pixel_count));
        }
        for (unsigned idx=0; idx < 16; ++idx)
        {
            if (r->children_[idx] != 0)
            {
                print_tree(r->children_[idx], d+1, idx);
            }
        }
    }

    // traverse tree and search for nodes with count!=0, that represent single color.
    // clip extreme alfa values
    void create_palette_rek(std::vector<rgba> & palette, node * itr) const
    {
        if (itr->count >= 3)
        {
            unsigned count = itr->count;
            byte a = byte(itr->alphas/float(count));
            if (a > InsertPolicy::MAX_ALPHA) a = 255;
            if (a < InsertPolicy::MIN_ALPHA) a = 0;
            palette.push_back(rgba((byte)round(gamma(itr->reds   / count, gamma_)),
                                   (byte)round(gamma(itr->greens / count, gamma_)),
                                   (byte)round(gamma(itr->blues  / count, gamma_)), a));
        }
        for (unsigned idx=0; idx < 16; ++idx)
        {
            if (itr->children_[idx] != 0)
            {
                create_palette_rek(palette, itr->children_[idx]);
            }
        }
    }

    // assign value to r, representing some penalty for assigning one
    // color to all pixels in this subtree
    void compute_cost(node *r)
    {
        //initial small value, so that all nodes have >0 cost
        r->reduce_cost = r->pixel_count/1000.0;
        if (r->children_count==0)
        {
            return;
        }
        // mean color of all pixels in subtree
        double mean_r = r->reds   / r->pixel_count;
        double mean_g = r->greens / r->pixel_count;
        double mean_b = r->blues  / r->pixel_count;
        double mean_a = r->alphas / r->pixel_count;
        for (unsigned idx=0; idx < 16; ++idx)
        {
            if (r->children_[idx] != 0)
            {
                double dr,dg,db,da;
                compute_cost(r->children_[idx]);
                // include childrens penalty
                r->reduce_cost += r->children_[idx]->reduce_cost;
                // difference between mean value and subtree mean value
                dr = r->children_[idx]->reds   / r->children_[idx]->pixel_count - mean_r;
                dg = r->children_[idx]->greens / r->children_[idx]->pixel_count - mean_g;
                db = r->children_[idx]->blues  / r->children_[idx]->pixel_count - mean_b;
                da = r->children_[idx]->alphas / r->children_[idx]->pixel_count - mean_a;
                // penalty_x = d_x^2 * pixel_count * mean_alfa/255, where x=r,g,b,a
                // mean_alpha/255 because more opaque color = more noticable differences
                r->reduce_cost += (dr*dr + dg*dg + db*db + da*da) * r->children_[idx]->alphas / 255;
            }
        }
    }

    // starting from root_, unfold nodes with biggest penalty
    // until all available colors are assigned to processed nodes
    void assign_node_colors()
    {
        compute_cost(root_.get());

        int tries = 0;

        // at the begining, single color assigned to root_
        colors_ = 1;
        root_->count = root_->pixel_count;

        std::set<node*,node_rev_cmp> colored_leaves_heap;
        colored_leaves_heap.insert(root_.get());
        while((!colored_leaves_heap.empty() && (colors_ < max_colors_) && (tries < 16)))
        {
            // select worst node to remove it from palette and replace with children
            node * cur_node = *colored_leaves_heap.begin();
            colored_leaves_heap.erase(colored_leaves_heap.begin());
            if (((cur_node->children_count + colors_ - 1) > max_colors_))
            {
                tries++;
                continue; // try few times, maybe next will have less children
            }
            tries = 0;
            // ignore leaves and also nodes with small mean error and not excessive number of pixels
            if (cur_node->pixel_count > 0 &&
                (cur_node->children_count > 0) &&
                (((cur_node->reduce_cost / cur_node->pixel_count + 1) * std::log(double(cur_node->pixel_count))) > 15)
                )
            {
                colors_--;
                cur_node->count = 0;
                for (unsigned idx=0; idx < 16; ++idx)
                {
                    if (cur_node->children_[idx] != 0)
                    {
                        node *n = cur_node->children_[idx];
                        n->count = n->pixel_count;
                        colored_leaves_heap.insert(n);
                        colors_++;
                    }
                }
            }
        }
    }
};
} // namespace mapnik

#endif // MAPNIK_HEXTREE_HPP
