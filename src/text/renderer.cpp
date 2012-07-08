#include <mapnik/text/renderer.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/grid/grid.hpp>

namespace mapnik
{

template <typename T>
text_renderer<T>::text_renderer (pixmap_type & pixmap, face_manager<freetype_engine> &font_manager_, stroker & s, composite_mode_e comp_op)
    : pixmap_(pixmap),
      font_manager_(font_manager_),
      stroker_(s),
      comp_op_(comp_op) {}

#if 0
template <typename T>
box2d<double> text_renderer<T>::prepare_glyphs(text_path *path)
{
    //clear glyphs
    glyphs_.clear();

    FT_Matrix matrix;
    FT_Vector pen;
    FT_Error  error;

    FT_BBox bbox;
    bbox.xMin = bbox.yMin = 32000;  // Initialize these so we can tell if we
    bbox.xMax = bbox.yMax = -32000; // properly grew the bbox later

    for (int i = 0; i < path->num_nodes(); i++)
    {
        char_info_ptr c;
        double x, y, angle;

        path->vertex(&c, &x, &y, &angle);

        // TODO Enable when we have support for setting verbosity
        // MAPNIK_LOG_DEBUG(font_engine_freetype) << "text_renderer: prepare_glyphs="
        //                                        << c << "," << x << "," << y << "," << angle;

        FT_BBox glyph_bbox;
        FT_Glyph image;

        pen.x = int(x * 64);
        pen.y = int(y * 64);

        face_set_ptr faces = font_manager_.get_face_set(c->format->face_name, c->format->fontset);
        faces->set_character_sizes(c->format->text_size);

        glyph_ptr glyph = faces->get_glyph(unsigned(c->c));
        FT_Face face = glyph->get_face()->get_face();

        matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
        matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
        matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
        matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

        FT_Set_Transform(face, &matrix, &pen);

        error = FT_Load_Glyph(face, glyph->get_index(), FT_LOAD_NO_HINTING);
        if ( error )
            continue;

        error = FT_Get_Glyph(face->glyph, &image);
        if ( error )
            continue;

        FT_Glyph_Get_CBox(image,ft_glyph_bbox_pixels, &glyph_bbox);
        if (glyph_bbox.xMin < bbox.xMin)
            bbox.xMin = glyph_bbox.xMin;
        if (glyph_bbox.yMin < bbox.yMin)
            bbox.yMin = glyph_bbox.yMin;
        if (glyph_bbox.xMax > bbox.xMax)
            bbox.xMax = glyph_bbox.xMax;
        if (glyph_bbox.yMax > bbox.yMax)
            bbox.yMax = glyph_bbox.yMax;

        // Check if we properly grew the bbox
        if ( bbox.xMin > bbox.xMax )
        {
            bbox.xMin = 0;
            bbox.yMin = 0;
            bbox.xMax = 0;
            bbox.yMax = 0;
        }

        // take ownership of the glyph
        glyphs_.push_back(new glyph_t(image, c->format));
    }

    return box2d<double>(bbox.xMin, bbox.yMin, bbox.xMax, bbox.yMax);
}
#endif

template <typename T>
void composite_bitmap(T & pixmap, FT_Bitmap *bitmap, unsigned rgba, int x, int y, double opacity, composite_mode_e comp_op)
{
#if 0
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
#endif
}

template <typename T>
void text_renderer<T>::render(glyph_positions_ptr pos)
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
        double halo_radius = itr->properties->halo_radius;
        //make sure we've got reasonable values.
        if (halo_radius <= 0.0 || halo_radius > 1024.0) continue;
        stroker_.init(halo_radius);
        FT_Glyph g;
        error = FT_Glyph_Copy(itr->image, &g);
        if (!error)
        {
            FT_Glyph_Transform(g,0,&start);
            FT_Glyph_Stroke(&g,stroker_.get(),1);
            error = FT_Glyph_To_Bitmap( &g,FT_RENDER_MODE_NORMAL,0,1);
            if ( ! error )
            {

                FT_BitmapGlyph bit = (FT_BitmapGlyph)g;
                composite_bitmap(pixmap_, &bit->bitmap, itr->properties->halo_fill.rgba(),
                                 bit->left,
                                 height - bit->top,
                                 itr->properties->text_opacity,
                                 comp_op_
                    );
            }
        }
        FT_Done_Glyph(g);
    }
    //render actual text
    for (itr = glyphs_.begin(); itr != glyphs_.end(); ++itr)
    {

        FT_Glyph_Transform(itr->image,0,&start);

        error = FT_Glyph_To_Bitmap( &(itr->image),FT_RENDER_MODE_NORMAL,0,1);
        if ( ! error )
        {

            FT_BitmapGlyph bit = (FT_BitmapGlyph)itr->image;
            //render_bitmap(&bit->bitmap, itr->properties->fill.rgba(),
            //              bit->left,
            //              height - bit->top, itr->properties->text_opacity);

            composite_bitmap(pixmap_, &bit->bitmap, itr->properties->fill.rgba(),
                             bit->left,
                             height - bit->top,
                             itr->properties->text_opacity,
                             comp_op_
                );
        }
    }
#endif
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

template <typename T>
void text_renderer<T>::render_bitmap(FT_Bitmap *bitmap, unsigned rgba, int x, int y, double opacity)
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
                pixmap_.blendPixel2(i, j, rgba, gray, opacity);
            }
        }
    }
#endif
}

template void text_renderer<image_32>::render(glyph_positions_ptr);
template text_renderer<image_32>::text_renderer(image_32&, face_manager<freetype_engine>&, stroker&, composite_mode_e);

template void text_renderer<grid>::render_id(int, pixel_position, double );
template text_renderer<grid>::text_renderer(grid&, face_manager<freetype_engine>&, stroker&, composite_mode_e);

}
