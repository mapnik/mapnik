#ifndef MAPNIK_SIMPLIFY_CONVERTER_HPP
#define MAPNIK_SIMPLIFY_CONVERTER_HPP

#include <mapnik/debug.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/simplify.hpp>

// STL
#include <limits>
#include <set>

// Boost
#include <boost/optional.hpp>

namespace mapnik
{

struct weighted_vertex : private boost::noncopyable {
    vertex2d coord;
    double weight;
    weighted_vertex *prev;
    weighted_vertex *next;

    weighted_vertex(vertex2d coord_) :
        coord(coord_),
        weight(std::numeric_limits<double>::infinity()),
        prev(NULL),
        next(NULL) { }

    double nominalWeight() {
        if (prev == NULL || next == NULL || coord.cmd != SEG_LINETO) {
            return std::numeric_limits<double>::infinity();
        }
        vertex2d& A = prev->coord;
        vertex2d& B = next->coord;
        vertex2d& C = coord;
        return std::abs((double)((A.x - C.x) * (B.y - A.y) - (A.x - B.x) * (C.y - A.y))) / 2.0;
    }

    struct ascending_sort {
        bool operator() (const weighted_vertex *a, const weighted_vertex *b) {
            return b->weight > a->weight;
        }
    };
};

template <typename Geometry>
struct MAPNIK_DECL simplify_converter
{
public:
    simplify_converter(Geometry& geom)
        : geom_(geom)
        , tolerance_(0.0)
        , status_(initial)
        , algorithm_(radial_distance)
    {
    }

    enum status
    {
        initial,
        process,
        closing,
        end
    };

    simplify_algorithm_e get_simplify_algorithm()
    {
        return algorithm_;
    }

    void set_simplify_algorithm(simplify_algorithm_e value) {
        if (algorithm_ != value) {
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

    void rewind(unsigned int)
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
        switch (algorithm_) {
            case visvalingam_whyatt:
                return output_vertex_cached(x, y);
            case radial_distance:
                return output_vertex_distance(x, y);
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
            default:
                throw std::runtime_error("simplification algorithm not yet implemented");
        }
    }

    status init_vertices_visvalingam_whyatt() {
        typedef std::set<weighted_vertex *, weighted_vertex::ascending_sort> VertexSet;
        typedef std::vector<weighted_vertex *> VertexList;

        std::vector<weighted_vertex *> v_list;
        vertex2d vtx(vertex2d::no_init);
        while ((vtx.cmd = geom_.vertex(&vtx.x, &vtx.y)) != SEG_END)
        {
            v_list.push_back(new weighted_vertex(vtx));
        }

        if (!v_list.size()) {
            return status_ = process;
        }

        // Connect the vertices in a linked list and insert them into the set.
        VertexSet v;
        for (VertexList::iterator i = v_list.begin(); i != v_list.end(); i++)
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
        for (VertexList::iterator i = v_list.begin(); i != v_list.end(); i++)
        {
            if ((*i)->weight >= tolerance_) {
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
    size_t                          pos_;
    std::vector<vertex2d>           vertices_;
    vertex2d                        previous_vertex_;
};

}

#endif // MAPNIK_SIMPLIFY_CONVERTER_HPP
