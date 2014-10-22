/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/global.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/utils.hpp>

#ifdef MAPNIK_USE_PROJ4
// proj4
#include <proj_api.h>
#endif

// stl
#include <vector>
#include <stdexcept>

namespace mapnik {

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

bool proj_transform::forward (double & x, double & y , double & z) const
{
    return forward(&x, &y, &z, 1);
}

bool proj_transform::forward (double * x, double * y , double * z, int point_count) const
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
            x[i] *= DEG_TO_RAD;
            y[i] *= DEG_TO_RAD;
        }
    }

    if (pj_transform( source_.proj_, dest_.proj_, point_count,
                      0, x,y,z) != 0)
    {
        return false;
    }

    if (is_dest_longlat_)
    {
        int i;
        for(i=0; i<point_count; i++) {
            x[i] *= RAD_TO_DEG;
            y[i] *= RAD_TO_DEG;
        }
    }
#endif
    return true;
}

bool proj_transform::backward (double * x, double * y , double * z, int point_count) const
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
        int i;
        for(i=0; i<point_count; i++) {
            x[i] *= DEG_TO_RAD;
            y[i] *= DEG_TO_RAD;
        }
    }

    if (pj_transform( dest_.proj_, source_.proj_, point_count,
                      0, x,y,z) != 0)
    {
        return false;
    }

    if (is_source_longlat_)
    {
        int i;
        for(i=0; i<point_count; i++) {
            x[i] *= RAD_TO_DEG;
            y[i] *= RAD_TO_DEG;
        }
    }
#endif
    return true;
}

bool proj_transform::backward (double & x, double & y , double & z) const
{
    return backward(&x, &y, &z, 1);
}


bool proj_transform::forward (box2d<double> & box) const
{
    if (is_source_equal_dest_)
        return true;

    double minx = box.minx();
    double miny = box.miny();
    double maxx = box.maxx();
    double maxy = box.maxy();
    double z = 0.0;
    if (!forward(minx,miny,z))
        return false;
    if (!forward(maxx,maxy,z))
        return false;
    box.init(minx,miny,maxx,maxy);
    return true;
}

bool proj_transform::backward (box2d<double> & box) const
{
    if (is_source_equal_dest_)
        return true;

    double minx = box.minx();
    double miny = box.miny();
    double maxx = box.maxx();
    double maxy = box.maxy();
    double z = 0.0;
    if (!backward(minx,miny,z))
        return false;
    if (!backward(maxx,maxy,z))
        return false;
    box.init(minx,miny,maxx,maxy);
    return true;
}

void envelope_points(std::vector< coord<double,2> > & coords, box2d<double>& env, int points)
{
    double width = env.width();
    double height = env.height();

    int steps;

    if (points <= 4) {
        steps = 0;
    } else {
        steps = static_cast<int>(std::ceil((points - 4) / 4.0));
    }

    steps += 1;
    double xstep = width / steps;
    double ystep = height / steps;

    for (int i=0; i<=steps; i++) {
        coords.push_back(coord<double,2>(env.minx() + i * xstep, env.miny()));
        coords.push_back(coord<double,2>(env.minx() + i * xstep, env.maxy()));

    }
    for (int i=1; i<steps; i++) {
        coords.push_back(coord<double,2>(env.minx(), env.miny() + i * ystep));
        coords.push_back(coord<double,2>(env.maxx(), env.miny() + i * ystep));
    }
}

box2d<double> calculate_bbox(std::vector<coord<double,2> > & points) {
    std::vector<coord<double,2> >::iterator it = points.begin();
    std::vector<coord<double,2> >::iterator it_end = points.end();

    box2d<double> env(*it, *(++it));
    for (; it!=it_end; ++it) {
        env.expand_to_include(*it);
    }
    return env;
}


/* More robust, but expensive, bbox transform
 * in the face of proj4 out of bounds conditions.
 * Can result in 20 -> 10 r/s performance hit.
 * Alternative is to provide proper clipping box
 * in the target srs by setting map 'maximum-extent'
 */
bool proj_transform::backward(box2d<double>& env, int points) const
{
    if (is_source_equal_dest_)
        return true;

    if (wgs84_to_merc_ || merc_to_wgs84_)
    {
        return backward(env);
    }

    std::vector<coord<double,2> > coords;
    envelope_points(coords, env, points);

    double z;
    for (std::vector<coord<double,2> >::iterator it = coords.begin(); it!=coords.end(); ++it) {
        z = 0;
        if (!backward(it->x, it->y, z)) {
            return false;
        }
    }

    box2d<double> result = calculate_bbox(coords);

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

    std::vector<coord<double,2> > coords;
    envelope_points(coords, env, points);

    double z;
    for (std::vector<coord<double,2> >::iterator it = coords.begin(); it!=coords.end(); ++it) {
        z = 0;
        if (!forward(it->x, it->y, z)) {
            return false;
        }
    }

    box2d<double> result = calculate_bbox(coords);

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
