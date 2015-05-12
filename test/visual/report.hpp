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

#ifndef CONSOLE_REPORT_HPP
#define CONSOLE_REPORT_HPP

// stl
#include <iostream>

#include <mapnik/util/variant.hpp>

#include "config.hpp"

namespace visual_tests
{

class console_report
{
public:
    console_report() : s(std::clog)
    {
    }

    console_report(std::ostream & s) : s(s)
    {
    }

    void report(result const & r);
    unsigned summary(result_list const & results);

protected:
    std::ostream & s;
};

class console_short_report : public console_report
{
public:
    console_short_report() : console_report()
    {
    }

    console_short_report(std::ostream & s) : console_report(s)
    {
    }

    void report(result const & r);
};

class html_report
{
public:
    html_report(std::ostream & s) : s(s)
    {
    }

    void report(result const & r, boost::filesystem::path const & output_dir);
    void summary(result_list const & results, boost::filesystem::path const & output_dir);

protected:
    std::ostream & s;
};

using report_type = mapnik::util::variant<console_report, console_short_report>;

class report_visitor
{
public:
    report_visitor(result const & r)
        : result_(r)
    {
    }

    template <typename T>
    void operator()(T & report) const
    {
        return report.report(result_);
    }

private:
    result const & result_;
};

class summary_visitor
{
public:
    summary_visitor(result_list const & r)
        : result_(r)
    {
    }

    template <typename T>
    unsigned operator()(T & report) const
    {
        return report.summary(result_);
    }

private:
    result_list const & result_;
};

void html_summary(result_list const & results, boost::filesystem::path output_dir);

}

#endif
