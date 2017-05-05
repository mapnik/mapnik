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
#include <mapnik/text/placement_finder.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/tolerance_iterator.hpp>
#include <mapnik/symbolizer_enumerations.hpp>

#include <memory>

namespace mapnik
{

template <typename T>
bool placement_finder::find_line_placements(T & path, bool points)
{
    if (!layouts_.line_count()) return true; //TODO
    vertex_cache pp(path);

    bool success = false;
    while (pp.next_subpath())
    {
        if (points)
        {
            if (pp.length() <= 0.001)
            {
                success = find_point_placement(pp.current_position()) || success;
                continue;
            }
        }
        else
        {
            if ((pp.length() < text_props_->minimum_path_length * scale_factor_)
                ||
                (pp.length() <= 0.001) // Clipping removed whole geometry
                ||
                (pp.length() < layouts_.width()))
                {
                    continue;
                }
        }

        double spacing = get_spacing(pp.length(), points ? 0. : layouts_.width());

        //horizontal_alignment_e halign = layouts_.back()->horizontal_alignment();

        // halign == H_LEFT -> don't move
        if (horizontal_alignment_ == H_MIDDLE || horizontal_alignment_ == H_AUTO || horizontal_alignment_ == H_ADJUST)
        {
            if (!pp.forward(spacing / 2.0)) continue;
        }
        else if (horizontal_alignment_ == H_RIGHT)
        {
            if (!pp.forward(pp.length())) continue;
        }

        if (move_dx_ != 0.0) path_move_dx(pp, move_dx_);

        do
        {
            tolerance_iterator tolerance_offset(text_props_->label_position_tolerance * scale_factor_, spacing); //TODO: Handle halign
            while (tolerance_offset.next())
            {
                vertex_cache::scoped_state state(pp);
                if (pp.move(tolerance_offset.get())
                    && ((points && find_point_placement(pp.current_position()))
                        || (!points && single_line_placement(pp, text_props_->upright))))
                {
                    success = true;
                    break;
                }
            }
        } while (pp.forward(spacing));
    }
    return success;
}

}// ns mapnik
