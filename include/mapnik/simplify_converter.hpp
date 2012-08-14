#ifndef MAPNIK_SIMPLIFY_CONVERTER_HPP
#define MAPNIK_SIMPLIFY_CONVERTER_HPP

#include <mapnik/debug.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/vertex.hpp>

namespace mapnik
{

template <typename Geometry>
struct MAPNIK_DECL simplify_converter
{
public:
    simplify_converter(Geometry& geom)
        : geom_(geom)
        , tolerance_(0.0)
    {
    }

    double simplify_tolerance(double tolerance)
    {
        return tolerance_;
    }

    void set_simplify_tolerance(double tolerance)
    {
        tolerance_ = tolerance;
    }

    void rewind(unsigned int path_id)
    {
        geom_.rewind(path_id);
    }

    unsigned vertex(double* x, double* y)
    {
        unsigned cmd = geom_.vertex(x, y);
        return cmd;
    }

private:
    Geometry&     geom_;
    double        tolerance_;
};

}

#endif // MAPNIK_SIMPLIFY_CONVERTER_HPP
