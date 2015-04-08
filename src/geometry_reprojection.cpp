/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/geometry_reprojection.hpp>
#include <mapnik/geometry.hpp>

namespace mapnik {

namespace geometry {

namespace detail {

geometry_empty reproject_internal(geometry_empty const&, proj_transform const&, unsigned int &)
{
    return geometry_empty();
}

point reproject_internal(point const & p, proj_transform const& proj_trans, unsigned int & n_err)
{
    point new_p(p);
    if (!proj_trans.forward(new_p))
    {
        ++n_err;
    }
    return new_p;
}

line_string reproject_internal(line_string const & ls, proj_transform const& proj_trans, unsigned int & n_err)
{
    line_string new_ls(ls);
    unsigned int err = proj_trans.forward(new_ls);
    if (err > 0)
    {
        n_err += err;
    }
    return new_ls;
}

polygon reproject_internal(polygon const & poly, proj_transform const& proj_trans, unsigned int & n_err)
{
    polygon new_poly;
    linear_ring new_ext(poly.exterior_ring);
    unsigned int err = proj_trans.forward(new_ext);
    // If the exterior ring doesn't transform don't bother with the holes.
    if (err > 0 || new_ext.empty())
    {
        n_err += err;
    }
    else
    {
        new_poly.set_exterior_ring(std::move(new_ext));
        new_poly.interior_rings.reserve(poly.interior_rings.size());

        for (auto const& lr : poly.interior_rings)
        {
            linear_ring new_lr(lr);
            err = proj_trans.forward(new_lr);
            if (err > 0 || new_lr.empty())
            {
                n_err += err;
                // If there is an error in interior ring drop
                // it from polygon.
                continue;
            }
            new_poly.add_hole(std::move(new_lr));
        }
    }
    return new_poly;
}

multi_point reproject_internal(multi_point const & mp, proj_transform const& proj_trans, unsigned int & n_err)
{
    multi_point new_mp;
    if (proj_trans.is_known())
    {
        // If the projection is known we do them all at once because it is faster
        // since we know that no point will fail reprojection
        new_mp.assign(mp.begin(), mp.end());
        proj_trans.forward(new_mp);
    }
    else
    {
        new_mp.reserve(mp.size());
        for (auto const& p : mp)
        {
            point new_p(p);
            if (!proj_trans.forward(new_p))
            {
                ++n_err;
            }
            else
            {
                new_mp.emplace_back(std::move(new_p));
            }
        }
    }
    return new_mp;
}

multi_line_string reproject_internal(multi_line_string const & mls, proj_transform const& proj_trans, unsigned int & n_err)
{
    multi_line_string new_mls;
    new_mls.reserve(mls.size());
    for (auto const& ls : mls)
    {
        line_string new_ls = reproject_internal(ls, proj_trans, n_err);
        if (!new_ls.empty())
        {
            new_mls.emplace_back(std::move(new_ls));
        }
    }
    return new_mls;
}

multi_polygon reproject_internal(multi_polygon const & mpoly, proj_transform const& proj_trans, unsigned int & n_err)
{
    multi_polygon new_mpoly;
    new_mpoly.reserve(mpoly.size());
    for (auto const& poly : mpoly)
    {
        polygon new_poly = reproject_internal(poly, proj_trans, n_err);
        if (!new_poly.exterior_ring.empty())
        {
            new_mpoly.emplace_back(std::move(new_poly));
        }
    }
    return new_mpoly;
}

geometry_collection reproject_internal(geometry_collection const & c, proj_transform const& proj_trans, unsigned int & n_err)
{
    geometry_collection new_c;
    new_c.reserve(c.size());
    for (auto const& g : c)
    {
        geometry new_g(std::move(reproject_copy(g, proj_trans, n_err)));
        if (!new_g.is<geometry_empty>())
        {
            new_c.emplace_back(std::move(new_g));
        }
    }
    return new_c;
}

struct geom_reproj_copy_visitor {

    geom_reproj_copy_visitor(proj_transform const & proj_trans, unsigned int & n_err)
        : proj_trans_(proj_trans),
          n_err_(n_err) {}

    geometry operator() (geometry_empty const&)
    {
        return geometry_empty();
    }

    geometry operator() (point const& p)
    {
        geometry geom; // default empty
        unsigned int intial_err = n_err_;
        point new_p = reproject_internal(p, proj_trans_, n_err_);
        if (n_err_ > intial_err) return geom;
        geom = std::move(new_p);
        return geom;
    }

    geometry operator() (line_string const& ls)
    {
        geometry geom; // default empty
        int intial_err = n_err_;
        line_string new_ls = reproject_internal(ls, proj_trans_, n_err_);
        if (n_err_ > intial_err || new_ls.empty()) return geom;
        geom = std::move(new_ls);
        return geom;
    }

    geometry operator() (polygon const& poly)
    {
        geometry geom; // default empty
        polygon new_poly = reproject_internal(poly, proj_trans_, n_err_);
        if (new_poly.exterior_ring.empty()) return geom;
        geom = std::move(new_poly);
        return geom;
    }

    geometry operator() (multi_point const& mp)
    {
        geometry geom; // default empty
        multi_point new_mp = reproject_internal(mp, proj_trans_, n_err_);
        if (new_mp.empty()) return geom;
        geom = std::move(new_mp);
        return geom;
    }

    geometry operator() (multi_line_string const& mls)
    {
        geometry geom; // default empty
        multi_line_string new_mls = reproject_internal(mls, proj_trans_, n_err_);
        if (new_mls.empty()) return geom;
        geom = std::move(new_mls);
        return geom;
    }

    geometry operator() (multi_polygon const& mpoly)
    {
        geometry geom; // default empty
        multi_polygon new_mpoly = reproject_internal(mpoly, proj_trans_, n_err_);
        if (new_mpoly.empty()) return geom;
        geom = std::move(new_mpoly);
        return geom;
    }

    geometry operator() (geometry_collection const& c)
    {
        geometry geom; // default empty
        geometry_collection new_c = reproject_internal(c, proj_trans_, n_err_);
        if (new_c.empty()) return geom;
        geom = std::move(new_c);
        return geom;
    }

private:
    proj_transform const& proj_trans_;
    unsigned int & n_err_;

};

} // end detail ns

geometry reproject_copy(geometry const& geom, proj_transform const& proj_trans, unsigned int & n_err)
{
    detail::geom_reproj_copy_visitor visit(proj_trans, n_err);
    return mapnik::util::apply_visitor(visit, geom);
}

template <typename T>
T reproject_copy(T const& geom, proj_transform const& proj_trans, unsigned int & n_err)
{
    return detail::reproject_internal(geom, proj_trans, n_err);
}

template MAPNIK_DECL geometry_empty reproject_copy(geometry_empty const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL point reproject_copy(point const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL line_string reproject_copy(line_string const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL polygon reproject_copy(polygon const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL multi_point reproject_copy(multi_point const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL multi_line_string reproject_copy(multi_line_string const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL multi_polygon reproject_copy(multi_polygon const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL geometry_collection reproject_copy(geometry_collection const& geom, proj_transform const& proj_trans, unsigned int & n_err);

template <typename T>
T reproject_copy(T const& geom, projection const& source, projection const& dest, unsigned int & n_err)
{
    proj_transform proj_trans(source, dest);
    return reproject_copy(geom, proj_trans, n_err);
}

template MAPNIK_DECL geometry reproject_copy(geometry const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL geometry_empty reproject_copy(geometry_empty const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL point reproject_copy(point const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL line_string reproject_copy(line_string const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL polygon reproject_copy(polygon const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL multi_point reproject_copy(multi_point const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL multi_line_string reproject_copy(multi_line_string const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL multi_polygon reproject_copy(multi_polygon const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL geometry_collection reproject_copy(geometry_collection const& geom, projection const& source, projection const& dest, unsigned int & n_err);

namespace detail {

struct geom_reproj_visitor {

    geom_reproj_visitor(proj_transform const & proj_trans)
        : proj_trans_(proj_trans) {}

    bool operator() (geometry & geom)
    {
        return mapnik::util::apply_visitor((*this), geom);
    }

    bool operator() (geometry_empty &) { return true; }

    bool operator() (point & p)
    {
        if (!proj_trans_.forward(p))
        {
            return false;
        }
        return true;
    }

    bool operator() (line_string & ls)
    {
        if (proj_trans_.forward(ls) > 0)
        {
            return false;
        }
        return true;
    }

    bool operator() (polygon & poly)
    {
        if (proj_trans_.forward(poly.exterior_ring) > 0)
        {
            return false;
        }

        for (auto & lr : poly.interior_rings)
        {
            if (proj_trans_.forward(lr) > 0)
            {
                return false;
            }
        }
        return true;
    }

    bool operator() (multi_point & mp)
    {
        return (*this) (static_cast<line_string &>(mp));
    }

    bool operator() (multi_line_string & mls)
    {
        for (auto & ls : mls)
        {
            if (! (*this) (ls))
            {
                return false;
            }
        }
        return true;
    }

    bool operator() (multi_polygon & mpoly)
    {
        for (auto & poly : mpoly)
        {
            if(! (*this)(poly))
            {
                return false;
            }
        }
        return true;
    }

    bool operator() (geometry_collection & c)
    {
        for (auto & g : c)
        {
            if (! (*this)(g) )
            {
                return false;
            }
        }
        return true;
    }

private:
    proj_transform const& proj_trans_;

};

} // end detail ns

template <typename T>
bool reproject(T & geom, proj_transform const& proj_trans)
{
    detail::geom_reproj_visitor visit(proj_trans);
    return visit(geom);
}

template MAPNIK_DECL bool reproject(geometry & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(geometry_empty & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(point & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(line_string & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(polygon & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(multi_point & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(multi_line_string & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(multi_polygon & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(geometry_collection & geom, proj_transform const& proj_trans);

template <typename T>
bool reproject(T & geom, projection const& source, projection const& dest)
{
    proj_transform proj_trans(source, dest);
    detail::geom_reproj_visitor visit(proj_trans);
    return visit(geom);
}

template MAPNIK_DECL bool reproject(geometry & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(geometry_empty & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(point & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(line_string & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(polygon & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(multi_point & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(multi_line_string & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(multi_polygon & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(geometry_collection & geom, projection const& source, projection const& dest);

} // end geometry ns

} // end mapnik ns
