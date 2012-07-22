#include <mapnik/text/renderer.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/text_properties.hpp>

namespace mapnik
{

template <typename T>
text_renderer<T>::text_renderer (pixmap_type & pixmap, face_manager<freetype_engine> &font_manager_, composite_mode_e comp_op, double scale_factor)
    : pixmap_(pixmap),
      font_manager_(font_manager_),
      stroker_(*(font_manager_.get_stroker())),
      comp_op_(comp_op),
      scale_factor_(scale_factor)
{}


template <typename T>
void text_renderer<T>::prepare_glyphs(glyph_positions_ptr pos)
{
    //clear glyphs
    glyphs_.clear();

    FT_Matrix matrix;
    FT_Vector pen;
    FT_Error  error;

    bool constant_angle = pos->is_constant_angle();
    if (constant_angle)
    {
        double angle = pos->get_angle();
        double cosa = cos(angle);
        double sina = sin(angle);
        matrix.xx = (FT_Fixed)( cosa * 0x10000L);
        matrix.xy = (FT_Fixed)(-sina * 0x10000L);
        matrix.yx = (FT_Fixed)( sina * 0x10000L);
        matrix.yy = (FT_Fixed)( cosa * 0x10000L);
    }
    glyph_positions::const_iterator itr = pos->begin(), end = pos->end();
    for (; itr != end; itr++)
    {
        glyph_info const& glyph = *(itr->glyph);
        glyph.face->set_character_sizes(glyph.format->text_size * scale_factor_); //TODO: Optimize this?

        pen.x = int((itr->pos.x + glyph.offset_x) * 64);
        pen.y = int((itr->pos.y + glyph.offset_y) * 64);

        if (!constant_angle)
        {
            double angle = itr->angle;
            double cosa = cos(angle);
            double sina = sin(angle);
            matrix.xx = (FT_Fixed)( cosa * 0x10000L);
            matrix.xy = (FT_Fixed)(-sina * 0x10000L);
            matrix.yx = (FT_Fixed)( sina * 0x10000L);
            matrix.yy = (FT_Fixed)( cosa * 0x10000L);
        }

        FT_Face face = glyph.face->get_face();
        FT_Set_Transform(face, &matrix, &pen);

        error = FT_Load_Glyph(face, glyph.glyph_index, FT_LOAD_NO_HINTING);
        if (error) continue;

        FT_Glyph image;
        error = FT_Get_Glyph(face->glyph, &image);
        if (error) continue;

        // take ownership of the glyph
        glyphs_.push_back(new glyph_t(image, glyph.format));
    }
}

template <typename T>
void composite_bitmap(T & pixmap, FT_Bitmap *bitmap, unsigned rgba, int x, int y, double opacity, composite_mode_e comp_op)
{
    int x_max=x+bitmap->width;
    int y_max=y+bitmap->rows;
    int i,p,j,q;

    for (i=x,p=0;i<x_max;++i,++p)
    {
        for (j=y,q=0;j<y_max;++j,++q)
        {
            unsigned gray=bitmap->buffer[q*bitmap->width+p];
            if (gray)
            {
                pixmap.composite_pixel(comp_op, i, j, rgba, gray, opacity);
            }
        }
    }
}

template <typename T>
void text_renderer<T>::render(glyph_positions_ptr pos)
{
    if (glyphs_.empty()) prepare_glyphs(pos);
    FT_Error  error;
    FT_Vector start;
    unsigned height = pixmap_.height();
    pixel_position const& base_point = pos->get_base_point();

    start.x =  static_cast<FT_Pos>(base_point.x * (1 << 6));
    start.y =  static_cast<FT_Pos>((height - base_point.y) * (1 << 6)); //TODO: Why is this inverted coordinate system used?

    //render halo
    typename boost::ptr_vector<glyph_t>::iterator itr;
    double halo_radius = 0;
    char_properties_ptr format;
    for (itr = glyphs_.begin(); itr != glyphs_.end(); ++itr)
    {
        if (itr->properties)
        {
            format = itr->properties;
            /* Settings have changed. */
            halo_radius = itr->properties->halo_radius;
            //make sure we've got reasonable values.
            if (halo_radius <= 0.0 || halo_radius > 1024.0) break;
            stroker_.init(halo_radius);
        }
        FT_Glyph g;
        error = FT_Glyph_Copy(itr->image, &g);
        if (!error)
        {
            FT_Glyph_Transform(g, 0, &start);
            FT_Glyph_Stroke(&g, stroker_.get(), 1);
            error = FT_Glyph_To_Bitmap(&g, FT_RENDER_MODE_NORMAL, 0, 1);
            if (!error)
            {
                FT_BitmapGlyph bit = (FT_BitmapGlyph)g;
                composite_bitmap(pixmap_, &bit->bitmap, format->halo_fill.rgba(),
                                 bit->left,
                                 height - bit->top,
                                 format->text_opacity,
                                 comp_op_);
            }
        }
        FT_Done_Glyph(g);
    }

    //render actual text
    for (itr = glyphs_.begin(); itr != glyphs_.end(); ++itr)
    {
        if (itr->properties)
        {
            format = itr->properties;
        }
        FT_Glyph_Transform(itr->image, 0, &start);

        error = FT_Glyph_To_Bitmap( &(itr->image),FT_RENDER_MODE_NORMAL,0,1);
        if (!error)
        {

            FT_BitmapGlyph bit = (FT_BitmapGlyph)itr->image;
            composite_bitmap(pixmap_, &bit->bitmap, format->fill.rgba(),
                             bit->left,
                             height - bit->top,
                             format->text_opacity,
                             comp_op_
                );
        }
    }
}


template <typename T>
void text_renderer<T>::render_id(int feature_id, pixel_position pos, double min_radius)
{
#if 0
    FT_Error  error;
    FT_Vector start;
    unsigned height = pixmap_.height();

    start.x =  static_cast<FT_Pos>(pos.x * (1 << 6));
    start.y =  static_cast<FT_Pos>((height - pos.y) * (1 << 6));

    // now render transformed glyphs
    typename glyphs_t::iterator itr;
    for (itr = glyphs_.begin(); itr != glyphs_.end(); ++itr)
    {
        stroker_.init(std::max(itr->properties->halo_radius, min_radius));
        FT_Glyph g;
        error = FT_Glyph_Copy(itr->image, &g);
        if (!error)
        {
            FT_Glyph_Transform(g,0,&start);
            FT_Glyph_Stroke(&g,stroker_.get(),1);
            error = FT_Glyph_To_Bitmap( &g,FT_RENDER_MODE_NORMAL,0,1);
            //error = FT_Glyph_To_Bitmap( &g,FT_RENDER_MODE_MONO,0,1);
            if ( ! error )
            {

                FT_BitmapGlyph bit = (FT_BitmapGlyph)g;
                render_bitmap_id(&bit->bitmap, feature_id,
                                 bit->left,
                                 height - bit->top);
            }
        }
        FT_Done_Glyph(g);
    }
#endif
}

template <typename T>
void text_renderer<T>::render_bitmap_id(FT_Bitmap *bitmap,int feature_id,int x,int y)
{
#if 0
    int x_max=x+bitmap->width;
    int y_max=y+bitmap->rows;
    int i,p,j,q;

    for (i=x,p=0;i<x_max;++i,++p)
    {
        for (j=y,q=0;j<y_max;++j,++q)
        {
            int gray=bitmap->buffer[q*bitmap->width+p];
            if (gray)
            {
                pixmap_.setPixel(i,j,feature_id);
                //pixmap_.blendPixel2(i,j,rgba,gray,opacity_);
            }
        }
    }
#endif
}

template class text_renderer<image_32>;
//template class text_renderer<grid>;
}
