#ifndef MAPNIK_TEXT_LAYOUT_HPP
#define MAPNIK_TEXT_LAYOUT_HPP

//mapnik
#include <mapnik/text/itemizer.hpp>

//stl
#include <vector>

namespace mapnik
{

struct glyph_info
{
      uint32_t codepoint;
      uint32_t byte_position;
      uint32_t x_advance;
};

class text_layout
{
public:
    text_layout(double text_ratio, double wrap_width);
    inline void add_text(UnicodeString const& str, char_properties const& format)
    {
        itemizer.add_text(str, format);
    }

    void break_lines();
    void shape_text();


private:
    text_itemizer itemizer;
    double text_ratio_;
    double wrap_width_;
    std::vector<glyph_info> glyphs_;
};
}

#endif // TEXT_LAYOUT_HPP
