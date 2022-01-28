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
#include <iomanip>
#include <fstream>
#include <numeric>
#include <map>

#include "report.hpp"

namespace visual_tests {

void console_report::report(result const& r)
{
    s << '"' << r.name << '-' << r.size.width << '-' << r.size.height;
    if (r.tiles.width > 1 || r.tiles.height > 1)
    {
        s << '-' << r.tiles.width << 'x' << r.tiles.height;
    }
    s << '-' << std::fixed << std::setprecision(1) << r.scale_factor << "\" with " << r.renderer_name << "... ";
    report_state(r);

    if (show_duration)
    {
        s << " (" << std::chrono::duration_cast<std::chrono::milliseconds>(r.duration).count() << " milliseconds)";
    }

    s << std::endl;
}

unsigned console_report::summary(result_list const& results)
{
    unsigned ok = 0;
    unsigned error = 0;
    unsigned fail = 0;
    unsigned overwrite = 0;

    using namespace std::chrono;
    using duration_map_type = std::map<std::string, high_resolution_clock::duration>;
    duration_map_type durations;

    for (auto const& r : results)
    {
        switch (r.state)
        {
            case STATE_OK:
                ok++;
                break;
            case STATE_FAIL:
                fail++;
                break;
            case STATE_ERROR:
                error++;
                break;
            case STATE_OVERWRITE:
                overwrite++;
                break;
        }

        if (show_duration)
        {
            duration_map_type::iterator duration = durations.find(r.renderer_name);
            if (duration == durations.end())
            {
                durations.emplace(r.renderer_name, r.duration);
            }
            else
            {
                duration->second += r.duration;
            }
        }
    }

    s << std::endl;
    s << "Visual rendering: " << fail << " failed / " << ok << " passed / " << overwrite << " overwritten / " << error
      << " errors" << std::endl;

    if (show_duration)
    {
        high_resolution_clock::duration total(0);
        for (auto const& duration : durations)
        {
            s << duration.first << ": \t" << duration_cast<milliseconds>(duration.second).count() << " milliseconds"
              << std::endl;
            total += duration.second;
        }
        s << "total: \t" << duration_cast<milliseconds>(total).count() << " milliseconds" << std::endl;
    }

    s << std::endl;
    report_failures(results);
    s << std::endl;

    return fail + error;
}

void console_report::report_state(result const& r)
{
    switch (r.state)
    {
        case STATE_OK:
            s << "OK";
            break;
        case STATE_FAIL:
            s << "FAILED (" << r.diff << " different pixels)";
            break;
        case STATE_OVERWRITE:
            s << "OVERWRITTEN (" << r.diff << " different pixels)";
            break;
        case STATE_ERROR:
            s << "ERROR (" << r.error_message << ")";
            break;
    }
}

void console_report::report_failures(result_list const& results)
{
    for (auto const& r : results)
    {
        if (r.state == STATE_OK)
        {
            continue;
        }

        s << r.name << " ";
        report_state(r);
        s << std::endl;
        if (!r.actual_image_path.empty())
        {
            s << "    " << r.actual_image_path << " (actual)" << std::endl;
            s << "    " << r.reference_image_path << " (reference)" << std::endl;
        }
    }
}

void console_short_report::report(result const& r)
{
    switch (r.state)
    {
        case STATE_OK:
            s << ".";
            break;
        case STATE_FAIL:
            s << "✘";
            break;
        case STATE_OVERWRITE:
            s << "✓";
            break;
        case STATE_ERROR:
            s << "ERROR (" << r.error_message << ")\n";
            break;
    }
}

void html_report::report(result const& r, boost::filesystem::path const& output_dir)
{
    if (r.state == STATE_ERROR)
    {
        s << "<div class=\"text\">Failed to render: " << r.name << "<br><em>" << r.error_message << "</em></div>\n";
    }
    else if (r.state == STATE_FAIL)
    {
        using namespace boost::filesystem;

        path reference = output_dir / r.reference_image_path.filename();
        path actual = output_dir / r.actual_image_path.filename();

        if (exists(reference))
        {
            remove(reference);
        }
        if (exists(actual))
        {
            remove(actual);
        }
        copy_file(r.reference_image_path, reference);
        copy_file(r.actual_image_path, actual);
        // clang-format off
        s << "<p>" << r.diff << "</p>\n"
            "<div class=\"r\">"
            "  <div class=\"i\">"
            "    <a href=" << reference.filename() << ">\n"
            "      <img src=" << reference.filename() << " width=\"100%\">\n"
            "    </a>\n"
            "  </div>\n"
            "  <div class=\"i2\">\n"
            "    <a href=" << actual.filename() << ">\n"
            "      <img src=" << actual.filename() << " width=\"100%\">\n"
            "    </a>\n"
            "  </div>\n"
            "</div>\n";
        // clang-format on
    }
}

constexpr const char* html_header = R"template(<!DOCTYPE html>
<html>
<head>
  <style>
    .r {
      width:100%;
      display: flex;
      position: relative;
      border:1px solid black;
      margin-bottom: 20px;
    }
    .i2 { max-width: 50%; width:50%; }
    .i { max-width: 50%; width:50%; }
    .i:hover{ position: absolute; top:0; left:0; }
    .i img, .i2 img { width: 100%; }
    .i img:hover { mix-blend-mode: difference; }
  </style>
</head>
<body>
)template";

constexpr const char* html_footer = R"template(</div>
</body>
</html>)template";

void html_report::summary(result_list const& results, boost::filesystem::path const& output_dir)
{
    s << html_header;

    for (auto const& r : results)
    {
        if (r.state != STATE_OK)
        {
            report(r, output_dir);
        }
    }

    s << html_footer;
}

void html_summary(result_list const& results, boost::filesystem::path output_dir)
{
    boost::filesystem::path html_root = output_dir / "visual-test-results";
    boost::filesystem::create_directories(html_root);
    boost::filesystem::path html_report_path = html_root / "index.html";
    std::clog << "View failure report at " << html_report_path << "\n";
    std::ofstream output_file(html_report_path.string());
    html_report report(output_file);
    report.summary(results, html_root);
}

} // namespace visual_tests
