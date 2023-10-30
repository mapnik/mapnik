#ifndef MAPNIK_ADAPTIVE_SMOOTH_HPP
#define MAPNIK_ADAPTIVE_SMOOTH_HPP

#include <mapnik/util/math.hpp>

#include <mapnik/warning.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/util/variant.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_conv_smooth_poly1.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

struct vcgen_smooth_calucate_adaptive
{
    using result_type = std::pair<agg::point_d, agg::point_d>;
    using vertex_type = agg::vertex_dist;

    static result_type apply(vertex_type const& v0,
                             vertex_type const& v1,
                             vertex_type const& v2,
                             vertex_type const& v3,
                             double smooth_value)
    {
        double k1 = v0.dist / (v0.dist + v1.dist);
        double k2 = v1.dist / (v1.dist + v2.dist);

        double xm1 = v0.x + (v2.x - v0.x) * k1;
        double ym1 = v0.y + (v2.y - v0.y) * k1;
        double xm2 = v1.x + (v3.x - v1.x) * k2;
        double ym2 = v1.y + (v3.y - v1.y) * k2;

        double s1 = 0;
        double s2 = 0;

        if (v1.dist > 0.0)
        {
            if (v0.dist > 0.0)
            {
                double dot1 = (v0.x - v1.x) * (v2.x - v1.x) + (v0.y - v1.y) * (v2.y - v1.y);
                double a1 = std::acos(dot1 / (v0.dist * v1.dist));
                if (a1 >= util::pi * 0.5)
                {
                    s1 = (a1 - util::pi * 0.5) / (util::pi * 0.5);
                }
            }

            if (v2.dist > 0.0)
            {
                double dot2 = (v1.x - v2.x) * (v3.x - v2.x) + (v1.y - v2.y) * (v3.y - v2.y);
                double a2 = std::acos(dot2 / (v1.dist * v2.dist));
                if (a2 >= util::pi * 0.5)
                {
                    s2 = (a2 - util::pi * 0.5) / (util::pi * 0.5);
                }
            }
        }

        return {{v1.x + s1 * smooth_value * (v2.x - xm1), v1.y + s1 * smooth_value * (v2.y - ym1)},
                {v2.x + s2 * smooth_value * (v1.x - xm2), v2.y + s2 * smooth_value * (v1.y - ym2)}};
    }
};

using vcgen_smooth_adaptive = agg::vcgen_smooth<vcgen_smooth_calucate_adaptive>;

template<class VertexSource>
using conv_smooth_adaptive = agg::conv_smooth_curve<VertexSource, vcgen_smooth_adaptive>;

template<typename Geometry>
class smooth_converter
{
    Geometry geom_;

    using basic_impl_type = typename agg::conv_smooth_poly1_curve<Geometry>;
    using adaptive_impl_type = conv_smooth_adaptive<Geometry>;
    using impl_type = util::variant<basic_impl_type, adaptive_impl_type>;
    impl_type impl_;

    impl_type init_impl(smooth_algorithm_enum algo, Geometry& geom) const
    {
        switch (algo)
        {
            case smooth_algorithm_enum::SMOOTH_ALGORITHM_ADAPTIVE:
                return adaptive_impl_type(geom);
            case smooth_algorithm_enum::SMOOTH_ALGORITHM_BASIC:
            default:
                break;
        }
        return basic_impl_type(geom);
    }

  public:
    smooth_converter(Geometry& geom)
        : geom_(geom)
        , impl_(std::move(init_impl(smooth_algorithm_enum::SMOOTH_ALGORITHM_BASIC, geom)))
    {}

    void algorithm(smooth_algorithm_enum algo) { impl_ = init_impl(algo, geom_); }

    void smooth_value(double v)
    {
        return util::apply_visitor([=](auto& impl) { impl.smooth_value(v); }, impl_);
    }

    void rewind(unsigned path_id)
    {
        return util::apply_visitor([=](auto& impl) { return impl.rewind(path_id); }, impl_);
    }

    unsigned vertex(double* x, double* y)
    {
        return util::apply_visitor([=](auto& impl) { return impl.vertex(x, y); }, impl_);
    }

    unsigned type() const
    {
        return util::apply_visitor([](auto const& impl) { return impl.type(); }, impl_);
    }
};

} // namespace mapnik

#endif // MAPNIK_ADAPTIVE_SMOOTH_HPP
