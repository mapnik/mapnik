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

#ifndef VISUAL_TEST_RUNNER_HPP
#define VISUAL_TEST_RUNNER_HPP

#include "config.hpp"
#include "report.hpp"
#include "renderer.hpp"

namespace visual_tests {

class runner
{
    using path_type = mapnik::fs::path;
    using files_iterator = std::vector<path_type>::const_iterator;

  public:
    using renderer_container = std::vector<renderer_type>;

    runner(path_type const& styles_dir,
           config const& cfg,
           std::size_t iterations,
           std::size_t fail_limit,
           std::size_t jobs,
           renderer_container const& renderers);

    result_list test_all(report_type& report) const;
    result_list test(std::vector<std::string> const& style_names, report_type& report) const;

  private:
    result_list test_parallel(std::vector<path_type> const& files, report_type& report, std::size_t jobs) const;
    result_list test_range(files_iterator begin,
                           files_iterator end,
                           std::reference_wrapper<report_type> report,
                           std::reference_wrapper<std::atomic<std::size_t>> fail_limit) const;
    result_list test_one(path_type const& style_path, report_type& report, std::atomic<std::size_t>& fail_limit) const;
    void parse_params(mapnik::parameters const& params, config& cfg) const;

    const path_type styles_dir_;
    const config defaults_;
    const std::size_t jobs_;
    const std::size_t iterations_;
    const std::size_t fail_limit_;
    const renderer_container renderers_;
};

} // namespace visual_tests

#endif // VISUAL_TEST_RUNNER_HPP
