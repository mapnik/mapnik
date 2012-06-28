#include <mapnik/text/layout.hpp>
#include <mapnik/text/shaping.hpp>

//stl
#include <iostream>

// harf-buzz
#include <harfbuzz/hb.h>

namespace mapnik
{
text_layout::text_layout()
{
}

void text_layout::break_lines()
{
}

void text_layout::shape_text()
{
    glyphs_.reserve(itemizer.get_text().length()); //Preallocate memory
    uint32_t byte_offset = 0;
    std::list<text_item> const& list = itemizer.itemize();
    std::list<text_item>::const_iterator itr = list.begin(), end = list.end();
    for (;itr!=end; itr++)
    {
        text_shaping shaper;
        uint32_t bytes = shaper.process_text(itr->str);
        hb_buffer_t *buffer = shaper.get_buffer();

        unsigned num_glyphs = hb_buffer_get_length(buffer);

        hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer, NULL);
        hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer, NULL);

        std::string s;
        std::cout << "Processing item '" << itr->str.toUTF8String(s) << "' (" << uscript_getName(itr->script) << "," << itr->str.length() << "," << num_glyphs << ")\n";

        for (unsigned i=0; i<num_glyphs; i++)
        {
            glyph_info tmp;
            tmp.byte_position = byte_offset + glyphs[i].cluster;
            tmp.codepoint = glyphs[i].codepoint;
            tmp.x_advance = positions[i].x_advance;
            glyphs_.push_back(tmp);
        }
        byte_offset += bytes;
    }
    std::string s;
    std::cout << "text_length: unicode chars: " << itemizer.get_text().length() << " bytes: " <<itemizer.get_text().toUTF8String(s).length() << " glyphs: " << glyphs_.size()  << "\n";
    std::vector<glyph_info>::const_iterator itr2 = glyphs_.begin(), end2 = glyphs_.end();
    for (;itr2 != end2; itr2++)
    {
        std::cout << "glyph codepoint:" << itr2->codepoint <<
                 " cluster: " << itr2->byte_position <<
                 " x_advance: "<< itr2->x_advance << "\n";
    }
}

void text_layout::clear()
{
    itemizer.clear();
    glyphs_.clear();
}

} //ns mapnik
