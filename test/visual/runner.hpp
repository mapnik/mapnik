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

#ifndef VISUAL_TEST_RUNNER_HPP
#define VISUAL_TEST_RUNNER_HPP

#include <mapnik/util/variant.hpp>

#include "config.hpp"
#include "report.hpp"
#include "renderer.hpp"
#include "map_sizes_grammar.hpp"

namespace visual_tests
{

class runner
{
    using renderer_type = mapnik::util::variant<renderer<agg_renderer>,
                                                renderer<cairo_renderer>/*,
                                                renderer<grid_renderer>*/>;
    using path_type = boost::filesystem::path;
    using files_iterator = std::vector<path_type>::const_iterator;

public:
    runner(path_type const & styles_dir,
           path_type const & output_dir,
           path_type const & reference_dir,
           bool overwrite,
           std::size_t jobs);

    result_list test_all(report_type & report) const;
    result_list test(std::vector<std::string> const & style_names, report_type & report) const;

private:
    result_list test_parallel(std::vector<path_type> const & files, report_type & report, std::size_t jobs) const;
    result_list test_range(files_iterator begin, files_iterator end, std::reference_wrapper<report_type> report) const;
    result_list test_one(path_type const & style_path, config cfg, report_type & report) const;
    void parse_map_sizes(std::string const & str, std::vector<map_size> & sizes) const;

    const map_sizes_grammar<std::string::const_iterator> map_sizes_parser_;
    const path_type styles_dir_;
    const path_type output_dir_;
    const path_type reference_dir_;
    const std::size_t jobs_;
    const renderer_type renderers_[boost::mpl::size<renderer_type::types>::value];
};

}

#endif
