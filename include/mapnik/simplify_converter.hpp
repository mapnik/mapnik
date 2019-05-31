#ifndef MAPNIK_SIMPLIFY_CONVERTER_HPP
#define MAPNIK_SIMPLIFY_CONVERTER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/simplify.hpp>
#include <mapnik/noncopyable.hpp>

// stl
#include <limits>
#include <set>
#include <vector>
#include <deque>
#include <cmath>
#include <stdexcept>

// boost
#include <boost/optional.hpp>

namespace mapnik
{

struct weighted_vertex : private mapnik::noncopyable
{
    vertex2d coord;
    double weight;
    weighted_vertex *prev;
    weighted_vertex *next;

    weighted_vertex(vertex2d coord_) :
        coord(coord_),
        weight(std::numeric_limits<double>::infinity()),
        prev(NULL),
        next(NULL) {}

    double nominalWeight()
    {
        if (prev == NULL || next == NULL || coord.cmd != SEG_LINETO) {
            return std::numeric_limits<double>::infinity();
        }
        vertex2d const& A = prev->coord;
        vertex2d const& B = next->coord;
        vertex2d const& C = coord;
        return std::abs((double)((A.x - C.x) * (B.y - A.y) - (A.x - B.x) * (C.y - A.y))) / 2.0;
    }

    struct ascending_sort
    {
        bool operator() (const weighted_vertex *a, const weighted_vertex *b) const
        {
            return b->weight > a->weight;
        }
    };
};

struct sleeve
{
    vertex2d v[5];

    sleeve(vertex2d const& v0, vertex2d const& v1, double offset)
    {
        double a = std::atan2((v1.y - v0.y), (v1.x - v0.x));
        double dx = offset * std::cos(a);
        double dy = offset * std::sin(a);
        v[0].x = v0.x + dy;
        v[0].y = v0.y - dx;
        v[1].x = v0.x - dy;
        v[1].y = v0.y + dx;
        v[2].x = v1.x - dy;
        v[2].y = v1.y + dx;
        v[3].x = v1.x + dy;
        v[3].y = v1.y - dx;
        v[4].x = v0.x + dy;
        v[4].y = v0.y - dx;
    }

    bool inside(vertex2d const& q)
    {
        bool inside=false;

        for (unsigned i=0;i<4;++i)
        {
            if ((((v[i+1].y <= q.y) && (q.y < v[i].y)) ||
                 ((v[i].y <= q.y) && (q.y < v[i+1].y))) &&
                (q.x < (v[i].x - v[i+1].x) * (q.y - v[i+1].y)/ (v[i].y - v[i+1].y) + v[i+1].x))
                inside=!inside;
        }
        return inside;
    }
    void print()
    {
        std::cerr << "LINESTRING("
                  << v[0].x << " " << -v[0].y << ","
                  << v[1].x << " " << -v[1].y << ","
                  << v[2].x << " " << -v[2].y << ","
                  << v[3].x << " " << -v[3].y << ","
                  << v[0].x << " " << -v[0].y << ")" << std::endl;

    }
};

template <typename Geometry>
struct MAPNIK_DECL simplify_converter
{
public:
    simplify_converter(Geometry& geom)
        : geom_(geom),
        tolerance_(0.0),
        status_(initial),
        algorithm_(radial_distance),
        pos_(0)
    {}

    enum status
    {
        initial,
        process,
        closing,
        end,
        cache
    };

    simplify_algorithm_e get_simplify_algorithm()
    {
        return algorithm_;
    }

    void set_simplify_algorithm(simplify_algorithm_e value)
    {
        if (algorithm_ != value)
        {
            algorithm_ = value;
            reset();
        }
    }

    double get_simplify_tolerance()
    {
        return tolerance_;
    }

    void set_simplify_tolerance(double value)
    {
        if (tolerance_ != value) {
            tolerance_ = value;
            reset();
        }
    }

    void reset()
    {
        geom_.rewind(0);
        vertices_.clear();
        status_ = initial;
        pos_ = 0;
    }

    void rewind(unsigned int) const
    {
        pos_ = 0;
    }

    unsigned vertex(double* x, double* y)
    {
        if (tolerance_ == 0.0)
            return geom_.vertex(x, y);

        if (status_ == initial)
            init_vertices();

        return output_vertex(x, y);
    }

private:
    unsigned output_vertex(double* x, double* y)
    {
        switch (algorithm_)
        {
        case visvalingam_whyatt:
            return output_vertex_cached(x, y);
        case radial_distance:
            return output_vertex_distance(x, y);
        case zhao_saalfeld:
            return output_vertex_sleeve(x, y);
        default:
            throw std::runtime_error("simplification algorithm not yet implemented");
        }

        return SEG_END;
    }

    unsigned output_vertex_cached(double* x, double* y) {
        if (pos_ >= vertices_.size())
            return SEG_END;

        previous_vertex_ = vertices_[pos_];
        *x = previous_vertex_.x;
        *y = previous_vertex_.y;
        pos_++;
        return previous_vertex_.cmd;
    }

    unsigned output_vertex_distance(double* x, double* y) {
        if (status_ == closing) {
            status_ = end;
            return SEG_CLOSE;
        }

        vertex2d last(vertex2d::no_init);
        vertex2d vtx(vertex2d::no_init);
        while ((vtx.cmd = geom_.vertex(&vtx.x, &vtx.y)) != SEG_END)
        {
            if (vtx.cmd == SEG_LINETO) {
                if (distance_to_previous(vtx) > tolerance_) {
                    // Only output a vertex if it's far enough away from the previous
                    break;
                } else {
                    last = vtx;
                    // continue
                }
            } else if (vtx.cmd == SEG_CLOSE) {
                if (last.cmd == vertex2d::no_init) {
                    // The previous vertex was already output in the previous call.
                    // We can now safely output SEG_CLOSE.
                    status_ = end;
                } else {
                    // We eliminated the previous point because it was too close, but
                    // we have to output it now anyway, since this is the end of the
                    // vertex stream. Make sure that we output SEG_CLOSE in the next call.
                    vtx = last;
                    status_ = closing;
                }
                break;
            } else if (vtx.cmd == SEG_MOVETO) {
                break;
            } else {
                throw std::runtime_error("Unknown vertex command");
            }
        }

        previous_vertex_ = vtx;
        *x = vtx.x;
        *y = vtx.y;
        return vtx.cmd;
    }

    template <typename Iterator>
    bool fit_sleeve(Iterator itr,Iterator end, vertex2d const& v)
    {
        sleeve s(*itr,v,tolerance_);
        ++itr; // skip first vertex
        for (; itr!=end; ++itr)
        {
            if (!s.inside(*itr))
            {
                return false;
            }
        }
        return true;
    }

    unsigned output_vertex_sleeve(double* x, double* y)
    {
        vertex2d vtx(vertex2d::no_init);
        std::size_t min_size = 1;
        while ((vtx.cmd = geom_.vertex(&vtx.x, &vtx.y)) != SEG_END)
        {
            //if ((std::fabs(vtx.x - previous_vertex_.x) < 0.5) &&
            //    (std::fabs(vtx.y - previous_vertex_.y) < 0.5))
            //    continue;

            if (status_ == cache &&
                vertices_.size() >= min_size)
                status_ = process;

            previous_vertex_ = vtx;

            if (vtx.cmd == SEG_MOVETO)
            {
                if (sleeve_cont_.size() > 1)
                {
                    vertices_.push_back(sleeve_cont_.back());
                    sleeve_cont_.clear();
                }
                vertices_.push_back(vtx);
                sleeve_cont_.push_back(vtx);
                if (status_ == process) break;
            }
            else if (vtx.cmd == SEG_LINETO)
            {
                if (sleeve_cont_.size() > 1 && !fit_sleeve(sleeve_cont_.begin(), sleeve_cont_.end(), vtx))
                {
                    vertex2d last = vtx;
                    vtx = sleeve_cont_.back();
                    sleeve_cont_.clear();
                    sleeve_cont_.push_back(vtx);
                    sleeve_cont_.push_back(last);
                    vertices_.push_back(vtx);
                    if (status_ == process) break;
                }
                else
                {
                    sleeve_cont_.push_back(vtx);
                }
            }
            else if (vtx.cmd == SEG_CLOSE)
            {
                if (sleeve_cont_.size() > 1)
                {
                    vertices_.push_back(sleeve_cont_.back());
                    sleeve_cont_.clear();
                }
                vertices_.push_back(vtx);
                if (status_ == process) break;
            }
        }

        if (status_ == cache)
        {
            if (vertices_.size() < min_size)
                return SEG_END;
            status_ = process;
        }

        if (vtx.cmd == SEG_END)
        {
            if (sleeve_cont_.size() > 1)
            {
                vertices_.push_back(sleeve_cont_.back());
            }
            sleeve_cont_.clear();
            vertices_.push_back(vtx);
        }

        if (vertices_.size() > 0)
        {
            vertex2d v = vertices_.front();
            vertices_.pop_front();
            *x = v.x;
            *y = v.y;
            return v.cmd;
        }
        return SEG_END;
    }

    double distance_to_previous(vertex2d const& vtx) {
        double dx = previous_vertex_.x - vtx.x;
        double dy = previous_vertex_.y - vtx.y;
        return dx * dx + dy * dy;
    }

    status init_vertices()
    {
        if (status_ != initial) // already initialized
            return status_;

        reset();

        switch (algorithm_) {
            case visvalingam_whyatt:
                return init_vertices_visvalingam_whyatt();
            case radial_distance:
                // Use
                vertices_.push_back(vertex2d(vertex2d::no_init));
                return status_ = process;
            case zhao_saalfeld:
                return status_ = cache;
            default:
                throw std::runtime_error("simplification algorithm not yet implemented");
        }
    }

    status init_vertices_visvalingam_whyatt()
    {
        typedef std::set<weighted_vertex *, weighted_vertex::ascending_sort> VertexSet;
        typedef std::vector<weighted_vertex *> VertexList;

        std::vector<weighted_vertex *> v_list;
        vertex2d vtx(vertex2d::no_init);
        while ((vtx.cmd = geom_.vertex(&vtx.x, &vtx.y)) != SEG_END)
        {
            v_list.push_back(new weighted_vertex(vtx));
        }

        if (v_list.empty()) {
            return status_ = process;
        }

        // Connect the vertices in a linked list and insert them into the set.
        VertexSet v;
        for (VertexList::iterator i = v_list.begin(); i != v_list.end(); ++i)
        {
            (*i)->prev = i == v_list.begin() ? NULL : *(i - 1);
            (*i)->next = i + 1 == v_list.end() ? NULL : *(i + 1);
            (*i)->weight = (*i)->nominalWeight();
            v.insert(*i);
        }

        // Use Visvalingam-Whyatt algorithm to calculate each point's weight.
        while (v.size() > 0)
        {
            VertexSet::iterator lowest = v.begin();
            weighted_vertex *removed = *lowest;
            if (removed->weight >= tolerance_) {
                break;
            }

            v.erase(lowest);

            // Connect adjacent vertices with each other
            if (removed->prev) removed->prev->next = removed->next;
            if (removed->next) removed->next->prev = removed->prev;
            // Adjust weight and reinsert prev/next to move them to their correct position.
            if (removed->prev) {
                v.erase(removed->prev);
                removed->prev->weight = std::max(removed->weight, removed->prev->nominalWeight());
                v.insert(removed->prev);
            }
            if (removed->next) {
                v.erase(removed->next);
                removed->next->weight = std::max(removed->weight, removed->next->nominalWeight());
                v.insert(removed->next);
            }
        }

        v.clear();

        // Traverse the remaining list and insert them into the vertex cache.
        for (VertexList::iterator i = v_list.begin(); i != v_list.end(); ++i)
        {
            if ((*i)->weight >= tolerance_)
            {
                vertices_.push_back((*i)->coord);
            }
            delete *i;
        }

        // Initialization finished.
        return status_ = process;
    }

    Geometry&                       geom_;
    double                          tolerance_;
    status                          status_;
    simplify_algorithm_e            algorithm_;
    std::deque<vertex2d>            vertices_;
    std::deque<vertex2d>            sleeve_cont_;
    vertex2d                        previous_vertex_;
    mutable size_t                  pos_;
};


}

#endif // MAPNIK_SIMPLIFY_CONVERTER_HPP
