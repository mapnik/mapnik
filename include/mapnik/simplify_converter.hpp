#ifndef MAPNIK_SIMPLIFY_CONVERTER_HPP
#define MAPNIK_SIMPLIFY_CONVERTER_HPP

#include <mapnik/debug.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/simplify.hpp>

// STL
#include <limits>

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

    struct descending_sort {
        bool operator() (const weighted_vertex *a, const weighted_vertex *b) {
            return a->weight > b->weight;
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
            default:
                throw std::runtime_error("simplification algorithm not yet implemented");
        }

        return SEG_END;
    }

    unsigned output_vertex_cached(double* x, double* y) {
        if (pos_ >= vertices_.size())
            return SEG_END;

        vertex2d const& vtx = vertices_[pos_];
        *x = vtx.x;
        *y = vtx.y;
        pos_++;
        return vtx.cmd;
    }

    status init_vertices()
    {
        if (status_ != initial) // already initialized
            return status_;

        reset();

        switch (algorithm_) {
            case visvalingam_whyatt:
                return init_vertices_visvalingam_whyatt();
            default:
                throw std::runtime_error("simplification algorithm not yet implemented");
        }
    }

    status init_vertices_visvalingam_whyatt() {
        typedef std::vector<weighted_vertex *> WeightedVertices;

        WeightedVertices v;
        vertex2d vtx(vertex2d::no_init);
        while ((vtx.cmd = geom_.vertex(&vtx.x, &vtx.y)) != SEG_END)
        {
            v.push_back(new weighted_vertex(vtx));
        }

        if (!v.size()) {
            return status_ = process;
        }

        // Connect the vertices in a linked list.
        for (WeightedVertices::iterator end = v.end(), begin = v.begin(), i = v.begin(); i != end; i++)
        {
            (*i)->prev = i == begin ? NULL : *(i - 1);
            (*i)->next = i + 1 == end ? NULL : *(i + 1);
            (*i)->weight = (*i)->nominalWeight();
        }

        weighted_vertex *front = v.front();

        typename weighted_vertex::descending_sort descending;

        std::sort(v.begin(), v.end(), descending);
        // Use Visvalingam-Whyatt algorithm to calculate each point's weight.
        while (v.size() > 0 && v.back()->weight < tolerance_)
        {
            weighted_vertex *removed = v.back();
            v.pop_back();

            // Connect adjacent vertices with each other
            if (removed->prev) removed->prev->next = removed->next;
            if (removed->next) removed->next->prev = removed->prev;
            if (removed->prev) removed->prev->weight = std::max(removed->weight, removed->prev->nominalWeight());
            if (removed->next) removed->next->weight = std::max(removed->weight, removed->next->nominalWeight());

            if (front == removed) {
                front = removed->next;
            }

            delete removed;

            // TODO: find a way so that we can efficiently resort the vector.
            // We only changed remove->prev and removed->next, so we should only
            // have to deal with them. E.g. use binary search to find prev/next
            // using the old value, determine new position using binary search
            // and if different, move it.
            std::sort(v.begin(), v.end(), descending);
        }

        // Traverse the remaining list and insert them into the vertex cache.
        for (weighted_vertex *vtx = front; vtx;) {
            vertices_.push_back(vtx->coord);
            weighted_vertex *removed = vtx;
            vtx = vtx->next;
            delete removed;
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

};

}

#endif // MAPNIK_SIMPLIFY_CONVERTER_HPP
