#ifndef MAPNIK_TEXT_RENDERER_HPP
#define MAPNIK_TEXT_RENDERER_HPP

//mapnik
#include <mapnik/text/placement_finder_ng.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/font_engine_freetype.hpp>

//boost
#include <boost/utility.hpp>


// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_STROKER_H
}

namespace mapnik
{

template <typename T>
class text_renderer : private boost::noncopyable
{
public:
    typedef T pixmap_type;

    text_renderer (pixmap_type & pixmap, face_manager<freetype_engine> &font_manager_, composite_mode_e comp_op = src_over);
    void render(glyph_positions_ptr pos);
    void render_id(int feature_id, pixel_position pos, double min_radius=1.0);
private:
    void render_bitmap_id(FT_Bitmap *bitmap,int feature_id,int x,int y);

    struct glyph_t : boost::noncopyable
    {
        FT_Glyph image;
        char_properties_ptr properties;

        glyph_t(FT_Glyph image_, char_properties_ptr properties_)
            : image(image_), properties(properties_) {}

        ~glyph_t () { FT_Done_Glyph(image);}

    };

    void prepare_glyphs(glyph_positions_ptr pos);

    pixmap_type & pixmap_;
    face_manager<freetype_engine> &font_manager_;
    stroker & stroker_;
    composite_mode_e comp_op_;
    boost::ptr_vector<glyph_t> glyphs_;
};
}
#endif // RENDERER_HPP
