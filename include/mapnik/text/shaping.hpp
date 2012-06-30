#ifndef MAPNIK_TEXT_SHAPING_HPP
#define MAPNIK_TEXT_SHAPING_HPP

//ICU
#include <unicode/unistr.h>
#include <unicode/uscript.h>
#include <harfbuzz/hb.h>

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
}

namespace mapnik
{

class text_shaping
{
public:
    //TODO: Get font file from font name
    text_shaping(FT_Face face);
    ~text_shaping();

    uint32_t process_text(UnicodeString const& text, bool rtl, UScriptCode script);
    hb_buffer_t *get_buffer() { return buffer_; }

protected:
    static void free_data(void *data);

    void load_font();

    hb_font_t *font_;
    hb_buffer_t *buffer_;
    FT_Face face_;
};
} //ns mapnik

#endif // TEXT_SHAPING_HPP
