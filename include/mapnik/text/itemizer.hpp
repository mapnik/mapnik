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

#ifndef MAPNIK_TEXT_ITEMIZER_HPP
#define MAPNIK_TEXT_ITEMIZER_HPP

//mapnik
#include <mapnik/text/evaluated_format_properties_ptr.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/config.hpp>

// stl
#include <string>
#include <list>
#include <vector>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <unicode/unistr.h>
#include <unicode/uscript.h>
#include <unicode/ubidi.h>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik
{

struct MAPNIK_DECL text_item : util::noncopyable
{
    text_item(unsigned s,
              unsigned e,
              UScriptCode sc,
              UBiDiDirection d,
              evaluated_format_properties_ptr const& f)
      : start(s),
        end(e),
        script(sc),
        dir(d),
        format_(f) {}
    text_item( text_item && rhs)
      : start(std::move(rhs.start)),
        end(std::move(rhs.end)),
        script(std::move(rhs.script)),
        dir(std::move(rhs.dir)),
        format_(rhs.format_) {}
    unsigned start; // First char (UTF16 offset)
    unsigned end; // Char _after_ the last char (UTF16 offset)
    UScriptCode script;
    UBiDiDirection dir;
    evaluated_format_properties_ptr const& format_;
};

// This class splits text into parts which all have the same
// - direction (LTR, RTL)
// - format
// - script (http://en.wikipedia.org/wiki/Scripts_in_Unicode)

class MAPNIK_DECL text_itemizer : util::noncopyable
{
public:
    text_itemizer();
    void add_text(value_unicode_string const& str, evaluated_format_properties_ptr const& format);
    std::list<text_item> const& itemize(unsigned start=0, unsigned end=0);
    void clear();
    value_unicode_string const& text() const { return text_; }
    // Returns the start and end position of a certain line.
    // Only forced line breaks with \n characters are handled here.
    std::pair<unsigned, unsigned> line(unsigned i) const;
    unsigned num_lines() const;
private:
    template<typename T> struct run : util::noncopyable
    {
        run(T const& _data, unsigned _start, unsigned _end)
            :  start(_start), end(_end), data(_data) {}
        unsigned start;
        unsigned end;
        T data;
    };
    using format_run_t = run<evaluated_format_properties_ptr const&>;
    using direction_run_t = run<UBiDiDirection>;
    using script_run_t = run<UScriptCode>;
    using format_run_list = std::list<format_run_t>;
    using script_run_list = std::list<script_run_t>;
    using direction_run_list = std::list<direction_run_t>;
    value_unicode_string text_;
    /// Format runs are always sorted by char index
    format_run_list format_runs_;
    /// Directions runs are always in visual order! This is different from
    /// format and script runs!
    direction_run_list direction_runs_;
    /// Script runs are always sorted by char index
    script_run_list script_runs_;
    void itemize_direction(unsigned start, unsigned end);
    void itemize_script();
    void create_item_list();
    std::list<text_item> output_;
    template <typename T> typename T::const_iterator find_run(T const& list, unsigned position);
    std::vector<unsigned> forced_line_breaks_; //Positions of \n characters
};
} //ns mapnik

#endif // TEXT_ITEMIZER_HPP
