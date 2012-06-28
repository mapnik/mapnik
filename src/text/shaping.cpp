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


text_shaping::text_shaping()
    : font_(0),
      buffer_ (hb_buffer_create())
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
    //TODO: hb_ft_font_create
    if (font_) return;

    char *font_data;
    unsigned int size;

//    std::ifstream file("./unifont-5.1.20080907.ttf" /*TODO*/, std::ios::in|std::ios::binary|std::ios::ate);
    std::ifstream file("./DejaVuSans.ttf" /*TODO*/, std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        font_data = new char[size];
        file.seekg(0, std::ios::beg);
        file.read(font_data, size);
        file.close();
    } else {
        std::cerr << "Could not open font!\n";
        return ;//TODO: Raise exception
    }


    hb_blob_t *blob = hb_blob_create(font_data, size, HB_MEMORY_MODE_WRITABLE, font_data, &free_data);
    hb_face_t *face = hb_face_create(blob, 0 /*face_index*/);
    hb_blob_destroy(blob);
    font_ = hb_font_create(face);
#if 1
    //TODO: Font size
    unsigned int upem = hb_face_get_upem(face);
    hb_font_set_scale(font_, upem, upem);
#endif
    hb_face_destroy(face);
    hb_ft_font_set_funcs(font_);
}

}
