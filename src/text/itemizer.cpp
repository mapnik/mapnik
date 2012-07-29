/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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
#include <iostream>
#include <algorithm>

namespace mapnik
{

text_itemizer::text_itemizer() : text_(), format_runs_(), direction_runs_(), script_runs_()
{

}

void text_itemizer::add_text(UnicodeString str, char_properties_ptr format)
{
    unsigned start = text_.length();
    text_ += str;
    format_runs_.push_back(format_run_t(format, start, text_.length()));
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
        if(direction != UBIDI_MIXED)
        {
            direction_runs_.push_back(direction_run_t(direction, start, end));
        } else
        {
            // mixed-directional
            int32_t count = ubidi_countRuns(bidi, &error);
            if(U_SUCCESS(error))
            {
                for(int i=0; i<count; i++)
                {
                    int32_t length;
                    int32_t run_start;
                    direction = ubidi_getVisualRun(bidi, i, &run_start, &length);
                    run_start += start; //Add offset to compensate offset in setPara
                    std::cout << "visual run, rtl:" << direction
                              << " start:" << start << " length:" << length << "\n";
                    direction_runs_.push_back(direction_run_t(direction, start, start+length));
                }
            }
        }
    } else{
        MAPNIK_LOG_ERROR(text_itemizer) << "ICU error: " << u_errorName(error) << "\n"; //TODO: Exception
    }
    ubidi_close(bidi);
}

void text_itemizer::itemize_script()
{
    if (!script_runs_.empty()) return; //Already done

    ScriptRun runs(text_.getBuffer(), text_.length());
    while (runs.next())
    {
        script_runs_.push_back(script_run_t(runs.getScriptCode(), runs.getScriptStart(), runs.getScriptEnd()));
    }
}

template <typename T>
typename T::const_iterator text_itemizer::find_run(T const& list, unsigned position)
{
    typename T::const_iterator itr = list.begin(), end = list.end();
    for (;itr!=end; itr++)
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
    direction_run_list::const_iterator dir_itr = direction_runs_.begin(), dir_end = direction_runs_.end();
    for (; dir_itr != dir_end; dir_itr++)
    {
        unsigned position = dir_itr->start;
        unsigned end = dir_itr->end;
        std::list<text_item>::iterator rtl_insertion_point = output_.end();
        // Find first script and format run
        format_run_list::const_iterator format_itr = find_run(format_runs_, position);
        script_run_list::const_iterator script_itr = find_run(script_runs_, position);
        while (position < end)
        {
            assert(script_itr != script_runs_.end());
            assert(format_itr != format_runs_.end());
            text_item item;
            item.start = position;
            position = std::min(script_itr->end, std::min(format_itr->end, end));
            item.end = position;
            item.format = format_itr->data;
            item.script = script_itr->data;
            item.rtl = dir_itr->data;
            if (dir_itr->data == UBIDI_LTR)
            {
                output_.push_back(item);
            } else
            {
                rtl_insertion_point = output_.insert(rtl_insertion_point, item);
            }
            if (script_itr->end == position) script_itr++;
            if (format_itr->end == position) format_itr++;
        }
    }
}
} //ns mapnik
