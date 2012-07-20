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
#include <mapnik/text/layout.hpp>
#include <mapnik/text/shaping.hpp>
#include <mapnik/text_properties.hpp>

//stl
#include <iostream>

// harf-buzz
#include <harfbuzz/hb.h>

// ICU
#include <unicode/brkiter.h>

/* TODO: Remove unused classes:
 * processed_text
 * string_info
 * text_path
 * char_info
 */

namespace mapnik
{
text_layout::text_layout(face_manager_freetype &font_manager)
    : font_manager_(font_manager)
{
}

void text_layout::break_lines(double break_width)
{
    if (total_width_ < break_width || !break_width) return;
    UnicodeString const& text = itemizer.get_text();
    Locale locale; //TODO: Is the default constructor correct?
    UErrorCode status = U_ZERO_ERROR;
    BreakIterator *breakitr = BreakIterator::createLineInstance(locale, status);
    //Not breaking the text if an error occurs is probably the best thing we can do.
    if (!U_SUCCESS(status)) return;
    breakitr->setText(text);
    unsigned current_line_length = 0;
    unsigned last_break_position = 0;
    for (unsigned i=0; i<text.length(); i++)
    {
        //TODO: Char spacing
        std::map<unsigned, double>::const_iterator width_itr = width_map.find(i);
        if (width_itr != width_map.end())
        {
            current_line_length += width_itr->second;
        }
        if (current_line_length > break_width)
        {
            unsigned break_position = breakitr->preceding(i);
            if (break_position <= last_break_position)
            {
                //A single word is longer than the maximum line width.
                //Violate line width requirement and choose next break position
                break_position = breakitr->following(i);
                std::cout << "Line overflow!\n";
            }
            //TODO: Add line
            std::cout << "Line to long ("<< current_line_length << ") at "<< i <<  " going to " << break_position << ". Last break was at " << last_break_position << "\n";
            last_break_position = break_position;
            i = break_position - 1;

            current_line_length = 0;
        }
    }

}

void text_layout::shape_text()
{
    UnicodeString const& text = itemizer.get_text();

    glyphs_.reserve(text.length()); //Preallocate memory

    total_width_ = 0.0;

    std::list<text_item> const& list = itemizer.itemize();
    std::list<text_item>::const_iterator itr = list.begin(), end = list.end();
    for (;itr!=end; itr++)
    {
        face_set_ptr face_set = font_manager_.get_face_set(itr->format->face_name, itr->format->fontset);
        face_set->set_character_sizes(itr->format->text_size);
        face_ptr face = *(face_set->begin()); //TODO: Implement font sets correctly
        text_shaping shaper(face->get_face()); //TODO: Make this more efficient by caching this object in font_face

        shaper.process_text(text, itr->start, itr->end, itr->rtl == UBIDI_RTL, itr->script);
        hb_buffer_t *buffer = shaper.get_buffer();

        unsigned num_glyphs = hb_buffer_get_length(buffer);

        hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer, NULL);
        hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer, NULL);

        std::string s;
//        std::cout << "Processing item '" << text.tempSubStringBetween(itr->start, itr->end).toUTF8String(s) << "' (" << uscript_getName(itr->script) << "," << itr->end - itr->start << "," << num_glyphs << "," << itr->rtl <<  ")\n";

        for (unsigned i=0; i<num_glyphs; i++)
        {
            glyph_info tmp;
            tmp.char_index = glyphs[i].cluster;
            tmp.glyph_index = glyphs[i].codepoint;
            tmp.width = positions[i].x_advance / 64.0;
            tmp.offset_x = positions[i].x_offset / 64.0;
            tmp.offset_y = positions[i].y_offset / 64.0;
            tmp.face = face;
            tmp.format = itr->format;
            face->glyph_dimensions(tmp);

            width_map[glyphs[i].cluster] += tmp.width;
            total_width_ += tmp.width;

            glyphs_.push_back(tmp);
//            std::cout << "glyph:" << glyphs[i].mask << " xa:" << positions[i].x_advance << " ya:" << positions[i].y_advance << " xo:" << positions[i].x_offset <<  " yo:" << positions[i].y_offset << "\n";
        }
    }
//    std::cout << "text_length: unicode chars: " << itemizer.get_text().length() << " glyphs: " << glyphs_.size()  << "\n";
//    std::vector<glyph_info>::const_iterator itr2 = glyphs_.begin(), end2 = glyphs_.end();
//    for (;itr2 != end2; itr2++)
//    {
//        std::cout << "'" << (char) itemizer.get_text().charAt(itr2->char_index) <<
//                 "' glyph codepoint:" << itr2->glyph_index <<
//                 " cluster: " << itr2->char_index <<
//                 " width: "<< itr2->width <<
//                 " height: " << itr2->height() <<
//                 "\n";
//    }
}

void text_layout::clear()
{
    itemizer.clear();
    glyphs_.clear();
}

#if 0
format_run::format_run(char_properties_ptr properties, double text_height)
    : properties_(properties), glyphs_(), width_(0), text_height_(text_height), line_height_(0)
{
}

void format_run::add_glyph(const glyph_info &info)
{
    glyphs_.push_back(info);
    width_ += info.width;
    line_height_ = info.line_height; //Same value for all characters with the same format
}

text_line::text_line()
    : max_line_height(0), max_text_height(0)
{
}

void text_line::add_run(format_run_ptr run)
{
    max_line_height = std::max(max_line_height, run->line_height());
    max_text_height = std::max(max_text_height, run->text_height());
    runs_.push_back(run);
}
#endif

} //ns mapnik
