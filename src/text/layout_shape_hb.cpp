//mapnik
#include <mapnik/text/layout.hpp>
#include <mapnik/text/shaping.hpp>
#include <mapnik/text/text_properties.hpp>

//stl
#include <list>

// harf-buzz
#include <harfbuzz/hb.h>

namespace mapnik
{


void text_layout::shape_text(text_line_ptr line)
{
    unsigned start = line->get_first_char();
    unsigned end = line->get_last_char();
    UnicodeString const& text = itemizer_.get_text();

    size_t length = end - start;

    line->reserve(length); //Preallocate memory

    std::list<text_item> const& list = itemizer_.itemize(start, end);
    std::list<text_item>::const_iterator itr = list.begin(), list_end = list.end();
    for (; itr != list_end; itr++)
    {
        face_set_ptr face_set = font_manager_.get_face_set(itr->format->face_name, itr->format->fontset);
        face_set->set_character_sizes(itr->format->text_size);
        font_face_set::iterator face_itr = face_set->begin(), face_end = face_set->end();
        for (; face_itr != face_end; face_itr++)
        {
            face_ptr face = *face_itr;
            text_shaping shaper(face->get_face()); //TODO: Make this more efficient by caching this object in font_face

            shaper.process_text(text, itr->start, itr->end, itr->rtl == UBIDI_RTL, itr->script);
            hb_buffer_t *buffer = shaper.get_buffer();

            unsigned num_glyphs = hb_buffer_get_length(buffer);

            hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer, NULL);
            hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer, NULL);

            bool font_has_all_glyphs = true;
            /* Check if all glyphs are valid. */
            for (unsigned i=0; i<num_glyphs; i++)
            {
                if (!glyphs[i].codepoint)
                {
                    font_has_all_glyphs = false;
                    break;
                }
            }
            if (!font_has_all_glyphs)
            {
                //Try next font in fontset
                continue;
            }

            for (unsigned i=0; i<num_glyphs; i++)
            {
                glyph_info tmp;
                tmp.char_index = glyphs[i].cluster;
                tmp.glyph_index = glyphs[i].codepoint;
                tmp.width = positions[i].x_advance / 64.0;
                tmp.offset.set(positions[i].x_offset / 64.0, positions[i].y_offset / 64.0);
                tmp.face = face;
                tmp.format = itr->format;
                face->glyph_dimensions(tmp);

                width_map_[glyphs[i].cluster] += tmp.width;

                line->add_glyph(tmp);
            }
            line->update_max_char_height(face->get_char_height());
            break; //When we reach this point the current font had all glyphs.
        }
    }
}

} //ns mapnik
