#ifndef MAPNIK_ROTATION_HPP
#define MAPNIK_ROTATION_HPP

#include <cmath>

namespace mapnik {

struct rotation
{
    rotation()
        : sin(0)
        , cos(1.)
    {}
    rotation(double sin_, double cos_)
        : sin(sin_)
        , cos(cos_)
    {}
    rotation(double angle)
        : sin(std::sin(angle))
        , cos(std::cos(angle))
    {}
    void reset()
    {
        sin = 0.;
        cos = 1.;
    }
    void init(double angle)
    {
        sin = std::sin(angle);
        cos = std::cos(angle);
    }
    double sin;
    double cos;
    rotation operator~() const { return rotation(sin, -cos); }
    rotation operator!() const { return rotation(-sin, cos); }

    double angle() const { return std::atan2(sin, cos); }
};
} // namespace mapnik

#endif // ROTATION_HPP
