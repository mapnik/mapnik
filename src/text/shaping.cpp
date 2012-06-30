#include <mapnik/text/shaping.hpp>


//stl
#include <iostream>
#include <fstream>

//harf-buzz
#define HAVE_FREETYPE
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb-icu.h>


namespace mapnik
{


text_shaping::text_shaping(FT_Face face)
    : font_(0),
      buffer_ (hb_buffer_create()),
      face_(face)
{
    hb_buffer_set_unicode_funcs(buffer_, hb_icu_get_unicode_funcs());
    load_font();
}

text_shaping::~text_shaping()
{
    hb_buffer_destroy(buffer_);
    hb_font_destroy(font_);
}

uint32_t text_shaping::process_text(UnicodeString const& text, bool rtl, UScriptCode script)
{
    if (!font_) return 0;
    hb_buffer_reset(buffer_);

    uint32_t length = text.length();

    hb_buffer_add_utf16(buffer_, text.getBuffer(), length, 0, -1);
    hb_buffer_set_direction(buffer_, rtl?HB_DIRECTION_RTL:HB_DIRECTION_LTR);
    hb_buffer_set_script(buffer_, hb_icu_script_to_script(script));
#if 0
    hb_buffer_set_language(buffer, hb_language_from_string (language, -1));
#endif
    hb_shape(font_, buffer_, 0 /*features*/, 0 /*num_features*/);
    return length;
}

void text_shaping::free_data(void *data)
{
    char *tmp = (char *)data;
    delete [] tmp;
}

void text_shaping::load_font()
{
    if (font_) return;
    font_ = hb_ft_font_create(face_, NULL);
}

}
