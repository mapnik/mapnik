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

#ifndef MAPNIK_HEXTREE_HPP
#define MAPNIK_HEXTREE_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/palette.hpp>
#include <mapnik/util/noncopyable.hpp>

// stl
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>
#include <cmath>

namespace mapnik {

struct RGBAPolicy
{
    static unsigned const MAX_LEVELS = 6;
    static unsigned const MIN_ALPHA = 5;
    static unsigned const MAX_ALPHA = 250;
    inline static unsigned index_from_level(unsigned level, rgba const& c)
    {
        unsigned shift = 7 - level;
        return (((c.a >> shift) & 1) << 3) | (((c.r >> shift) & 1) << 2) | (((c.g >> shift) & 1) << 1) |
               ((c.b >> shift) & 1);
    }
};

template<typename T, typename InsertPolicy = RGBAPolicy>
class hextree : private util::noncopyable
{
    struct node
    {
        node()
            : reds(0.0),
              greens(0.0),
              blues(0.0),
              alphas(0.0),
              count(0),
              pixel_count(0),
              reduce_cost(0.0),
              children_count(0)
        {
            std::fill(children_, children_ + 16, nullptr);
        }

        bool is_leaf() const { return (children_count == 0); }
        node* children_[16];
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
        std::uint8_t children_count;
    };
    static_assert(std::is_trivially_destructible<node>::value, "pooled nodes must be trivially destructible");

    static constexpr std::size_t node_block_size = 256;
    struct node_block
    {
        alignas(node) unsigned char data[sizeof(node) * node_block_size];
    };

    class node_pool
    {
      public:
        node* create()
        {
            if (next_ == node_block_size)
            {
                blocks_.push_back(std::unique_ptr<node_block>(new node_block));
                next_ = 0;
            }
            unsigned char* storage = blocks_.back()->data + sizeof(node) * next_++;
            return ::new (static_cast<void*>(storage)) node();
        }

      private:
        std::vector<std::unique_ptr<node_block>> blocks_;
        std::size_t next_ = node_block_size;
    };

    // highest reduce_cost first
    struct node_rev_cmp
    {
        bool operator()(node const* lhs, node const* rhs) const
        {
            if (lhs->reduce_cost != rhs->reduce_cost)
            {
                return (lhs->reduce_cost > rhs->reduce_cost);
            }
            return (lhs > rhs);
        }
    };

    static std::uint64_t mean_sort_key(rgba const& color)
    {
        std::uint64_t const mean = color.a + color.r + color.g + color.b;
        return (mean << 32) | (std::uint64_t(color.a) << 24) | (std::uint64_t(color.r) << 16) |
               (std::uint64_t(color.g) << 8) | color.b;
    }

    unsigned max_colors_;
    unsigned colors_;
    // flag indicating existance of invisible pixels (a < InsertPolicy::MIN_ALPHA)
    bool has_holes_;
    node_pool node_pool_;
    node* const root_;
    // leaves (colors at the tree's resolution) when the palette was created
    std::size_t leaf_colors_ = 0;
    // working palette for quantization, sorted on mean(r,g,b,a) for easier searching NN
    std::vector<rgba> sorted_pal_;
    std::vector<std::uint64_t> sorted_keys_;
    // index remaping of sorted_pal_ indexes to indexes of returned image palette
    std::vector<unsigned> pal_remap_;
    // the colors quantized so far and their indexes in the returned palette (color 0, which is not cached,
    // marks an empty entry): open addressing, sized for the tree's leaves and grown if more colors come
    struct qentry
    {
        std::uint32_t color;
        std::uint32_t index;
    };
    mutable std::vector<qentry> qcache_;
    mutable std::size_t qcount_ = 0;
    mutable unsigned qbits_ = 0;
    // gamma correction to prioritize dark colors (>1.0)
    double gamma_;
    // look up table for gamma correction
    double gammaLUT_[256];
    // transparency handling
    enum transparency_mode_t { NO_TRANSPARENCY = 0, BINARY_TRANSPARENCY = 1, FULL_TRANSPARENCY = 2 };
    unsigned trans_mode_;

    static double gamma(double b, double g) { return 255 * std::pow(b / 255, g); }

  public:
    explicit hextree(unsigned max_colors = 256, double g = 2.0)
        : max_colors_(max_colors),
          colors_(0),
          has_holes_(false),
          root_(node_pool_.create()),
          trans_mode_(FULL_TRANSPARENCY)
    {
        setGamma(g);
    }

    ~hextree() {}

    void setMaxColors(unsigned max_colors) { max_colors_ = max_colors; }

    void setGamma(double g)
    {
        gamma_ = g;
        if (g == 2.0)
        {
            // the default: computed once (256 pow calls per tree otherwise)
            static std::array<double, 256> const default_lut = [] {
                std::array<double, 256> lut;
                for (unsigned i = 0; i < 256; i++)
                {
                    lut[i] = gamma(i, 1 / 2.0);
                }
                return lut;
            }();
            std::copy(default_lut.begin(), default_lut.end(), gammaLUT_);
            return;
        }
        for (unsigned i = 0; i < 256; i++)
        {
            gammaLUT_[i] = gamma(i, 1 / gamma_);
        }
    }

    void setTransMode(unsigned t) { trans_mode_ = t; }

    transparency_mode_t getTransMode() const { return trans_mode_; }

    // process alpha value based on trans_mode_
    std::uint8_t preprocessAlpha(std::uint8_t a) const
    {
        switch (trans_mode_)
        {
            case NO_TRANSPARENCY:
                return 255;
            case BINARY_TRANSPARENCY:
                return a < 127 ? 0 : 255;
            default:
                return a;
        }
    }

    // adds count pixels of a color (callers insert each color once, with all its pixels)
    void insert(T const& data, unsigned count = 1)
    {
        if (count == 0)
        {
            return;
        }
        std::uint8_t a = preprocessAlpha(data.a);
        if (a < InsertPolicy::MIN_ALPHA)
        {
            has_holes_ = true;
            return;
        }
        double const reds = count * gammaLUT_[data.r];
        double const greens = count * gammaLUT_[data.g];
        double const blues = count * gammaLUT_[data.b];
        unsigned const alphas = count * a;
        node* cur_node = root_;
        for (unsigned level = 0;; ++level)
        {
            bool const new_color = cur_node->pixel_count == 0;
            cur_node->pixel_count += count;
            cur_node->reds += reds;
            cur_node->greens += greens;
            cur_node->blues += blues;
            cur_node->alphas += alphas;
            if (level == InsertPolicy::MAX_LEVELS)
            {
                if (new_color)
                {
                    ++colors_;
                }
                break;
            }
            unsigned idx = InsertPolicy::index_from_level(level, data);
            if (cur_node->children_[idx] == 0)
            {
                cur_node->children_count++;
                cur_node->children_[idx] = node_pool_.create();
            }
            cur_node = cur_node->children_[idx];
        }
    }

    // quantize() without the cache, for callers that quantize each color once
    int quantize_once(unsigned val) const { return quantize_uncached(val, nullptr); }

    // return color index in returned earlier palette
    int quantize(unsigned val) const
    {
        // the colors quantized before are cached, and most pixels repeat one: that lookup is
        // what gets inlined into the callers' loops
        if (val != 0 && colors_ > 1)
        {
            qentry& entry = qcache_find(val);
            if (entry.color == val)
            {
                return static_cast<int>(entry.index);
            }
            return quantize_uncached(val, &entry);
        }
        return quantize_uncached(val, nullptr);
    }

  private:
    // quantize() for a color not cached yet: cached, when given, is its entry to fill
    __attribute__((noinline)) int quantize_uncached(unsigned val, qentry* cached) const
    {
        std::uint8_t a = preprocessAlpha(U2ALPHA(val));
        unsigned ind = 0;
        if (a < InsertPolicy::MIN_ALPHA || colors_ == 0)
        {
            return 0;
        }
        if (colors_ == 1)
        {
            return pal_remap_[has_holes_ ? 1 : 0];
        }
        {
            rgba c(val);
            int dr, dg, db, da;
            int dist, newdist;

            // find closest match based on mean of r,g,b,a
            // std::lower_bound without branches (they mispredict half the time here)
            std::uint64_t const key = mean_sort_key(c);
            std::uint64_t const* base = sorted_keys_.data();
            for (std::size_t n = sorted_keys_.size(); n > 1;)
            {
                std::size_t const half = n / 2;
                base = (base[half] < key) ? base + half : base;
                n -= half;
            }
            ind = static_cast<unsigned>(base - sorted_keys_.data()) + (*base < key);
            if (ind == sorted_pal_.size())
                ind--;
            dr = sorted_pal_[ind].r - c.r;
            dg = sorted_pal_[ind].g - c.g;
            db = sorted_pal_[ind].b - c.b;
            da = sorted_pal_[ind].a - a;
            dist = dr * dr + dg * dg + db * db + da * da;
            int poz = ind;

            // search neighbour positions in both directions for better match
            for (int i = poz - 1; i >= 0; i--)
            {
                dr = sorted_pal_[i].r - c.r;
                dg = sorted_pal_[i].g - c.g;
                db = sorted_pal_[i].b - c.b;
                da = sorted_pal_[i].a - a;
                // stop criteria based on properties of used sorting
                if (((dr + db + dg + da) * (dr + db + dg + da) / 4 > dist))
                {
                    break;
                }
                newdist = dr * dr + dg * dg + db * db + da * da;
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
                if ((dr + db + dg + da) * (dr + db + dg + da) / 4 > dist)
                {
                    break;
                }
                newdist = dr * dr + dg * dg + db * db + da * da;
                if (newdist < dist)
                {
                    ind = i;
                    dist = newdist;
                }
            }
            if (cached)
            {
                cached->color = val;
                cached->index = pal_remap_[ind];
                qcache_added();
            }
        }

        return pal_remap_[ind];
    }

  public:
    void create_palette(std::vector<rgba>& palette)
    {
        sorted_pal_.clear();
        if (has_holes_)
        {
            max_colors_--;
            sorted_pal_.push_back(rgba(0, 0, 0, 0));
        }
        leaf_colors_ = colors_;
        assign_node_colors();
        // sized on the first quantize(): callers that quantize each color once don't need it
        qcache_.clear();

        sorted_pal_.reserve(colors_);
        create_palette_rek(sorted_pal_, root_);

        // sort palette for binary searching in quantization
        std::sort(sorted_pal_.begin(), sorted_pal_.end(), rgba::mean_sort_cmp());
        sorted_keys_.clear();
        sorted_keys_.reserve(sorted_pal_.size());
        for (rgba const& color : sorted_pal_)
        {
            sorted_keys_.push_back(mean_sort_key(color));
        }
        // returned palette is rearanged, so that colors with a<255 are at the begining
        pal_remap_.resize(sorted_pal_.size());
        palette.clear();
        palette.reserve(sorted_pal_.size());
        for (unsigned i = 0; i < sorted_pal_.size(); ++i)
        {
            if (sorted_pal_[i].a < 255)
            {
                pal_remap_[i] = static_cast<unsigned>(palette.size());
                palette.push_back(sorted_pal_[i]);
            }
        }
        for (unsigned i = 0; i < sorted_pal_.size(); ++i)
        {
            if (sorted_pal_[i].a == 255)
            {
                pal_remap_[i] = static_cast<unsigned>(palette.size());
                palette.push_back(sorted_pal_[i]);
            }
        }
    }

  private:

    void qcache_reset(std::size_t colors) const
    {
        qbits_ = 4;
        while ((std::size_t(1) << qbits_) < 2 * colors)
        {
            ++qbits_;
        }
        qcache_.assign(std::size_t(1) << qbits_, qentry{0, 0});
        qcount_ = 0;
    }

    // the entry of val, or the empty one where it goes
    qentry& qcache_find(unsigned val) const
    {
        if (qcache_.empty())
        {
            qcache_reset(leaf_colors_);
        }
        std::size_t const mask = qcache_.size() - 1;
        std::size_t i = static_cast<std::size_t>((std::uint64_t(val) * 0x9E3779B97F4A7C15ULL) >> (64 - qbits_));
        while (qcache_[i].color != val && qcache_[i].color != 0)
        {
            i = (i + 1) & mask;
        }
        return qcache_[i];
    }

    void qcache_added() const
    {
        if (2 * ++qcount_ > qcache_.size())
        {
            std::vector<qentry> old;
            old.swap(qcache_);
            std::size_t const count = qcount_;
            qcache_reset(count);
            for (qentry const& e : old)
            {
                if (e.color != 0)
                {
                    qcache_find(e.color) = e;
                }
            }
            qcount_ = count;
        }
    }

    void print_tree(node* r, int d = 0, int id = 0) const
    {
        for (int i = 0; i < d; i++)
        {
            printf("\t");
        }
        if (r->count > 0)
        {
            printf("%d: (+%d/%d/%.5f) (%d %d %d %d)\n",
                   id,
                   static_cast<int>(r->count),
                   static_cast<int>(r->pixel_count),
                   r->reduce_cost,
                   static_cast<int>(round(gamma(r->reds / r->count, gamma_))),
                   static_cast<int>(round(gamma(r->greens / r->count, gamma_))),
                   static_cast<int>(round(gamma(r->blues / r->count, gamma_))),
                   static_cast<int>((r->alphas / r->count)));
        }
        else
        {
            printf("%d: (%d/%d/%.5f) (%d %d %d %d)\n",
                   id,
                   static_cast<int>(r->count),
                   static_cast<int>(r->pixel_count),
                   r->reduce_cost,
                   static_cast<int>(round(gamma(r->reds / r->pixel_count, gamma_))),
                   static_cast<int>(round(gamma(r->greens / r->pixel_count, gamma_))),
                   static_cast<int>(round(gamma(r->blues / r->pixel_count, gamma_))),
                   static_cast<int>((r->alphas / r->pixel_count)));
        }
        for (unsigned idx = 0; idx < 16; ++idx)
        {
            if (r->children_[idx] != 0)
            {
                print_tree(r->children_[idx], d + 1, idx);
            }
        }
    }

    // traverse tree and search for nodes with count!=0, that represent single color.
    // clip extreme alfa values
    void create_palette_rek(std::vector<rgba>& palette, node* itr) const
    {
        if (itr->count != 0)
        {
            // assign_node_colors colors a node's children only when it splits the node, which uncolors it:
            // nothing below a colored node is colored
            unsigned count = itr->count;
            std::uint8_t a = std::uint8_t(itr->alphas / float(count));
            if (a > InsertPolicy::MAX_ALPHA)
                a = 255;
            if (a < InsertPolicy::MIN_ALPHA)
                a = 0;
            palette.push_back(rgba(static_cast<std::uint8_t>(round(gamma(itr->reds / count, gamma_))),
                                   static_cast<std::uint8_t>(round(gamma(itr->greens / count, gamma_))),
                                   static_cast<std::uint8_t>(round(gamma(itr->blues / count, gamma_))),
                                   a));
            return;
        }
        for (unsigned idx = 0; idx < 16; ++idx)
        {
            if (itr->children_[idx] != 0)
            {
                create_palette_rek(palette, itr->children_[idx]);
            }
        }
    }

    // assign value to r, representing some penalty for assigning one
    // color to all pixels in this subtree
    void compute_cost(node* r)
    {
        // initial small value, so that all nodes have >0 cost
        r->reduce_cost = r->pixel_count / 1000.0;
        if (r->children_count == 0)
        {
            return;
        }
        // mean color of all pixels in subtree
        double mean_r = r->reds / r->pixel_count;
        double mean_g = r->greens / r->pixel_count;
        double mean_b = r->blues / r->pixel_count;
        double mean_a = r->alphas / r->pixel_count;
        for (unsigned idx = 0; idx < 16; ++idx)
        {
            if (r->children_[idx] != 0)
            {
                double dr, dg, db, da;
                compute_cost(r->children_[idx]);
                // include childrens penalty
                r->reduce_cost += r->children_[idx]->reduce_cost;
                // difference between mean value and subtree mean value
                dr = r->children_[idx]->reds / r->children_[idx]->pixel_count - mean_r;
                dg = r->children_[idx]->greens / r->children_[idx]->pixel_count - mean_g;
                db = r->children_[idx]->blues / r->children_[idx]->pixel_count - mean_b;
                da = r->children_[idx]->alphas / r->children_[idx]->pixel_count - mean_a;
                // penalty_x = d_x^2 * pixel_count * mean_alfa/255, where x=r,g,b,a
                // mean_alpha/255 because more opaque color = more noticable differences
                r->reduce_cost += (dr * dr + dg * dg + db * db + da * da) * r->children_[idx]->alphas / 255;
            }
        }
    }

    // starting from root_, unfold nodes with biggest penalty
    // until all available colors are assigned to processed nodes
    void assign_node_colors()
    {
        compute_cost(root_);

        int tries = 0;

        // at the begining, single color assigned to root_
        colors_ = 1;
        root_->count = root_->pixel_count;

        // a max-heap on the reverse of node_rev_cmp: its top is the node a set ordered by node_rev_cmp would
        // have first (a strict total order, and no node enters twice)
        auto const heap_cmp = [](node const* lhs, node const* rhs) { return node_rev_cmp()(rhs, lhs); };
        std::vector<node*> colored_leaves_heap;
        colored_leaves_heap.push_back(root_);
        while ((!colored_leaves_heap.empty() && (colors_ < max_colors_) && (tries < 16)))
        {
            // select worst node to remove it from palette and replace with children
            std::pop_heap(colored_leaves_heap.begin(), colored_leaves_heap.end(), heap_cmp);
            node* cur_node = colored_leaves_heap.back();
            colored_leaves_heap.pop_back();
            if (((cur_node->children_count + colors_ - 1) > max_colors_))
            {
                tries++;
                continue; // try few times, maybe next will have less children
            }
            tries = 0;
            // ignore leaves and also nodes with small mean error and not excessive number of pixels
            if (cur_node->pixel_count > 0 && (cur_node->children_count > 0) &&
                (((cur_node->reduce_cost / cur_node->pixel_count + 1) * std::log(double(cur_node->pixel_count))) > 15))
            {
                colors_--;
                cur_node->count = 0;
                for (unsigned idx = 0; idx < 16; ++idx)
                {
                    if (cur_node->children_[idx] != 0)
                    {
                        node* n = cur_node->children_[idx];
                        n->count = n->pixel_count;
                        colored_leaves_heap.push_back(n);
                        std::push_heap(colored_leaves_heap.begin(), colored_leaves_heap.end(), heap_cmp);
                        colors_++;
                    }
                }
            }
        }
    }
};
} // namespace mapnik

#endif // MAPNIK_HEXTREE_HPP
