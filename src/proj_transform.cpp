/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

// mapnik
#include <mapnik/geometry/boost_adapters.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/geometry/multi_point.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/util/is_clockwise.hpp>

// boost
#include <boost/geometry/algorithms/envelope.hpp>

#ifdef MAPNIK_USE_PROJ4
// proj4
#include <proj_api.h>
#endif

// stl
#include <vector>
#include <stdexcept>

namespace mapnik {

namespace { // (local)

// Returns points in clockwise order. This allows us to do anti-meridian checks.
template <typename T>
auto envelope_points(box2d<T> const& env, std::size_t num_points)
    -> geometry::multi_point<T>
{
    auto width = env.width();
    auto height = env.height();

    geometry::multi_point<T> coords;
    coords.reserve(num_points);

    // top side: left >>> right
    // gets extra point if (num_points % 4 >= 1)
    for (std::size_t i = 0, n = (num_points + 3) / 4; i < n; ++i)
    {
        auto x = env.minx() + (i * width) / n;
        coords.emplace_back(x, env.maxy());
    }

    // right side: top >>> bottom
    // gets extra point if (num_points % 4 >= 3)
    for (std::size_t i = 0, n = (num_points + 1) / 4; i < n; ++i)
    {
        auto y = env.maxy() - (i * height) / n;
        coords.emplace_back(env.maxx(), y);
    }

    // bottom side: right >>> left
    // gets extra point if (num_points % 4 >= 2)
    for (std::size_t i = 0, n = (num_points + 2) / 4; i < n; ++i)
    {
        auto x = env.maxx() - (i * width) / n;
        coords.emplace_back(x, env.miny());
    }

    // left side: bottom >>> top
    // never gets extra point
    for (std::size_t i = 0, n = (num_points + 0) / 4; i < n; ++i)
    {
        auto y = env.miny() + (i * height) / n;
        coords.emplace_back(env.minx(), y);
    }

    return coords;
}

} // namespace mapnik::(local)

proj_transform::proj_transform(projection const& source,
                               projection const& dest)
    : source_(source),
      dest_(dest),
      is_source_longlat_(false),
      is_dest_longlat_(false),
      is_source_equal_dest_(false),
      wgs84_to_merc_(false),
      merc_to_wgs84_(false)
{
    is_source_equal_dest_ = (source_ == dest_);
    if (!is_source_equal_dest_)
    {
        is_source_longlat_ = source_.is_geographic();
        is_dest_longlat_ = dest_.is_geographic();
        boost::optional<well_known_srs_e> src_k = source.well_known();
        boost::optional<well_known_srs_e> dest_k = dest.well_known();
        bool known_trans = false;
        if (src_k && dest_k)
        {
            if (*src_k == WGS_84 && *dest_k == G_MERC)
            {
                wgs84_to_merc_ = true;
                known_trans = true;
            }
            else if (*src_k == G_MERC && *dest_k == WGS_84)
            {
                merc_to_wgs84_ = true;
                known_trans = true;
            }
        }
        if (!known_trans)
        {
#ifdef MAPNIK_USE_PROJ4
            source_.init_proj4();
            dest_.init_proj4();
#else
            throw std::runtime_error(std::string("Cannot initialize proj_transform for given projections without proj4 support (-DMAPNIK_USE_PROJ4): '") + source_.params() + "'->'" + dest_.params() + "'");
#endif
        }
    }
}

bool proj_transform::equal() const
{
    return is_source_equal_dest_;
}

bool proj_transform::is_known() const
{
    return merc_to_wgs84_ || wgs84_to_merc_;
}

bool proj_transform::forward (double & x, double & y , double & z) const
{
    return forward(&x, &y, &z, 1);
}

bool proj_transform::forward (geometry::point<double> & p) const
{
    double z = 0;
    return forward(&(p.x), &(p.y), &z, 1);
}

unsigned int proj_transform::forward (std::vector<geometry::point<double>> & ls) const
{
    std::size_t size = ls.size();
    if (size == 0) return 0;

    if (is_source_equal_dest_)
        return 0;

    if (wgs84_to_merc_)
    {
        lonlat2merc(ls);
        return 0;
    }
    else if (merc_to_wgs84_)
    {
        merc2lonlat(ls);
        return 0;
    }

    geometry::point<double> * ptr = ls.data();
    double * x = reinterpret_cast<double*>(ptr);
    double * y = x + 1;
    double * z = nullptr;
    if(!forward(x, y, z, size, 2))
    {
        return size;
    }
    return 0;
}

bool proj_transform::forward (double * x, double * y , double * z, int point_count, int offset) const
{

    if (is_source_equal_dest_)
        return true;

    if (wgs84_to_merc_)
    {
        return lonlat2merc(x,y,point_count);
    }
    else if (merc_to_wgs84_)
    {
        return merc2lonlat(x,y,point_count);
    }

#ifdef MAPNIK_USE_PROJ4
    if (is_source_longlat_)
    {
        int i;
        for(i=0; i<point_count; i++) {
            x[i*offset] *= DEG_TO_RAD;
            y[i*offset] *= DEG_TO_RAD;
        }
    }

    if (pj_transform( source_.proj_, dest_.proj_, point_count,
                      offset, x,y,z) != 0)
    {
        return false;
    }

    for(int j=0; j<point_count; j++) {
        if (x[j] == HUGE_VAL || y[j] == HUGE_VAL)
        {
            return false;
        }
    }

    if (is_dest_longlat_)
    {
        int i;
        for(i=0; i<point_count; i++) {
            x[i*offset] *= RAD_TO_DEG;
            y[i*offset] *= RAD_TO_DEG;
        }
    }
#endif
    return true;
}

bool proj_transform::backward (double * x, double * y , double * z, int point_count, int offset) const
{
    if (is_source_equal_dest_)
        return true;

    if (wgs84_to_merc_)
    {
        return merc2lonlat(x,y,point_count);
    }
    else if (merc_to_wgs84_)
    {
        return lonlat2merc(x,y,point_count);
    }

#ifdef MAPNIK_USE_PROJ4
    if (is_dest_longlat_)
    {
        for (int i = 0; i < point_count; ++i)
        {
            x[i * offset] *= DEG_TO_RAD;
            y[i * offset] *= DEG_TO_RAD;
        }
    }

    if (pj_transform(dest_.proj_, source_.proj_, point_count,
                     offset, x, y, z) != 0)
    {
        return false;
    }

    for (int j = 0; j < point_count; ++j)
    {
        if (x[j] == HUGE_VAL || y[j] == HUGE_VAL)
        {
            return false;
        }
    }

    if (is_source_longlat_)
    {
        for (int i = 0; i < point_count; ++i)
        {
            x[i * offset] *= RAD_TO_DEG;
            y[i * offset] *= RAD_TO_DEG;
        }
    }
#endif
    return true;
}

bool proj_transform::backward (double & x, double & y , double & z) const
{
    return backward(&x, &y, &z, 1);
}

bool proj_transform::backward (geometry::point<double> & p) const
{
    double z = 0;
    return backward(&(p.x), &(p.y), &z, 1);
}

unsigned int proj_transform::backward (std::vector<geometry::point<double>> & ls) const
{
    std::size_t size = ls.size();
    if (size == 0) return 0;

    if (is_source_equal_dest_)
        return 0;

    if (wgs84_to_merc_)
    {
        merc2lonlat(ls);
        return 0;
    }
    else if (merc_to_wgs84_)
    {
        lonlat2merc(ls);
        return 0;
    }

    geometry::point<double> * ptr = ls.data();
    double * x = reinterpret_cast<double*>(ptr);
    double * y = x + 1;
    double * z = nullptr;
    if (!backward(x, y, z, size, 2))
    {
        return size;
    }
    return 0;
}

bool proj_transform::forward (box2d<double> & box) const
{
    if (is_source_equal_dest_)
        return true;

    double llx = box.minx();
    double ulx = box.minx();
    double lly = box.miny();
    double lry = box.miny();
    double lrx = box.maxx();
    double urx = box.maxx();
    double uly = box.maxy();
    double ury = box.maxy();
    double z = 0.0;
    if (!forward(llx,lly,z))
        return false;
    if (!forward(lrx,lry,z))
        return false;
    if (!forward(ulx,uly,z))
        return false;
    if (!forward(urx,ury,z))
        return false;

    double minx = std::min(ulx, llx);
    double miny = std::min(lly, lry);
    double maxx = std::max(urx, lrx);
    double maxy = std::max(ury, uly);
    box.init(minx,
             miny,
             maxx,
             maxy);
    return true;
}

bool proj_transform::backward (box2d<double> & box) const
{
    if (is_source_equal_dest_)
        return true;

    double x[4], y[4];
    x[0] = box.minx(); // llx 0
    y[0] = box.miny(); // lly 1
    x[1] = box.maxx(); // lrx 2
    y[1] = box.miny(); // lry 3
    x[2] = box.minx(); // ulx 4
    y[2] = box.maxy(); // uly 5
    x[3] = box.maxx(); // urx 6
    y[3] = box.maxy(); // ury 7

    if (!backward(x, y, nullptr, 4, 1))
        return false;

    double minx = std::min(x[0], x[2]);
    double miny = std::min(y[0], y[1]);
    double maxx = std::max(x[1], x[3]);
    double maxy = std::max(y[2], y[3]);
    box.init(minx, miny, maxx, maxy);
    return true;
}

// More robust, but expensive, bbox transform
// in the face of proj4 out of bounds conditions.
// Can result in 20 -> 10 r/s performance hit.
// Alternative is to provide proper clipping box
// in the target srs by setting map 'maximum-extent'

bool proj_transform::backward(box2d<double>& env, int points) const
{
    if (is_source_equal_dest_)
        return true;

    if (wgs84_to_merc_ || merc_to_wgs84_)
    {
        return backward(env);
    }

    auto coords = envelope_points(env, points);  // this is always clockwise

    for (auto & p : coords)
    {
        double z = 0;
        if (!backward(p.x, p.y, z))
            return false;
    }

    box2d<double> result;
    boost::geometry::envelope(coords, result);

    if (is_source_longlat_ && !util::is_clockwise(coords))
    {
        // we've gone to a geographic CS, and our clockwise envelope has
        // changed into an anticlockwise one. This means we've crossed the antimeridian, and
        // need to expand the X direction to +/-180 to include all the data. Once we can deal
        // with multiple bboxes in queries we can improve.
        double miny = result.miny();
        result.expand_to_include(-180.0, miny);
        result.expand_to_include(180.0, miny);
    }

    env.re_center(result.center().x, result.center().y);
    env.height(result.height());
    env.width(result.width());
    return true;
}

bool proj_transform::forward(box2d<double>& env, int points) const
{
    if (is_source_equal_dest_)
        return true;

    if (wgs84_to_merc_ || merc_to_wgs84_)
    {
        return forward(env);
    }

    auto coords = envelope_points(env, points);  // this is always clockwise

    for (auto & p : coords)
    {
        double z = 0;
        if (!forward(p.x, p.y, z))
            return false;
    }

    box2d<double> result;
    boost::geometry::envelope(coords, result);

    if (is_dest_longlat_ && !util::is_clockwise(coords))
    {
        // we've gone to a geographic CS, and our clockwise envelope has
        // changed into an anticlockwise one. This means we've crossed the antimeridian, and
        // need to expand the X direction to +/-180 to include all the data. Once we can deal
        // with multiple bboxes in queries we can improve.
        double miny = result.miny();
        result.expand_to_include(-180.0, miny);
        result.expand_to_include(180.0, miny);
    }

    env.re_center(result.center().x, result.center().y);
    env.height(result.height());
    env.width(result.width());

    return true;
}

mapnik::projection const& proj_transform::source() const
{
    return source_;
}
mapnik::projection const& proj_transform::dest() const
{
    return dest_;
}

}
