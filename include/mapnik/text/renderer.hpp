#ifndef MAPNIK_TEXT_RENDERER_HPP
#define MAPNIK_TEXT_RENDERER_HPP

//mapnik
#include <mapnik/text/placement_finder.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/text_symbolizer.hpp>
//TODO: Find a better place for halo_rasterizer_e!
//TODO: Halo rasterizer selection should go to text_properties because it might make sense to use a different rasterizer for different fonts

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
    text_renderer (halo_rasterizer_e rasterizer, composite_mode_e comp_op = src_over, double scale_factor=1.0, stroker_ptr stroker=stroker_ptr());
protected:
    struct glyph_t : boost::noncopyable
    {
        FT_Glyph image;
        char_properties_ptr properties;

        glyph_t(FT_Glyph image_, char_properties_ptr properties_)
            : image(image_), properties(properties_) {}

        ~glyph_t () { FT_Done_Glyph(image);}

    };

    void prepare_glyphs(glyph_positions_ptr pos);

    halo_rasterizer_e rasterizer_;
    composite_mode_e comp_op_;
    double scale_factor_;
    boost::ptr_vector<glyph_t> glyphs_;
    stroker_ptr stroker_;

};

template <typename T>
class agg_text_renderer : public text_renderer
{
public:
    typedef T pixmap_type;
    agg_text_renderer (pixmap_type & pixmap, halo_rasterizer_e rasterizer,
                       composite_mode_e comp_op = src_over,
                       double scale_factor=1.0,
                       stroker_ptr stroker=stroker_ptr());
    void render(glyph_positions_ptr pos);
private:
    pixmap_type & pixmap_;
    void render_halo(FT_Bitmap *bitmap, unsigned rgba, int x, int y, int halo_radius, double opacity);
};

template <typename T>
class grid_text_renderer : public text_renderer
{
public:
    typedef T pixmap_type;
    grid_text_renderer (pixmap_type & pixmap, composite_mode_e comp_op = src_over,
                        double scale_factor=1.0);
    void render(glyph_positions_ptr pos, value_integer feature_id);
private:
    pixmap_type & pixmap_;
    void render_halo_id(FT_Bitmap *bitmap, mapnik::value_integer feature_id, int x, int y, int halo_radius);
};

}
#endif // RENDERER_HPP
