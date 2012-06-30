#include <mapnik/text/layout.hpp>
#include <mapnik/text/shaping.hpp>

//stl
#include <iostream>

// harf-buzz
#include <harfbuzz/hb.h>

namespace mapnik
{
text_layout::text_layout(face_manager_freetype &font_manager)
    : font_manager_(font_manager)
{
}

void text_layout::break_lines()
{
}

void text_layout::shape_text()
{
    glyphs_.reserve(itemizer.get_text().length()); //Preallocate memory
    uint32_t offset = 0; //in utf16 code points
    std::list<text_item> const& list = itemizer.itemize();
    std::list<text_item>::const_iterator itr = list.begin(), end = list.end();
    for (;itr!=end; itr++)
    {
        face_set_ptr face_set = font_manager_.get_face_set(itr->format.face_name, itr->format.fontset);
        face_set->set_character_sizes(itr->format.text_size);
        face_ptr face = *(face_set->begin()); //TODO: Implement font sets correctly
        text_shaping shaper(face->get_face()); //TODO: Make this more efficient by caching this object in font_face

        uint32_t chars = shaper.process_text(itr->str, itr->rtl == UBIDI_DEFAULT_RTL, itr->script);
        hb_buffer_t *buffer = shaper.get_buffer();

        unsigned num_glyphs = hb_buffer_get_length(buffer);

        hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer, NULL);
        hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer, NULL);

        std::string s;
        std::cout << "Processing item '" << itr->str.toUTF8String(s) << "' (" << uscript_getName(itr->script) << "," << itr->str.length() << "," << num_glyphs << "," << itr->rtl <<  ")\n";

        for (unsigned i=0; i<num_glyphs; i++)
        {
            glyph_info tmp;
            tmp.char_index = offset + glyphs[i].cluster;
            tmp.glyph.index = glyphs[i].codepoint;
            tmp.glyph.face = face;
            tmp.x_advance = positions[i].x_advance;
            glyphs_.push_back(tmp);
        }
        offset += chars;
    }
    std::cout << "text_length: unicode chars: " << itemizer.get_text().length() << " glyphs: " << glyphs_.size()  << "\n";
    std::vector<glyph_info>::const_iterator itr2 = glyphs_.begin(), end2 = glyphs_.end();
    for (;itr2 != end2; itr2++)
    {
        std::cout << "'" << (char) itemizer.get_text().charAt(itr2->char_index) <<
                 "' glyph codepoint:" << itr2->glyph.index <<
                 " cluster: " << itr2->char_index <<
                 " x_advance: "<< itr2->x_advance/64.0 << "\n";
    }
}

void text_layout::clear()
{
    itemizer.clear();
    glyphs_.clear();
}

} //ns mapnik
