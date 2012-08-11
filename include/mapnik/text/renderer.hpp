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

class text_renderer : private boost::noncopyable
{
public:
    text_renderer (composite_mode_e comp_op = src_over, double scale_factor=1.0);
//    void render_id(int feature_id, pixel_position pos, double min_radius=1.0);
protected:
//    void render_bitmap_id(FT_Bitmap *bitmap,int feature_id,int x,int y);

    struct glyph_t : boost::noncopyable
    {
        FT_Glyph image;
        char_properties_ptr properties;

        glyph_t(FT_Glyph image_, char_properties_ptr properties_)
            : image(image_), properties(properties_) {}

        ~glyph_t () { FT_Done_Glyph(image);}

    };

    void prepare_glyphs(glyph_positions_ptr pos);

    composite_mode_e comp_op_;
    double scale_factor_;
    boost::ptr_vector<glyph_t> glyphs_;
};

template <typename T>
class agg_text_renderer : public text_renderer
{
public:
    typedef T pixmap_type;
    agg_text_renderer (pixmap_type & pixmap, stroker &stroker,
                       composite_mode_e comp_op = src_over, double scale_factor=1.0);
    void render(glyph_positions_ptr pos);
private:
    pixmap_type & pixmap_;
    stroker & stroker_;
};

}
#endif // RENDERER_HPP
