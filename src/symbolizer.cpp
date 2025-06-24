/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/symbolizer.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/transform/transform_processor.hpp>

namespace mapnik {

// START FIXME - move to its own compilation unit
void evaluate_transform(agg::trans_affine& tr,
                        feature_impl const& feature,
                        attributes const& vars,
                        transform_list_ptr const& trans_expr,
                        double scale_factor)
{
    if (trans_expr)
    {
        transform_processor_type::evaluate(tr, feature, vars, *trans_expr, scale_factor);
    }
}
// END FIXME

} // end of namespace mapnik
