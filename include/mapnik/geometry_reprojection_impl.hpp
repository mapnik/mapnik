/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

template <typename T>
point<T> reproject_internal(point<T> const& p, proj_transform const& proj_trans, unsigned int & n_err)
{
    point<T> new_p(p);
    if (!proj_trans.forward(new_p))
    {
        ++n_err;
    }
    return new_p;
}

template <typename T>
line_string<T> reproject_internal(line_string<T> const& ls, proj_transform const& proj_trans, unsigned int & n_err)
{
    line_string<T> new_ls(ls);
    unsigned int err = proj_trans.forward(new_ls);
    if (err > 0)
    {
        n_err += err;
    }
    return new_ls;
}

template <typename T>
polygon<T> reproject_internal(polygon<T> const& poly, proj_transform const& proj_trans, unsigned int & n_err)
{
    polygon<T> new_poly;
    linear_ring<T> new_ext(poly.exterior_ring);
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
            linear_ring<T> new_lr(lr);
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

template <typename T>
multi_point<T> reproject_internal(multi_point<T> const & mp, proj_transform const& proj_trans, unsigned int & n_err)
{
    multi_point<T> new_mp;
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
            point<T> new_p(p);
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

template <typename T>
multi_line_string<T> reproject_internal(multi_line_string<T> const & mls, proj_transform const& proj_trans, unsigned int & n_err)
{
    multi_line_string<T> new_mls;
    new_mls.reserve(mls.size());
    for (auto const& ls : mls)
    {
        line_string<T> new_ls = reproject_internal(ls, proj_trans, n_err);
        if (!new_ls.empty())
        {
            new_mls.emplace_back(std::move(new_ls));
        }
    }
    return new_mls;
}

template <typename T>
multi_polygon<T> reproject_internal(multi_polygon<T> const & mpoly, proj_transform const& proj_trans, unsigned int & n_err)
{
    multi_polygon<T> new_mpoly;
    new_mpoly.reserve(mpoly.size());
    for (auto const& poly : mpoly)
    {
        polygon<T> new_poly = reproject_internal(poly, proj_trans, n_err);
        if (!new_poly.exterior_ring.empty())
        {
            new_mpoly.emplace_back(std::move(new_poly));
        }
    }
    return new_mpoly;
}

template <typename T>
geometry_collection<T> reproject_internal(geometry_collection<T> const & c, proj_transform const& proj_trans, unsigned int & n_err)
{
    geometry_collection<T> new_c;
    new_c.reserve(c.size());
    for (auto const& g : c)
    {

        geometry<T> new_g = reproject_copy(g, proj_trans, n_err);
        if (!new_g.template is<geometry_empty>())
        {
            new_c.emplace_back(std::move(new_g));
        }
    }
    return new_c;
}

template <typename T>
struct geom_reproj_copy_visitor
{

    geom_reproj_copy_visitor(proj_transform const & proj_trans, unsigned int & n_err)
        : proj_trans_(proj_trans),
          n_err_(n_err) {}

    geometry<T> operator() (geometry_empty const&) const
    {
        return geometry_empty();
    }

    geometry<T> operator() (point<T> const& p) const
    {
        geometry<T> geom; // default empty
        unsigned int intial_err = n_err_;
        point<T> new_p = reproject_internal(p, proj_trans_, n_err_);
        if (n_err_ > intial_err) return geom;
        geom = std::move(new_p);
        return geom;
    }

    geometry<T> operator() (line_string<T> const& ls) const
    {
        geometry<T> geom; // default empty
        unsigned int intial_err = n_err_;
        line_string<T> new_ls = reproject_internal(ls, proj_trans_, n_err_);
        if (n_err_ > intial_err || new_ls.empty()) return geom;
        geom = std::move(new_ls);
        return geom;
    }

    geometry<T> operator() (polygon<T> const& poly) const
    {
        geometry<T> geom; // default empty
        polygon<T> new_poly = reproject_internal(poly, proj_trans_, n_err_);
        if (new_poly.exterior_ring.empty()) return geom;
        geom = std::move(new_poly);
        return geom;
    }

    geometry<T> operator() (multi_point<T> const& mp) const
    {
        geometry<T> geom; // default empty
        multi_point<T> new_mp = reproject_internal(mp, proj_trans_, n_err_);
        if (new_mp.empty()) return geom;
        geom = std::move(new_mp);
        return geom;
    }

    geometry<T> operator() (multi_line_string<T> const& mls) const
    {
        geometry<T> geom; // default empty
        multi_line_string<T> new_mls = reproject_internal(mls, proj_trans_, n_err_);
        if (new_mls.empty()) return geom;
        geom = std::move(new_mls);
        return geom;
    }

    geometry<T> operator() (multi_polygon<T> const& mpoly) const
    {
        geometry<T> geom; // default empty
        multi_polygon<T> new_mpoly = reproject_internal(mpoly, proj_trans_, n_err_);
        if (new_mpoly.empty()) return geom;
        geom = std::move(new_mpoly);
        return geom;
    }

    geometry<T> operator() (geometry_collection<T> const& c) const
    {
        geometry<T> geom; // default empty
        geometry_collection<T> new_c = reproject_internal(c, proj_trans_, n_err_);
        if (new_c.empty()) return geom;
        geom = std::move(new_c);
        return geom;
    }

private:
    proj_transform const& proj_trans_;
    unsigned int & n_err_;

};

} // end detail ns

template <typename T>
geometry<T> reproject_copy(geometry<T> const& geom, proj_transform const& proj_trans, unsigned int & n_err)
{
    detail::geom_reproj_copy_visitor<T> visit(proj_trans, n_err);
    return mapnik::util::apply_visitor(visit, geom);
}

template <typename T>
T reproject_copy(T const& geom, proj_transform const& proj_trans, unsigned int & n_err)
{
    return detail::reproject_internal(geom, proj_trans, n_err);
}

template <typename T>
T reproject_copy(T const& geom, projection const& source, projection const& dest, unsigned int & n_err)
{
    proj_transform proj_trans(source, dest);
    return reproject_copy(geom, proj_trans, n_err);
}

namespace detail {

struct geom_reproj_visitor {

    geom_reproj_visitor(proj_transform const & proj_trans)
        : proj_trans_(proj_trans) {}

    template <typename T>
    bool operator() (geometry<T> & geom) const
    {
        return mapnik::util::apply_visitor((*this), geom);
    }

    bool operator() (geometry_empty &) const { return true; }

    template <typename T>
    bool operator() (point<T> & p) const
    {
        if (!proj_trans_.forward(p))
        {
            return false;
        }
        return true;
    }

    template <typename T>
    bool operator() (line_string<T> & ls) const
    {
        if (proj_trans_.forward(ls) > 0)
        {
            return false;
        }
        return true;
    }

    template <typename T>
    bool operator() (polygon<T> & poly) const
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

    template <typename T>
    bool operator() (multi_point<T> & mp) const
    {
        return (*this) (static_cast<line_string<T> &>(mp));
    }

    template <typename T>
    bool operator() (multi_line_string<T> & mls) const
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

    template <typename T>
    bool operator() (multi_polygon<T> & mpoly) const
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

    template <typename T>
    bool operator() (geometry_collection<T> & c) const
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

template <typename T>
bool reproject(T & geom, projection const& source, projection const& dest)
{
    proj_transform proj_trans(source, dest);
    detail::geom_reproj_visitor visit(proj_trans);
    return visit(geom);
}

} // end geometry ns

} // end mapnik ns
