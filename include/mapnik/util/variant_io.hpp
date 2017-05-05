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

#ifndef MAPNIK_UTIL_VARIANT_IO_HPP
#define MAPNIK_UTIL_VARIANT_IO_HPP


namespace mapbox { namespace util {

namespace detail {

// operator<< helper
template <typename Out>
class printer
{
public:
    explicit printer(Out & out)
        : out_(out) {}
    printer& operator=(printer const&) = delete;

// visitor
    template <typename T>
    void operator()(T const& operand) const
    {
        out_ << operand;
    }

/// specialized visitor for boolean
    void operator()(bool const & val) const
    {
        if (val) {
            out_ << "true";
        } else {
            out_ << "false";
        }
    }
private:
    Out & out_;
};

} // namespace detail

// operator<<
template <typename charT, typename traits, typename... Types>
VARIANT_INLINE std::basic_ostream<charT, traits>&
operator<< (std::basic_ostream<charT, traits>& out, variant<Types...> const& rhs)
{
    detail::printer<std::basic_ostream<charT, traits>> visitor(out);
    mapnik::util::apply_visitor(visitor, rhs);
    return out;
}

}}

#endif // MAPNIK_UTIL_VARIANT_IO_HPP
