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
//mapnik
#include <mapnik/text/itemizer.hpp>
#include <mapnik/text/scrptrun.hpp>
#include <mapnik/debug.hpp>

// stl
#include <algorithm>
#include <cassert>

namespace mapnik
{

text_itemizer::text_itemizer()
    : text_(), format_runs_(), direction_runs_(), script_runs_()
{
    forced_line_breaks_.push_back(0);
}

void text_itemizer::add_text(mapnik::value_unicode_string const& str, evaluated_format_properties_ptr const& format)
{
    unsigned start = text_.length();
    text_ += str;
    format_runs_.emplace_back(format, start, text_.length());

    while ((start = text_.indexOf('\n', start)+1) > 0)
    {
        forced_line_breaks_.push_back(start);
    }
}

std::list<text_item> const& text_itemizer::itemize(unsigned start, unsigned end)
{
    if (end == 0) {
        end = text_.length();
    }
    // format itemiziation is done by add_text()
    itemize_direction(start, end);
    itemize_script();
    create_item_list();
    return output_;
}

void text_itemizer::clear()
{
    output_.clear();
    text_.remove();
    format_runs_.clear();
    forced_line_breaks_.clear();
    forced_line_breaks_.push_back(0);
}

std::pair<unsigned, unsigned> text_itemizer::line(unsigned i) const
{
#ifdef MAPNIK_DEBUG
    if (i >= forced_line_breaks_.size()) return std::make_pair(0, 0);
#endif
    if (i == forced_line_breaks_.size()-1)
    {
        return std::make_pair(forced_line_breaks_[i], text_.length());
    }
    //Note -1 offset to exclude the \n char
    return std::make_pair(forced_line_breaks_[i], forced_line_breaks_[i+1]-1);
}

unsigned text_itemizer::num_lines() const
{
    return forced_line_breaks_.size();
}

void text_itemizer::itemize_direction(unsigned start, unsigned end)
{
    direction_runs_.clear();
    UErrorCode error = U_ZERO_ERROR;
    int32_t length = end - start;
    UBiDi *bidi = ubidi_openSized(length, 0, &error);
    if (!bidi || U_FAILURE(error))
    {
        MAPNIK_LOG_ERROR(text_itemizer) << "Failed to create bidi object: " << u_errorName(error) << "\n";
        return;
    }
    ubidi_setPara(bidi, text_.getBuffer() + start, length, UBIDI_DEFAULT_LTR, 0, &error);
    if (U_SUCCESS(error))
    {
        UBiDiDirection direction = ubidi_getDirection(bidi);
        if (direction != UBIDI_MIXED)
        {
            direction_runs_.emplace_back(direction, start, end);
        }
        else
        {
            // mixed-directional
            int32_t count = ubidi_countRuns(bidi, &error);
            if(U_SUCCESS(error))
            {
                for(int i=0; i<count; ++i)
                {
                    int32_t vis_length;
                    int32_t run_start;
                    direction = ubidi_getVisualRun(bidi, i, &run_start, &vis_length);
                    run_start += start; //Add offset to compensate offset in setPara
                    direction_runs_.emplace_back(direction, run_start, run_start+vis_length);
                }
            }
        }
    }
    else
    {
        MAPNIK_LOG_ERROR(text_itemizer) << "ICU error: " << u_errorName(error) << "\n"; //TODO: Exception
    }
    ubidi_close(bidi);
}

void text_itemizer::itemize_script()
{
    script_runs_.clear();

    ScriptRun runs(text_.getBuffer(), text_.length());
    while (runs.next())
    {
        script_runs_.emplace_back(runs.getScriptCode(), runs.getScriptStart(), runs.getScriptEnd());
    }
}

template <typename T>
typename T::const_iterator text_itemizer::find_run(T const& list, unsigned position)
{
    typename T::const_iterator itr = list.begin(), end = list.end();
    for ( ;itr!=end; ++itr)
    {
        // end is the first character not included in text range!
        if (itr->start <= position && itr->end > position) return itr;
    }
    return itr;
}

void text_itemizer::create_item_list()
{
    /* This function iterates over direction runs in visual order and splits them if neccessary.
     * Split RTL runs are processed in reverse order to keep glyphs in correct order.
     *
     * logical      123 | 456789
     * LTR visual   123 | 456789
     * RTL visual   987654 | 321
     * Glyphs within a single run are reversed by the shaper.
     */
    output_.clear();
    for (auto const& dir_run : direction_runs_)
    {
        unsigned position = dir_run.start;
        unsigned end = dir_run.end;
        std::list<text_item>::iterator rtl_insertion_point = output_.end();
        // Find first script and format run
        format_run_list::const_iterator format_itr = find_run(format_runs_, position);
        script_run_list::const_iterator script_itr = find_run(script_runs_, position);
        while (position < end)
        {
            assert(script_itr != script_runs_.end());
            assert(format_itr != format_runs_.end());
            unsigned start = position;
            position = std::min(script_itr->end, std::min(format_itr->end, end));
            if (dir_run.data == UBIDI_LTR)
            {
                output_.emplace_back(start,position,script_itr->data,dir_run.data,format_itr->data);
            }
            else
            {
                rtl_insertion_point = output_.emplace(rtl_insertion_point,start,position,script_itr->data,dir_run.data,format_itr->data);
            }
            if (script_itr->end == position) ++script_itr;
            if (format_itr->end == position) ++format_itr;
        }
    }
}
} //ns mapnik
