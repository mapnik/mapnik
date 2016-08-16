#ifndef MAPNIK_SIMPLIFY_CONVERTER_HPP
#define MAPNIK_SIMPLIFY_CONVERTER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/simplify.hpp>
#include <mapnik/util/noncopyable.hpp>

// stl
#include <limits>
#include <set>
#include <vector>
#include <deque>
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace mapnik
{

struct weighted_vertex : private util::noncopyable
{
    vertex2d coord;
    double weight;
    weighted_vertex *prev;
    weighted_vertex *next;

    weighted_vertex(vertex2d coord_) :
        coord(coord_),
        weight(std::numeric_limits<double>::infinity()),
        prev(nullptr),
        next(nullptr) {}

    double nominalWeight()
    {
        if (prev == nullptr || next == nullptr || coord.cmd != SEG_LINETO)
        {
            return std::numeric_limits<double>::infinity();
        }
        vertex2d const& A = prev->coord;
        vertex2d const& B = next->coord;
        vertex2d const& C = coord;
        return std::abs(static_cast<double>((A.x - C.x) * (B.y - A.y) - (A.x - B.x) * (C.y - A.y))) / 2.0;
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
        bool _inside=false;

        for (unsigned i=0;i<4;++i)
        {
            if ((((v[i+1].y <= q.y) && (q.y < v[i].y)) ||
                 ((v[i].y <= q.y) && (q.y < v[i+1].y))) &&
                (q.x < (v[i].x - v[i+1].x) * (q.y - v[i+1].y)/ (v[i].y - v[i+1].y) + v[i+1].x))
                _inside=!_inside;
        }
        return _inside;
    }
};

template <typename Geometry>
struct simplify_converter
{
public:
    simplify_converter(Geometry & geom)
        : geom_(geom),
        tolerance_(0.0),
        status_(initial),
        algorithm_(radial_distance),
        pos_(0)
    {}

    enum status : std::uint8_t
    {
        initial,
        process,
        closing,
        done,
        cache
    };

    unsigned type() const
    {
        return static_cast<unsigned>(geom_.type());
    }

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
        if (tolerance_ != value)
        {
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
        case douglas_peucker:
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

    unsigned output_vertex_cached(double* x, double* y)
    {
        if (pos_ >= vertices_.size())
            return SEG_END;

        previous_vertex_ = vertices_[pos_];
        if (previous_vertex_.cmd == SEG_CLOSE)
        {
            *x = *y = 0.0; // restore SEG_CLOSE command
        }
        else
        {
            *x = previous_vertex_.x;
            *y = previous_vertex_.y;
        }
        pos_++;
        return previous_vertex_.cmd;
    }

    unsigned output_vertex_distance(double* x, double* y)
    {
        if (status_ == closing)
        {
            *x = *y = 0.0;
            status_ = done;
            return SEG_CLOSE;
        }

        vertex2d last;
        vertex2d vtx(vertex2d::no_init);
        while ((vtx.cmd = geom_.vertex(&vtx.x, &vtx.y)) != SEG_END)
        {
            if (vtx.cmd == SEG_LINETO)
            {
                if (distance_to_previous(vtx) > tolerance_)
                {
                    // Only output a vertex if it's far enough away from the previous
                    break;
                }
                else
                {
                    last = vtx;
                    // continue
                }
            }
            else if (vtx.cmd == SEG_CLOSE)
            {
                if (last.cmd == SEG_END)
                {
                    // The previous vertex was already output in the previous call.
                    // We can now safely output SEG_CLOSE.
                    status_ = done;
                }
                else
                {
                    // We eliminated the previous point because it was too close, but
                    // we have to output it now anyway, since this is the end of the
                    // vertex stream. Make sure that we output SEG_CLOSE in the next call.
                    vtx.x = start_vertex_.x;
                    vtx.y = start_vertex_.y;
                    status_ = closing;
                }
                break;
            }
            else if (vtx.cmd == SEG_MOVETO)
            {
                start_vertex_ = vtx;
                break;
            }
            else
            {
                throw std::runtime_error("Unknown vertex command");
            }
        }

        previous_vertex_ = vtx;
        *x = vtx.x;
        *y = vtx.y;
        return vtx.cmd;
    }

    template <typename Iterator>
    bool fit_sleeve(Iterator itr, Iterator end, vertex2d const& v)
    {
        sleeve s(*itr,v,tolerance_);
        ++itr; // skip first vertex
        for (; itr != end; ++itr)
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

            if (vtx.cmd == SEG_MOVETO)
            {
                if (sleeve_cont_.size() > 1)
                {
                    vertices_.push_back(sleeve_cont_.back());
                    sleeve_cont_.clear();
                }
                vertices_.push_back(vtx);
                sleeve_cont_.push_back(vtx);
                start_vertex_ = vtx;
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
                vtx.x = start_vertex_.x;
                vtx.y = start_vertex_.y;
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
            if (v.cmd == SEG_CLOSE)
            {
                *x = *y = 0.0; // restore SEG_CLOSE command
            }
            else
            {
                *x = v.x;
                *y = v.y;
            }
            return v.cmd;
        }
        return SEG_END;
    }

    double distance_to_previous(vertex2d const& vtx)
    {
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
            case douglas_peucker:
                return init_vertices_RDP();
            default:
                throw std::runtime_error("simplification algorithm not yet implemented");
        }
    }

    status init_vertices_visvalingam_whyatt()
    {
        using VertexSet = std::set<weighted_vertex *, weighted_vertex::ascending_sort>;
        using VertexList = std::vector<weighted_vertex *>;

        std::vector<weighted_vertex *> v_list;
        vertex2d vtx(vertex2d::no_init);
        while ((vtx.cmd = geom_.vertex(&vtx.x, &vtx.y)) != SEG_END)
        {
            if (vtx.cmd == SEG_MOVETO)
            {
                start_vertex_ = vtx;
            }
            else if (vtx.cmd == SEG_CLOSE)
            {
                vtx.x = start_vertex_.x;
                vtx.y = start_vertex_.y;
            }
            v_list.push_back(new weighted_vertex(vtx));
        }

        if (v_list.empty())
        {
            return status_ = process;
        }

        // Connect the vertices in a linked list and insert them into the set.
        VertexSet v;
        for (VertexList::iterator i = v_list.begin(); i != v_list.end(); ++i)
        {
            (*i)->prev = i == v_list.begin() ? nullptr : *(i - 1);
            (*i)->next = i + 1 == v_list.end() ? nullptr : *(i + 1);
            (*i)->weight = (*i)->nominalWeight();
            v.insert(*i);
        }

        // Use Visvalingam-Whyatt algorithm to calculate each point's weight.
        while (v.size() > 0)
        {
            VertexSet::iterator lowest = v.begin();
            weighted_vertex *removed = *lowest;
            if (removed->weight >= tolerance_)
            {
                break;
            }

            v.erase(lowest);

            // Connect adjacent vertices with each other
            if (removed->prev) removed->prev->next = removed->next;
            if (removed->next) removed->next->prev = removed->prev;
            // Adjust weight and reinsert prev/next to move them to their correct position.
            if (removed->prev)
            {
                v.erase(removed->prev);
                removed->prev->weight = std::max(removed->weight, removed->prev->nominalWeight());
                v.insert(removed->prev);
            }
            if (removed->next)
            {
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

    void RDP(std::vector<vertex2d>& vertices, const size_t first, const size_t last)
    {
        // Squared length of a vector
        auto sqlen = [] (vertex2d const& vec) { return vec.x*vec.x + vec.y*vec.y; };
        // Compute square distance of p to a line segment
        auto segment_distance = [&sqlen] (vertex2d const& p, vertex2d const& a, vertex2d const& b, vertex2d const& dir, double dir_sq_len)
        {
            // Special case where segment has same first and last point at which point we are just doing a radius check
            if (dir_sq_len == 0)
            {
                return sqlen(vertex2d(p.x - b.x, p.y - b.y, SEG_END));
            }

            // Project p onto dir by ((p dot dir / dir dot dir) * dir)
            double scale = ((p.x - a.x) * dir.x + (p.y - a.y) * dir.y) / dir_sq_len;
            double projected_x = dir.x * scale;
            double projected_y = dir.y * scale;
            double projected_origin_distance = projected_x * projected_x + projected_y * projected_y;

            // Projected point doesn't lie on the segment
            if (projected_origin_distance > dir_sq_len)
            {
                // Projected point lies past the end of the segment
                if (scale > 0)
                {
                    return sqlen(vertex2d(p.x - b.x, p.y - b.y, SEG_END));
                }// Projected point lies before the beginning of the segment
                else
                {
                    return sqlen(vertex2d(p.x - a.x, p.y - a.y, SEG_END));
                }
            }// Projected point lies on the segment
            else
            {
                return sqlen(vertex2d(p.x - (projected_x + a.x), p.y - (projected_y + a.y), SEG_END));
            }
        };

        // Compute the directional vector along the segment
        vertex2d dir = vertex2d(vertices[last].x - vertices[first].x, vertices[last].y - vertices[first].y, SEG_END);
        double dir_sq_len = sqlen(dir);

        // Find the point with the maximum distance from this line segment
        double max = std::numeric_limits<double>::min();
        size_t keeper = 0;
        for (size_t i = first + 1; i < last; ++i)
        {
            double d = segment_distance(vertices[i], vertices[first], vertices[last], dir, dir_sq_len);
            if (d > max)
            {
                keeper = i;
                max = d;
            }
        }

        // Split at the vertex that is furthest outside of the tolerance
        // NOTE: we work in square distances to avoid sqrt so we sqaure tolerance accordingly
        if (max > tolerance_ * tolerance_)
        {
            // Make sure not to smooth out the biggest outlier (keeper)
            if (keeper - first != 1)
            {
                RDP(vertices, first, keeper);
            }
            if (last - keeper != 1)
            {
                RDP(vertices, keeper, last);
            }
        }// Everyone between the first and the last was close enough to the line
        else
        {
            // Mark each of them as discarded
            for (size_t i = first + 1; i < last; ++i)
            {
                vertices[i].cmd = SEG_END;
            }
        }
    }

    status init_vertices_RDP()
    {
        // Slurp out the original vertices
        std::vector<vertex2d> vertices;
        //vertices.reserve(geom_.size());
        vertex2d vtx(vertex2d::no_init);
        while ((vtx.cmd = geom_.vertex(&vtx.x, &vtx.y)) != SEG_END)
        {
            if (vtx.cmd == SEG_MOVETO)
            {
                start_vertex_ = vtx;
            }
            else if (vtx.cmd == SEG_CLOSE)
            {
                vtx.x = start_vertex_.x;
                vtx.y = start_vertex_.y;
            }
            vertices.push_back(vtx);
        }

        // Run ramer douglas peucker on it
        if (vertices.size() > 2)
        {
            RDP(vertices, 0, vertices.size() - 1);
        }

        // Slurp the points back out that haven't been marked as discarded
        for (vertex2d const& v : vertices)
        {
            if (v.cmd != SEG_END)
            {
                vertices_.emplace_back(v);
            }
        }

        return status_ = process;
    }

    Geometry &                      geom_;
    double                          tolerance_;
    status                          status_;
    simplify_algorithm_e            algorithm_;
    std::deque<vertex2d>            vertices_;
    std::deque<vertex2d>            sleeve_cont_;
    vertex2d                        previous_vertex_;
    vertex2d                        start_vertex_;
    mutable size_t                  pos_;
};


}

#endif // MAPNIK_SIMPLIFY_CONVERTER_HPP
