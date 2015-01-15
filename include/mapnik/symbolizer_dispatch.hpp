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

#ifndef MAPNIK_SYMBOLIZER_DISPATCH_HPP
#define MAPNIK_SYMBOLIZER_DISPATCH_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/util/variant.hpp>

namespace mapnik
{

template <typename T0,typename T1> struct has_process;

template <bool>
struct process_impl
{
    template <typename T0, typename T1, typename T2, typename T3>
    static void process(T0 & ren, T1 const& sym, T2 & f, T3 const& tr)
    {
        ren.process(sym,f,tr);
    }
};

template <> // No-op specialization
struct process_impl<false>
{
    template <typename T0, typename T1, typename T2, typename T3>
    static void process(T0 &, T1 const&, T2 &, T3 const&)
    {
#ifdef MAPNIK_DEBUG
    #ifdef _MSC_VER
    #pragma NOTE(process function not implemented)
    #else
    #warning process function not implemented
    #endif
#endif
    }
};

/** Calls the renderer's process function,
 * \param output     Renderer
 * \param f          Feature to process
 * \param prj_trans  Projection
 * \param sym        Symbolizer object
 */
template <typename Processor>
struct symbolizer_dispatch
{
    symbolizer_dispatch(Processor & output,
                        mapnik::feature_impl & f,
                        proj_transform const& prj_trans)
        : output_(output),
          f_(f),
          prj_trans_(prj_trans)  {}

    template <typename T>
    void operator () (T const& sym) const
    {
        process_impl<has_process<Processor,T>::value>::process(output_,sym,f_,prj_trans_);
    }

    Processor & output_;
    mapnik::feature_impl & f_;
    proj_transform const& prj_trans_;
};

using no_tag = char (&)[1];
using yes_tag = char (&)[2];

template <typename T0, typename T1, void (T0::*)(T1 const&, mapnik::feature_impl &, proj_transform const&) >
struct process_memfun_helper {};

template <typename T0, typename T1> no_tag  has_process_helper(...);
template <typename T0, typename T1> yes_tag has_process_helper(process_memfun_helper<T0, T1, &T0::process>* p);

template<typename T0,typename T1>
struct has_process
{
    using processor_impl_type = typename T0::processor_impl_type;
    BOOST_STATIC_CONSTANT(bool
                          , value = sizeof(has_process_helper<processor_impl_type,T1>(0)) == sizeof(yes_tag)
        );
};

}

#endif // MAPNIK_SYMBOLIZER_DISPATCH_HPP
