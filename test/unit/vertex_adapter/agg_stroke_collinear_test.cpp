#include "catch.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_conv_stroke.h"
#include "agg_path_storage.h"
MAPNIK_DISABLE_WARNING_POP

namespace {

double dist_to_segment(double px, double py, double ax, double ay, double bx, double by)
{
    double const dx = bx - ax;
    double const dy = by - ay;
    double const len2 = dx * dx + dy * dy;
    double t = len2 > 0 ? ((px - ax) * dx + (py - ay) * dy) / len2 : 0.0;
    t = std::clamp(t, 0.0, 1.0);
    return std::hypot(px - (ax + t * dx), py - (ay + t * dy));
}

} // namespace

TEST_CASE("agg stroke")
{
    SECTION("nearly collinear outer joins stay within the miter limit")
    {
        // Pixel coordinates of a real-world line whose first vertices are almost,
        // but not exactly, collinear. For outer joins between such segments the
        // AGG stroker used the intersection of two nearly parallel offset lines,
        // which round-off placed millions of pixels away from the line. Without
        // a rasterizer clip box the resulting sliver made the scanline
        // rasterizer write outside its cover buffer. Every stroke vertex must
        // stay within the miter limit (width * 4) of the source polyline.
        static double const pts[][2] = {{1079.2551884794566, 546.19585810014632},
                                        {1080.2626843172829, 546.19585826387618},
                                        {1081.2701801551095, 546.19585842849517},
                                        {1082.2776759929359, 546.19585859311417},
                                        {1083.2851718308118, 546.19585875773305},
                                        {1084.2798876793904, 546.42937835762984},
                                        {1085.013007490742, 547.11738955853082},
                                        {1085.4941786584147, 548.01359219954543},
                                        {1085.6474654753165, 549.00293382225345},
                                        {1085.3435339457576, 549.95826903451984},
                                        {1085.1604691282562, 550.41008930733926}};
        int const n = sizeof(pts) / sizeof(pts[0]);
        double const width = 0.44901064765406917;
        // bevel and round joins both take the nearly-collinear shortcut
        agg::line_join_e const joins[] = {agg::bevel_join, agg::round_join};

        for (auto const join : joins)
        {
            agg::path_storage path;
            path.move_to(pts[0][0], pts[0][1]);
            for (int i = 1; i < n; ++i)
                path.line_to(pts[i][0], pts[i][1]);

            agg::conv_stroke<agg::path_storage> stroke(path);
            stroke.width(width);
            stroke.line_cap(agg::round_cap);
            stroke.inner_join(agg::inner_bevel);
            stroke.line_join(join);

            double max_dist = 0;
            double x, y;
            unsigned cmd;
            stroke.rewind(0);
            while (!agg::is_stop(cmd = stroke.vertex(&x, &y)))
            {
                if (!agg::is_vertex(cmd))
                    continue;
                double d = std::numeric_limits<double>::max();
                for (int i = 1; i < n; ++i)
                    d = std::min(d, dist_to_segment(x, y, pts[i - 1][0], pts[i - 1][1], pts[i][0], pts[i][1]));
                max_dist = std::max(max_dist, d);
            }
            INFO("join " << static_cast<int>(join) << ": stroke vertex " << max_dist << " px from the line");
            CHECK(max_dist <= width * stroke.miter_limit() + 1e-9);
        }
    }
}
