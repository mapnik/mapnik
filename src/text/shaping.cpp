#include <mapnik/text/shaping.hpp>


//stl
#include <iostream>
#include <fstream>

//harf-buzz
#define HAVE_FREETYPE
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>


namespace mapnik
{


text_shaping::text_shaping(FT_Face face)
    : font_(0),
      buffer_ (hb_buffer_create()),
      face_(face)
{
    load_font();
}

text_shaping::~text_shaping()
{
    hb_buffer_destroy(buffer_);
    hb_font_destroy(font_);
}

uint32_t text_shaping::process_text(const UnicodeString &text)
{
    if (!font_) return 0;
    hb_buffer_reset(buffer_);

    std::string s;
    text.toUTF8String(s);
    hb_buffer_add_utf8(buffer_, s.c_str(), s.length(), 0, -1);
#if 0
    hb_buffer_set_direction(buffer, hb_direction_from_string (direction, -1));
    hb_buffer_set_script(buffer, hb_script_from_string (script, -1));
    hb_buffer_set_language(buffer, hb_language_from_string (language, -1));
#endif
    hb_shape(font_, buffer_, 0 /*features*/, 0 /*num_features*/);
    return s.length();
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
