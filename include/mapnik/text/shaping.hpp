#ifndef MAPNIK_TEXT_SHAPING_HPP
#define MAPNIK_TEXT_SHAPING_HPP

//ICU
#include <unicode/unistr.h>
class hb_font_t;
class hb_buffer_t;
class hb_glyph_info_t;

namespace mapnik
{

class text_shaping
{
public:
    //TODO: Get font file from font name
    text_shaping();
    ~text_shaping();

    uint32_t process_text(UnicodeString const& text);
    hb_buffer_t *get_buffer() { return buffer_; }

protected:
    static void free_data(void *data);

    void load_font();

    hb_font_t *font_;
    hb_buffer_t *buffer_;
};
} //ns mapnik

#endif // TEXT_SHAPING_HPP
