#ifndef MAPNIK_TEXT_LAYOUT_HPP
#define MAPNIK_TEXT_LAYOUT_HPP

//mapnik
#include <mapnik/text/itemizer.hpp>
#include <mapnik/font_engine_freetype.hpp>

//stl
#include <vector>

namespace mapnik
{

struct glyph_info
{
      font_glyph glyph;
      uint32_t char_index; //Position in the string of all characters i.e. before itemizing
      uint32_t x_advance;
};

class text_layout
{
public:
    text_layout(face_manager_freetype & font_manager);
    inline void add_text(UnicodeString const& str, char_properties const& format)
    {
        itemizer.add_text(str, format);
    }

    void break_lines();
    void shape_text();
    void clear();

private:
    text_itemizer itemizer;
    std::vector<glyph_info> glyphs_;
    face_manager_freetype &font_manager_;
};
}

#endif // TEXT_LAYOUT_HPP
