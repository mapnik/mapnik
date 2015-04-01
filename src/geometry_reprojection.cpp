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

namespace mapnik {

namespace detail {

struct geom_reproj_visitor {

    geom_reproj_visitor(proj_transform const & proj_trans, unsigned int & n_err, bool reverse)
        : proj_trans_(proj_trans), n_err_(n_err), reverse_(reverse) {}

    geometry::geometry operator() (geometry::geometry_empty const&) 
    {
        return geometry::geometry(geometry::geometry_empty());
    }
    
    bool trans_point(geometry::point & new_p)
    {
        if (!reverse_)
        {
            if (!proj_trans_.forward(new_p))
            {
                n_err_++;
                return false;
            }
        }
        else
        {
            if (!proj_trans_.backward(new_p))
            {
                n_err_++;
                return false;
            }
        }
        return true;
    }
    
    geometry::geometry operator() (geometry::point const & p)
    {
        geometry::point new_p(p);
        if (!trans_point(new_p))
        {
            return geometry::geometry(geometry::geometry_empty());
        }
        return geometry::geometry(std::move(new_p));
    }

    bool trans_ls(geometry::line_string & new_ls)
    {
        unsigned int err = 0;
        if (!reverse_)
        {
            err = proj_trans_.forward(new_ls);
        }
        else
        {
            err = proj_trans_.backward(new_ls);
        }
        if (err > 0)
        {
            n_err_ += err;
            return false;
        }
        return true;
    }
    
    geometry::geometry operator() (geometry::line_string const & ls)
    {
        geometry::line_string new_ls(ls);
        if (!trans_ls(new_ls))
        {
            return geometry::geometry(geometry::geometry_empty());
        }
        return geometry::geometry(std::move(new_ls));
    }

    bool trans_poly(geometry::polygon & new_poly, geometry::polygon const & poly)
    {
        geometry::linear_ring new_ext(poly.exterior_ring);
        int err = 0;
        if (!reverse_)
        {
            err = proj_trans_.forward(new_ext);
        }
        else
        {
            err = proj_trans_.backward(new_ext);
        }
        // If the exterior ring doesn't transform don't bother with the holes.
        if (err > 0)
        {
            n_err_ += err;
            return false;
        }
        new_poly.set_exterior_ring(std::move(new_ext));
        new_poly.interior_rings.reserve(poly.interior_rings.size());

        for (auto lr : poly.interior_rings)
        {
            geometry::linear_ring new_lr(lr);
            if (!reverse_)
            {
                err = proj_trans_.forward(new_lr);
            }
            else
            {
                err = proj_trans_.backward(new_lr);
            }
            if (err > 0)
            {
                n_err_ += err;
                // If there is an error in interior ring drop
                // it from polygon.
                continue;
            }
            new_poly.add_hole(std::move(new_lr));
        }
        return true;
    }
    
    geometry::geometry operator() (geometry::polygon const & poly)
    {
        geometry::polygon new_poly;
        if (!trans_poly(new_poly, poly))
        {
            return std::move(geometry::geometry_empty());
        }
        return std::move(new_poly);
    }
    
    geometry::geometry operator() (geometry::multi_point const & mp)
    {
        if (proj_trans_.is_known())
        {
            geometry::multi_point new_mp(mp);
            if (!trans_ls(new_mp))
            {
                // should be impossible to reach this currently
                /* LCOV_EXCL_START */
                return std::move(geometry::geometry_empty());
                /* LCOV_EXCL_END */
            }
            return std::move(new_mp);
        }
        geometry::multi_point new_mp;
        new_mp.reserve(mp.size());
        for (auto p : mp)
        {
            geometry::point new_p(p);
            if (trans_point(new_p))
            {
                new_mp.emplace_back(new_p);
            }
        }
        if (new_mp.empty())
        {
            return std::move(geometry::geometry_empty());
        }
        return std::move(new_mp);
    }
    
    geometry::geometry operator() (geometry::multi_line_string const & mls)
    {
        geometry::multi_line_string new_mls;
        new_mls.reserve(mls.size());
        for (auto ls : mls)
        {
            geometry::line_string new_ls(ls);
            if (trans_ls(new_ls))
            {
                new_mls.emplace_back(new_ls);
            }
        }
        if (new_mls.empty())
        {
            return std::move(geometry::geometry_empty());
        }
        return std::move(new_mls);
    }
    
    geometry::geometry operator() (geometry::multi_polygon const & mpoly)
    {
        geometry::multi_polygon new_mpoly;
        new_mpoly.reserve(mpoly.size());
        for (auto poly : mpoly)
        {
            geometry::polygon new_poly;
            if (trans_poly(new_poly, poly))
            {
                new_mpoly.emplace_back(new_poly);
            }
        }
        if (new_mpoly.empty())
        {
            return std::move(geometry::geometry_empty());
        }
        return std::move(new_mpoly);
    }
    
    geometry::geometry operator() (geometry::geometry_collection const & c)
    {
        geometry::geometry_collection new_c;
        new_c.reserve(c.size());
        for (auto g : c)
        {
            geometry::geometry new_g(std::move(reproject(g, proj_trans_, n_err_, reverse_)));
            if (!new_g.is<geometry::geometry_empty>())
            {
                new_c.emplace_back(new_g);
            }
        }
        if (new_c.empty())
        {
            return std::move(geometry::geometry_empty());
        }
        return std::move(new_c);
    }

  private:
    proj_transform const& proj_trans_;
    unsigned int & n_err_;
    bool reverse_;

};

} // end detail ns

geometry::geometry reproject(geometry::geometry const& geom, proj_transform const& proj_trans, unsigned int & n_err, bool backwards)
{
    detail::geom_reproj_visitor visit(proj_trans, n_err, backwards);
    return util::apply_visitor(visit, geom);  
}

} // end mapnik ns
