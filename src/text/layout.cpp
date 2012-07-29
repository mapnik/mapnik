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

// boost
#include <boost/make_shared.hpp>

namespace mapnik
{
text_layout::text_layout(face_manager_freetype &font_manager)
    : font_manager_(font_manager), itemizer_(), width_(0), height_(0), lines_()
{
}

void text_layout::layout(double wrap_width, unsigned text_ratio)
{
    text_line_ptr line = boost::make_shared<text_line>(0, itemizer_.get_text().length());
    shape_text(line, 0, itemizer_.get_text().length()); //Process full text
    break_line(line, wrap_width, text_ratio); //Break line if neccessary
}


void text_layout::break_line(text_line_ptr line, double wrap_width, unsigned text_ratio)
{
    //TODO: Handle \n chars.
    if (!wrap_width || line->width() < wrap_width)
    {
        add_line(line);
        return;

    }
    if (text_ratio)
    {
        double wrap_at;
        double string_width = line->width();
        double string_height = line->line_height();
        for (double i = 1.0; ((wrap_at = string_width/i)/(string_height*i)) > text_ratio && (string_width/i) > wrap_width; i += 1.0) ;
    }

    UnicodeString const& text = itemizer_.get_text();
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
        std::map<unsigned, double>::const_iterator width_itr = width_map_.find(i);
        if (width_itr != width_map_.end())
        {
            current_line_length += width_itr->second;
        }
        if (current_line_length > wrap_width)
        {
            unsigned break_position = breakitr->preceding(i);
            if (break_position <= last_break_position)
            {
                //A single word is longer than the maximum line width.
                //Violate line width requirement and choose next break position
                break_position = breakitr->following(i);
                std::cout << "Line overflow!\n";
            }
            std::cout << "Line to long ("<< current_line_length << ") at "<< i <<  " going to " << break_position << ". Last break was at " << last_break_position << "\n";
            text_line_ptr new_line = boost::make_shared<text_line>(last_break_position, break_position);
            shape_text(new_line, last_break_position, break_position);
            add_line(new_line);
            last_break_position = break_position;
            i = break_position - 1;

            current_line_length = 0;
        }
    }
}

void text_layout::shape_text(text_line_ptr line, unsigned start, unsigned end)
{
    UnicodeString const& text = itemizer_.get_text();

    size_t length = end - start;

    line->reserve(length); //Preallocate memory

    std::list<text_item> const& list = itemizer_.itemize(start, end);
    std::list<text_item>::const_iterator itr = list.begin(), list_end = list.end();
    for (; itr != list_end; itr++)
    {
        face_set_ptr face_set = font_manager_.get_face_set(itr->format->face_name, itr->format->fontset);
        face_set->set_character_sizes(itr->format->text_size);
        face_ptr face = *(face_set->begin()); //TODO: Implement font sets correctly
        text_shaping shaper(face->get_face()); //TODO: Make this more efficient by caching this object in font_face

        line->update_max_char_height(face->get_char_height());

        shaper.process_text(text, itr->start, itr->end, itr->rtl == UBIDI_RTL, itr->script);
        hb_buffer_t *buffer = shaper.get_buffer();

        unsigned num_glyphs = hb_buffer_get_length(buffer);

        hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer, NULL);
        hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer, NULL);

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

            width_map_[glyphs[i].cluster] += tmp.width;

            line->add_glyph(tmp);
//            std::cout << "glyph:" << glyphs[i].mask << " xa:" << positions[i].x_advance << " ya:" << positions[i].y_advance << " xo:" << positions[i].x_offset <<  " yo:" << positions[i].y_offset << "\n";
        }
    }
}

void text_layout::add_line(text_line_ptr line)
{
    if (lines_.empty())
    {
        line->set_first_line(true);
    }
    lines_.push_back(line);
    width_ = std::max(width_, line->width());
    height_ += line->height();
}

void text_layout::clear()
{
    itemizer_.clear();
    lines_.clear();
}

double text_layout::height() const
{
    return height_; //TODO
}

double text_layout::width() const
{
    return width_; //TODO
}

text_layout::const_iterator text_layout::begin() const
{
    return lines_.begin();
}

text_layout::const_iterator text_layout::end() const
{
    return lines_.end();
}

unsigned text_layout::size() const
{
    return lines_.size();
}

/*********************************************************************************************/

text_line::text_line(unsigned first_char, unsigned last_char)
    : glyphs_(), line_height_(0.), max_char_height_(0.),
      width_(0.), first_char_(first_char), last_char_(last_char),
      first_line_(false)
{
}

void text_line::add_glyph(const glyph_info &glyph)
{
    line_height_ = std::max(line_height_, glyph.line_height + glyph.format->line_spacing);
    if (glyphs_.empty())
    {
        width_ = glyph.width;
    } else {
        width_ += glyph.width + glyphs_.back().format->character_spacing;
    }
    glyphs_.push_back(glyph);
}


void text_line::reserve(glyph_vector::size_type length)
{
    glyphs_.reserve(length);
}

text_line::const_iterator text_line::begin() const
{
    return glyphs_.begin();
}

text_line::const_iterator text_line::end() const
{
    return glyphs_.end();
}

double text_line::height() const
{
    if (first_line_) return max_char_height_;
    return line_height_;
}

void text_line::update_max_char_height(double max_char_height)
{
    max_char_height_ = std::max(max_char_height_, max_char_height);
}

void text_line::set_first_line(bool first_line)
{
    first_line_ = first_line;
}

} //ns mapnik
