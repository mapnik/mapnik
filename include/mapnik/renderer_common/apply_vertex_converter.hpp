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

#ifndef MAPNIK_APPLY_VERTEX_ADAPTER_HPP
#define MAPNIK_APPLY_VERTEX_ADAPTER_HPP

namespace mapnik { namespace detail {

template <typename VertexConverter, typename Processor>
struct apply_vertex_converter
{
    apply_vertex_converter(VertexConverter & converter, Processor & proc)
        : converter_(converter), proc_(proc) {}
    template <typename Adapter>
    void operator() (Adapter const& adapter) const
    {
        converter_.apply(adapter, proc_);
    }
    VertexConverter & converter_;
    Processor & proc_;
};

}}

#endif // MAPNIK_APPLY_VERTEX_ADAPTER_HPP
