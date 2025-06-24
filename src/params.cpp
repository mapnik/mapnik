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
#include <mapnik/boolean.hpp>
#include <mapnik/params.hpp>
#include <mapnik/params_impl.hpp>
#include <mapnik/value/types.hpp>

namespace mapnik {

template std::optional<std::string> parameters::get(std::string const& key) const;
template std::optional<std::string> parameters::get(std::string const& key, std::string const& default_opt_value) const;

template std::optional<value_double> parameters::get(std::string const& key) const;
template std::optional<value_double> parameters::get(std::string const& key,
                                                     value_double const& default_opt_value) const;

template std::optional<value_bool> parameters::get(std::string const& key) const;
template std::optional<value_bool> parameters::get(std::string const& key, value_bool const& default_opt_value) const;

template std::optional<boolean_type> parameters::get(std::string const& key) const;
template std::optional<boolean_type> parameters::get(std::string const& key,
                                                     boolean_type const& default_opt_value) const;

template std::optional<value_null> parameters::get(std::string const& key) const;
template std::optional<value_null> parameters::get(std::string const& key, value_null const& default_opt_value) const;

template std::optional<value_integer> parameters::get(std::string const& key) const;
template std::optional<value_integer> parameters::get(std::string const& key,
                                                      value_integer const& default_opt_value) const;

} // namespace mapnik
