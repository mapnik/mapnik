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

// stl
#include <algorithm>
#include <future>
#include <atomic>

#include <mapnik/load_map.hpp>

#include "runner.hpp"
#include "parse_map_sizes.hpp"

namespace visual_tests {

struct renderer_name_visitor
{
    template<typename Renderer>
    std::string operator()(Renderer const&) const
    {
        return Renderer::renderer_type::name;
    }
};

class renderer_visitor
{
  public:
    renderer_visitor(std::string const& name,
                     mapnik::Map& map,
                     map_size const& tiles,
                     double scale_factor,
                     result_list& results,
                     report_type& report,
                     std::size_t iterations,
                     bool is_fail_limit,
                     std::atomic<std::size_t>& fail_count)
        : name_(name)
        , map_(map)
        , tiles_(tiles)
        , scale_factor_(scale_factor)
        , results_(results)
        , report_(report)
        , iterations_(iterations)
        , is_fail_limit_(is_fail_limit)
        , fail_count_(fail_count)
    {}

    template<typename T, typename std::enable_if<T::renderer_type::support_tiles>::type* = nullptr>
    void operator()(T const& renderer) const
    {
        test(renderer);
    }

    template<typename T, typename std::enable_if<!T::renderer_type::support_tiles>::type* = nullptr>
    void operator()(T const& renderer) const
    {
        if (tiles_.width == 1 && tiles_.height == 1)
        {
            test(renderer);
        }
    }

  private:
    template<typename T>
    void test(T const& renderer) const
    {
        map_size size{map_.width(), map_.height()};
        std::chrono::high_resolution_clock::time_point start(std::chrono::high_resolution_clock::now());
        for (std::size_t i = iterations_; i > 0; i--)
        {
            typename T::image_type image(render(renderer));
            if (i == 1)
            {
                std::chrono::high_resolution_clock::time_point end(std::chrono::high_resolution_clock::now());
                result r(renderer.report(image, name_, size, tiles_, scale_factor_));
                r.duration = end - start;
                mapnik::util::apply_visitor(report_visitor(r), report_);
                results_.push_back(std::move(r));
                if (is_fail_limit_ && r.state == STATE_FAIL)
                {
                    ++fail_count_;
                }
            }
        }
    }

    template<typename T, typename std::enable_if<T::renderer_type::support_tiles>::type* = nullptr>
    typename T::image_type render(T const& renderer) const
    {
        if (tiles_.width == 1 && tiles_.height == 1)
        {
            return renderer.render(map_, scale_factor_);
        }
        else
        {
            return renderer.render(map_, scale_factor_, tiles_);
        }
    }

    template<typename T, typename std::enable_if<!T::renderer_type::support_tiles>::type* = nullptr>
    typename T::image_type render(T const& renderer) const
    {
        return renderer.render(map_, scale_factor_);
    }

    std::string const& name_;
    mapnik::Map& map_;
    map_size const& tiles_;
    double scale_factor_;
    result_list& results_;
    report_type& report_;
    std::size_t iterations_;
    bool is_fail_limit_;
    std::atomic<std::size_t>& fail_count_;
};

runner::runner(runner::path_type const& styles_dir,
               config const& defaults,
               std::size_t iterations,
               std::size_t fail_limit,
               std::size_t jobs,
               runner::renderer_container const& renderers)
    : styles_dir_(styles_dir)
    , defaults_(defaults)
    , jobs_(jobs)
    , iterations_(iterations)
    , fail_limit_(fail_limit)
    , renderers_(renderers)
{}

result_list runner::test_all(report_type& report) const
{
    boost::filesystem::directory_iterator begin(styles_dir_);
    boost::filesystem::directory_iterator end;
    std::vector<runner::path_type> files(begin, end);
    return test_parallel(files, report, jobs_);
}

result_list runner::test(std::vector<std::string> const& style_names, report_type& report) const
{
    std::vector<runner::path_type> files;
    files.reserve(style_names.size());
    std::transform(style_names.begin(),
                   style_names.end(),
                   std::back_inserter(files),
                   [&](runner::path_type const& name) {
                       return (name.extension() == ".xml") ? name : (styles_dir_ / (name.string() + ".xml"));
                   });
    return test_parallel(files, report, jobs_);
}

result_list
  runner::test_parallel(std::vector<runner::path_type> const& files, report_type& report, std::size_t jobs) const
{
    result_list results;

    if (files.empty())
    {
        return results;
    }

    if (jobs == 0)
    {
        jobs = 1;
    }

    std::size_t chunk_size = files.size() / jobs;

    if (chunk_size == 0)
    {
        chunk_size = files.size();
        jobs = 1;
    }

    std::launch launch(jobs == 1 ? std::launch::deferred : std::launch::async);
    std::vector<std::future<result_list>> futures(jobs);
    std::atomic<std::size_t> fail_count(0);

    for (std::size_t i = 0; i < jobs; i++)
    {
        files_iterator begin(files.begin() + i * chunk_size);
        files_iterator end(files.begin() + (i + 1) * chunk_size);

        // Handle remainder of files.size() / jobs
        if (i == jobs - 1)
        {
            end = files.end();
        }

        futures[i] = std::async(launch, &runner::test_range, this, begin, end, std::ref(report), std::ref(fail_count));
    }

    for (auto& f : futures)
    {
        result_list r = f.get();
        std::move(r.begin(), r.end(), std::back_inserter(results));
    }

    return results;
}

result_list runner::test_range(files_iterator begin,
                               files_iterator end,
                               std::reference_wrapper<report_type> report,
                               std::reference_wrapper<std::atomic<std::size_t>> fail_count) const
{
    result_list results;

    for (runner::files_iterator i = begin; i != end; i++)
    {
        runner::path_type const& file = *i;
        if (file.extension() == ".xml")
        {
            try
            {
                result_list r = test_one(file, report, fail_count.get());
                std::move(r.begin(), r.end(), std::back_inserter(results));
            } catch (std::exception const& ex)
            {
                result r;
                r.state = STATE_ERROR;
                r.name = file.string();
                r.error_message = ex.what();
                r.duration = std::chrono::high_resolution_clock::duration::zero();
                results.emplace_back(r);
                mapnik::util::apply_visitor(report_visitor(r), report.get());
                ++fail_count.get();
            }
        }
        if (fail_limit_ && fail_count.get() >= fail_limit_)
        {
            break;
        }
    }

    return results;
}

void runner::parse_params(mapnik::parameters const& params, config& cfg) const
{
    cfg.status = *params.get<mapnik::value_bool>("status", cfg.status);

    boost::optional<std::string> sizes = params.get<std::string>("sizes");

    if (sizes)
    {
        cfg.sizes.clear();
        parse_map_sizes(*sizes, cfg.sizes);
    }

    boost::optional<std::string> tiles = params.get<std::string>("tiles");

    if (tiles)
    {
        cfg.tiles.clear();
        parse_map_sizes(*tiles, cfg.tiles);
    }

    boost::optional<std::string> bbox_string = params.get<std::string>("bbox");

    if (bbox_string)
    {
        cfg.bbox.from_string(*bbox_string);
    }

    for (auto const& renderer : renderers_)
    {
        std::string renderer_name = mapnik::util::apply_visitor(renderer_name_visitor(), renderer);
        boost::optional<mapnik::value_bool> enabled = params.get<mapnik::value_bool>(renderer_name);
        if (enabled && !*enabled)
        {
            cfg.ignored_renderers.insert(renderer_name);
        }
    }
}

result_list
  runner::test_one(runner::path_type const& style_path, report_type& report, std::atomic<std::size_t>& fail_count) const
{
    config cfg(defaults_);
    mapnik::Map map(cfg.sizes.front().width, cfg.sizes.front().height);
    result_list results;

    try
    {
        mapnik::load_map(map, style_path.string(), true);
    } catch (std::exception const& ex)
    {
        std::string what = ex.what();
        if (what.find("Could not create datasource") != std::string::npos ||
            what.find("Postgis Plugin: could not connect to server") != std::string::npos)
        {
            return results;
        }
        throw;
    }

    parse_params(map.get_extra_parameters(), cfg);

    if (!cfg.status)
    {
        return results;
    }

    std::string name(style_path.stem().string());

    for (auto const& size : cfg.sizes)
    {
        for (auto const& scale_factor : cfg.scales)
        {
            for (auto const& tiles_count : cfg.tiles)
            {
                if (!tiles_count.width || !tiles_count.height)
                {
                    throw std::runtime_error("Cannot render zero tiles.");
                }
                if (size.width % tiles_count.width || size.height % tiles_count.height)
                {
                    throw std::runtime_error("Tile size is not an integer.");
                }

                for (auto const& ren : renderers_)
                {
                    std::string renderer_name = mapnik::util::apply_visitor(renderer_name_visitor(), ren);
                    if (cfg.ignored_renderers.count(renderer_name))
                    {
                        continue;
                    }

                    map.resize(size.width, size.height);
                    if (cfg.bbox.valid())
                    {
                        map.zoom_to_box(cfg.bbox);
                    }
                    else
                    {
                        map.zoom_all();
                    }
                    mapnik::util::apply_visitor(renderer_visitor(name,
                                                                 map,
                                                                 tiles_count,
                                                                 scale_factor,
                                                                 results,
                                                                 report,
                                                                 iterations_,
                                                                 fail_limit_,
                                                                 fail_count),
                                                ren);
                    if (fail_limit_ && fail_count >= fail_limit_)
                    {
                        return results;
                    }
                }
            }
        }
    }
    return results;
}

} // namespace visual_tests
