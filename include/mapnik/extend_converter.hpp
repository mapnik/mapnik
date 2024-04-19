/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_EXTEND_CONVERTER_HPP
#define MAPNIK_EXTEND_CONVERTER_HPP

#include <mapnik/vertex.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
MAPNIK_DISABLE_UNUSED_VARIABLE
#include <mapnik/warning_ignore.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <cmath>
#include <optional>

namespace mapnik {

namespace detail {

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;

template<typename T>
T extend(T const& v1, T const& v2, double length)
{
    double dx = v2.x - v1.x;
    double dy = v2.y - v1.y;
    double l12 = std::sqrt(dx * dx + dy * dy);
    double coef = 1.0 + length / l12;
    return vertex2d(v1.x + dx * coef, v1.y + dy * coef, v2.cmd);
}

namespace events {
struct vertex_event
{
    vertex_event(vertex2d const& vertex)
        : vertex(vertex)
    {}
    vertex2d const& vertex;
};

struct move_to : vertex_event
{
    using vertex_event::vertex_event;
};
struct line_to : vertex_event
{
    using vertex_event::vertex_event;
};
struct close : vertex_event
{
    using vertex_event::vertex_event;
};
struct end : vertex_event
{
    using vertex_event::vertex_event;
};
} // namespace events

namespace actions {
struct store
{
    template<class FSM, class EVT, class SourceState, class TargetState>
    void operator()(EVT const& e, FSM& m, SourceState&, TargetState&)
    {
        m.v2 = m.v1;
        m.v1 = e.vertex;
        m.output = std::nullopt;
    }
};

struct output
{
    template<class FSM, class EVT, class SourceState, class TargetState>
    void operator()(EVT const& e, FSM& m, SourceState&, TargetState&)
    {
        m.output = e.vertex;
    }
};

struct store_and_output
{
    template<class FSM, class EVT, class SourceState, class TargetState>
    void operator()(EVT const& e, FSM& m, SourceState&, TargetState&)
    {
        m.v2 = m.v1;
        m.v1 = e.vertex;
        m.output = m.v2;
    }
};

struct output_begin
{
    template<class FSM, class EVT, class SourceState, class TargetState>
    void operator()(EVT const& e, FSM& m, SourceState&, TargetState&)
    {
        m.v2 = m.v1;
        m.v1 = e.vertex;
        m.output = extend(m.v1, m.v2, m.extend_length);
    }
};

struct output_end
{
    template<class FSM, class EVT, class SourceState, class TargetState>
    void operator()(EVT const& e, FSM& m, SourceState&, TargetState&)
    {
        m.output = extend(m.v2, m.v1, m.extend_length);
        m.v1 = e.vertex;
    }
};
} // namespace actions

struct extender_def : public msm::front::state_machine_def<extender_def>
{
    using no_exception_thrown = int;
    using no_message_queue = int;

    struct initial : public msm::front::state<>
    {};
    struct vertex_one : public msm::front::state<>
    {};
    struct vertex_two : public msm::front::state<>
    {};
    struct end : public msm::front::state<>
    {};

    using initial_state = initial;

    // clang-format off
    struct transition_table : mpl::vector<
        //  Start         Event                Next      Action                Guard
        //  +------------+-----------------+------------+--------------------+------+
        Row < initial    , events::move_to , vertex_one , actions::store            >,
        Row < initial    , events::line_to , vertex_one , actions::store            >,
        Row < initial    , events::close   , initial                                >,
        Row < initial    , events::end     , end        , actions::output           >,
        Row < vertex_one , events::move_to , vertex_one , actions::store_and_output >,
        Row < vertex_one , events::line_to , vertex_two , actions::output_begin     >,
        Row < vertex_one , events::close   , initial    , actions::store_and_output >,
        Row < vertex_one , events::end     , end        , actions::store_and_output >,
        Row < vertex_two , events::move_to , vertex_one , actions::output_end       >,
        Row < vertex_two , events::line_to , vertex_two , actions::store_and_output >,
        Row < vertex_two , events::close   , initial    , actions::output_end       >,
        Row < vertex_two , events::end     , end        , actions::output_end       >,
        Row < end        , events::end     , end        , actions::output           >
    > {};
    // clang-format on

    extender_def(double extend_length)
        : extend_length(extend_length)
    {}

    std::optional<vertex2d> output;
    vertex2d v1, v2;
    double extend_length;
};

using extender = msm::back::state_machine<extender_def>;

} // namespace detail

template<typename Geometry>
struct extend_converter
{
    extend_converter(Geometry& geom)
        : extend_converter(geom, 0)
    {}

    extend_converter(Geometry& geom, double extend)
        : geom_(geom)
        , extender_(extend)
    {}

    void set_extend(double extend) { extender_.extend_length = extend; }

    unsigned vertex(double* x, double* y)
    {
        using namespace detail;
        vertex2d v;
        do
        {
            v.cmd = geom_.vertex(&v.x, &v.y);
            switch (v.cmd)
            {
                case SEG_MOVETO:
                    extender_.process_event(events::move_to(v));
                    break;
                case SEG_LINETO:
                    extender_.process_event(events::line_to(v));
                    break;
                case SEG_CLOSE:
                    extender_.process_event(events::close(v));
                    break;
                case SEG_END:
                    extender_.process_event(events::end(v));
                    break;
            }
        } while (!extender_.output);

        vertex2d const& output = *extender_.output;
        *x = output.x;
        *y = output.y;
        return output.cmd;
    }

    void rewind(unsigned)
    {
        geom_.rewind(0);
        extender_.start();
    }

  private:
    Geometry& geom_;
    detail::extender extender_;
};

} // namespace mapnik

#endif // MAPNIK_EXTEND_CONVERTER_HPP
