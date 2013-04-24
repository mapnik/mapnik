/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text_properties.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/text_path.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/font_util.hpp>


// boost
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>

// stl
#include <sstream>
#include <algorithm>

// icu
#include <unicode/ubidi.h>
#include <unicode/ushape.h>
#include <unicode/schriter.h>
#include <unicode/uversion.h>

namespace mapnik
{


freetype_engine::freetype_engine() :
    library_(NULL)

{
    FT_Error error = FT_Init_FreeType( &library_ );
    if (error)
    {
        throw std::runtime_error("can not load FreeType2 library");
    }
}

freetype_engine::~freetype_engine()
{
    FT_Done_FreeType(library_);
}

bool freetype_engine::is_font_file(std::string const& file_name)
{
    // only accept files that will be matched by freetype2's `figurefiletype()`
    std::string fn = file_name;
    std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
    return boost::algorithm::ends_with(fn,std::string(".ttf")) ||
        boost::algorithm::ends_with(fn,std::string(".otf")) ||
        boost::algorithm::ends_with(fn,std::string(".ttc")) ||
        boost::algorithm::ends_with(fn,std::string(".pfa")) ||
        boost::algorithm::ends_with(fn,std::string(".pfb")) ||
        boost::algorithm::ends_with(fn,std::string(".ttc")) ||
        // Plus OSX custom ext
        boost::algorithm::ends_with(fn,std::string(".dfont"));
}

bool freetype_engine::register_font(std::string const& file_name)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    FT_Library library = 0;
    FT_Error error = FT_Init_FreeType(&library);
    if (error)
    {
        throw std::runtime_error("Failed to initialize FreeType2 library");
    }

    FT_Face face = 0;
    int num_faces = 0;
    bool success = false;
    // some font files have multiple fonts in a file
    // the count is in the 'root' face library[0]
    // see the FT_FaceRec in freetype.h
    for ( int i = 0; face == 0 || i < num_faces; i++ ) {
        // if face is null then this is the first face
        error = FT_New_Face (library,file_name.c_str(),i,&face);
        if (error)
        {
            break;
        }
        // store num_faces locally, after FT_Done_Face it can not be accessed any more
        if (!num_faces)
            num_faces = face->num_faces;
        // some fonts can lack names, skip them
        // http://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html#FT_FaceRec
        if (face->family_name && face->style_name)
        {
            std::string name = std::string(face->family_name) + " " + std::string(face->style_name);
            // skip fonts with leading . in the name
            if (!boost::algorithm::starts_with(name,"."))
            {
                name2file_.insert(std::make_pair(name, std::make_pair(i,file_name)));
                success = true;
            }
        }
        else
        {
            std::ostringstream s;
            s << "Warning: unable to load font file '" << file_name << "' ";
            if (!face->family_name && !face->style_name)
                s << "which lacks both a family name and style name";
            else if (face->family_name)
                s << "which reports a family name of '" << std::string(face->family_name) << "' and lacks a style name";
            else if (face->style_name)
                s << "which reports a style name of '" << std::string(face->style_name) << "' and lacks a family name";

            MAPNIK_LOG_DEBUG(font_engine_freetype) << "freetype_engine: " << s.str();
        }
    }
    if (face)
        FT_Done_Face(face);
    if (library)
        FT_Done_FreeType(library);
    return success;
}

bool freetype_engine::register_fonts(std::string const& dir, bool recurse)
{
    boost::filesystem::path path(dir);
    if (!boost::filesystem::exists(path))
    {
        return false;
    }
    if (!boost::filesystem::is_directory(path))
    {
        return mapnik::freetype_engine::register_font(dir);
    }
    boost::filesystem::directory_iterator end_itr;
    bool success = false;
    for (boost::filesystem::directory_iterator itr(dir); itr != end_itr; ++itr)
    {
#if (BOOST_FILESYSTEM_VERSION == 3)
        std::string file_name = itr->path().string();
#else // v2
        std::string file_name = itr->string();
#endif
        if (boost::filesystem::is_directory(*itr) && recurse)
        {
            if (register_fonts(file_name, true))
            {
                success = true;
            }
        }
        else
        {
#if (BOOST_FILESYSTEM_VERSION == 3)
            std::string base_name = itr->path().filename().string();
#else // v2
            std::string base_name = itr->filename();
#endif
            if (!boost::algorithm::starts_with(base_name,".") &&
                boost::filesystem::is_regular_file(file_name) &&
                is_font_file(file_name))
            {
                if (mapnik::freetype_engine::register_font(file_name))
                {
                    success = true;
                }
            }
        }
    }
    return success;
}


std::vector<std::string> freetype_engine::face_names ()
{
    std::vector<std::string> names;
    std::map<std::string,std::pair<int,std::string> >::const_iterator itr;
    for (itr = name2file_.begin();itr!=name2file_.end();++itr)
    {
        names.push_back(itr->first);
    }
    return names;
}

std::map<std::string,std::pair<int,std::string> > const& freetype_engine::get_mapping()
{
    return name2file_;
}


face_ptr freetype_engine::create_face(std::string const& family_name)
{
    std::map<std::string, std::pair<int,std::string> >::const_iterator itr;
    itr = name2file_.find(family_name);
    if (itr != name2file_.end())
    {
        FT_Face face;

        std::map<std::string,std::string>::const_iterator mem_font_itr = memory_fonts_.find(itr->second.second);

        if (mem_font_itr != memory_fonts_.end()) // memory font
        {
            FT_Error error = FT_New_Memory_Face(library_,
                                                (FT_Byte const*) mem_font_itr->second.c_str(), //buffer
                                                mem_font_itr->second.size(), // size
                                                itr->second.first, // face index
                                                &face);

            if (!error) return boost::make_shared<font_face>(face);
        }
        else
        {
            // load font into memory
#ifdef MAPNIK_THREADSAFE
            mutex::scoped_lock lock(mutex_);
#endif
            std::ifstream is(itr->second.second.c_str() , std::ios::binary);
            std::string buffer((std::istreambuf_iterator<char>(is)),
                               std::istreambuf_iterator<char>());
            std::pair<std::map<std::string,std::string>::iterator,bool> result
                = memory_fonts_.insert(std::make_pair(itr->second.second, buffer));

            FT_Error error = FT_New_Memory_Face (library_,
                                                 (FT_Byte const*) result.first->second.c_str(),
                                                 buffer.size(),
                                                 itr->second.first,
                                                 &face);
            if (!error) return boost::make_shared<font_face>(face);
            else
            {
                // we can't load font, erase it.
                memory_fonts_.erase(result.first);
            }
        }
    }
    return face_ptr();
}

stroker_ptr freetype_engine::create_stroker()
{
    FT_Stroker s;
    FT_Error error = FT_Stroker_New(library_, &s);
    if (!error)
    {
        return boost::make_shared<stroker>(s);
    }
    return stroker_ptr();
}

void font_face_set::add(face_ptr face)
{
    faces_.push_back(face);
    dimension_cache_.clear(); //Make sure we don't use old cached data
}

font_face_set::size_type font_face_set::size() const
{
    return faces_.size();
}

glyph_ptr font_face_set::get_glyph(unsigned c) const
{
    BOOST_FOREACH ( face_ptr const& face, faces_)
    {
        FT_UInt g = face->get_char(c);
        if (g) return boost::make_shared<font_glyph>(face, g);
    }

    // Final fallback to empty square if nothing better in any font
    return boost::make_shared<font_glyph>(*faces_.begin(), 0);
}

char_info font_face_set::character_dimensions(unsigned int c)
{
    //Check if char is already in cache
    typedef std::map<unsigned, char_info>::const_iterator iterator_type;
    iterator_type itr = dimension_cache_.find(c);
    if (itr != dimension_cache_.end())
    {
        return itr->second;
    }

    FT_Matrix matrix;
    FT_Vector pen;
    FT_Error  error;

    pen.x = 0;
    pen.y = 0;

    FT_BBox glyph_bbox;
    FT_Glyph image;

    glyph_ptr glyph = get_glyph(c);
    FT_Face face = glyph->get_face()->get_face();

    matrix.xx = (FT_Fixed)( 1 * 0x10000L );
    matrix.xy = (FT_Fixed)( 0 * 0x10000L );
    matrix.yx = (FT_Fixed)( 0 * 0x10000L );
    matrix.yy = (FT_Fixed)( 1 * 0x10000L );

    FT_Set_Transform(face, &matrix, &pen);

    error = FT_Load_Glyph (face, glyph->get_index(), FT_LOAD_NO_HINTING);
    if ( error )
        return char_info();

    error = FT_Get_Glyph(face->glyph, &image);
    if ( error )
        return char_info();

    FT_Glyph_Get_CBox(image, ft_glyph_bbox_pixels, &glyph_bbox);
    FT_Done_Glyph(image);

    unsigned tempx = face->glyph->advance.x >> 6;

    char_info dim(c, tempx, glyph_bbox.yMax, glyph_bbox.yMin, face->size->metrics.height/64.0);
    dimension_cache_.insert(std::make_pair(c, dim));
    return dim;
}


void font_face_set::get_string_info(string_info & info, UnicodeString const& ustr, char_properties *format)
{
    double avg_height = character_dimensions('X').height();
    UErrorCode err = U_ZERO_ERROR;
    UnicodeString reordered;
    UnicodeString shaped;

    int32_t length = ustr.length();

    UBiDi *bidi = ubidi_openSized(length, 0, &err);
    ubidi_setPara(bidi, ustr.getBuffer(), length, UBIDI_DEFAULT_LTR, 0, &err);

    ubidi_writeReordered(bidi, reordered.getBuffer(length),
                         length, UBIDI_DO_MIRRORING, &err);

    reordered.releaseBuffer(length);

    u_shapeArabic(reordered.getBuffer(), length,
                  shaped.getBuffer(length), length,
                  U_SHAPE_LETTERS_SHAPE | U_SHAPE_LENGTH_FIXED_SPACES_NEAR |
                  U_SHAPE_TEXT_DIRECTION_VISUAL_LTR, &err);

    shaped.releaseBuffer(length);

    if (U_SUCCESS(err)) {
        StringCharacterIterator iter(shaped);
        for (iter.setToStart(); iter.hasNext();) {
            UChar ch = iter.nextPostInc();
            char_info char_dim = character_dimensions(ch);
            char_dim.format = format;
            char_dim.avg_height = avg_height;
            info.add_info(char_dim);
        }
    }


#if (U_ICU_VERSION_MAJOR_NUM*100 + U_ICU_VERSION_MINOR_NUM >= 406)
    if (ubidi_getBaseDirection(ustr.getBuffer(), length) == UBIDI_RTL)
    {
        info.set_rtl(true);
    }
#endif

    ubidi_close(bidi);
}

void font_face_set::set_pixel_sizes(unsigned size)
{
    BOOST_FOREACH ( face_ptr const& face, faces_)
    {
        face->set_pixel_sizes(size);
    }
}

void font_face_set::set_character_sizes(double size)
{
    BOOST_FOREACH ( face_ptr const& face, faces_)
    {
        face->set_character_sizes(size);
    }
}


template <typename T>
void composite_bitmap(T & pixmap,
                      FT_Bitmap *bitmap,
                      unsigned rgba,
                      int x,
                      int y,
                      double opacity,
                      composite_mode_e comp_op)
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
void render_halo(T & pixmap,
                 FT_Bitmap *bitmap,
                 unsigned rgba,
                 int x1,
                 int y1,
                 double halo_radius,
                 double opacity,
                 composite_mode_e comp_op)
{
    int width = bitmap->width;
    int height = bitmap->rows;
    int x, y;
    if (halo_radius < 1.0)
    {
        for (x=0; x < width; x++)
        {
            for (y=0; y < height; y++)
            {
                int gray = bitmap->buffer[y*bitmap->width+x];
                if (gray)
                {
                    pixmap.composite_pixel(comp_op, x+x1-1, y+y1-1, rgba, gray*halo_radius*halo_radius, opacity);
                    pixmap.composite_pixel(comp_op, x+x1,   y+y1-1, rgba, gray*halo_radius, opacity);
                    pixmap.composite_pixel(comp_op, x+x1+1, y+y1-1, rgba, gray*halo_radius*halo_radius, opacity);

                    pixmap.composite_pixel(comp_op, x+x1-1, y+y1,   rgba, gray*halo_radius, opacity);
                    pixmap.composite_pixel(comp_op, x+x1,   y+y1,   rgba, gray, opacity);
                    pixmap.composite_pixel(comp_op, x+x1+1, y+y1,   rgba, gray*halo_radius, opacity);

                    pixmap.composite_pixel(comp_op, x+x1-1, y+y1+1, rgba, gray*halo_radius*halo_radius, opacity);
                    pixmap.composite_pixel(comp_op, x+x1,   y+y1+1, rgba, gray*halo_radius, opacity);
                    pixmap.composite_pixel(comp_op, x+x1+1, y+y1+1, rgba, gray*halo_radius*halo_radius, opacity);
                }
            }
        }
    } else {
        for (x=0; x < width; x++)
        {
            for (y=0; y < height; y++)
            {
                int gray = bitmap->buffer[y*bitmap->width+x];
                if (gray)
                {
                    for (int n=-halo_radius; n <=halo_radius; ++n)
                        for (int m=-halo_radius; m <= halo_radius; ++m)
                            pixmap.composite_pixel(comp_op, x+x1+m, y+y1+n, rgba, gray, opacity);
                }
            }
        }
    }
}

template <typename T>
void render_halo_id(T & pixmap,
                    FT_Bitmap *bitmap,
                    mapnik::value_integer feature_id,
                    int x1,
                    int y1,
                    int halo_radius)
{
    int width = bitmap->width;
    int height = bitmap->rows;
    int x, y;
    for (x=0; x < width; x++)
    {
        for (y=0; y < height; y++)
        {
            int gray = bitmap->buffer[y*bitmap->width+x];
            if (gray)
            {
                for (int n=-halo_radius; n <=halo_radius; ++n)
                    for (int m=-halo_radius; m <= halo_radius; ++m)
                        pixmap.setPixel(x+x1+m,y+y1+n,feature_id);
            }
        }
    }
}

template <typename T>
text_renderer<T>::text_renderer(pixmap_type & pixmap,
                                face_manager<freetype_engine> & font_manager,
                                halo_rasterizer_e rasterizer,
                                composite_mode_e comp_op,
                                double scale_factor)
    : pixmap_(pixmap),
      font_manager_(font_manager),
      rasterizer_(rasterizer),
      comp_op_(comp_op),
      scale_factor_(scale_factor) {}

template <typename T>
box2d<double> text_renderer<T>::prepare_glyphs(text_path const& path)
{
    //clear glyphs
    glyphs_.clear();

    FT_Matrix matrix;
    FT_Vector pen;
    FT_Error  error;

    FT_BBox bbox;
    bbox.xMin = bbox.yMin = 32000;  // Initialize these so we can tell if we
    bbox.xMax = bbox.yMax = -32000; // properly grew the bbox later

    for (int i = 0; i < path.num_nodes(); ++i)
    {
        char_info_ptr c;
        double x, y, angle;

        path.vertex(c, x, y, angle);

        // TODO Enable when we have support for setting verbosity
        // MAPNIK_LOG_DEBUG(font_engine_freetype) << "text_renderer: prepare_glyphs="
        //                                        << c << "," << x << "," << y << "," << angle;

        FT_BBox glyph_bbox;
        FT_Glyph image;

        pen.x = int(x * 64);
        pen.y = int(y * 64);

        face_set_ptr faces = font_manager_.get_face_set(c->format->face_name, c->format->fontset);
        faces->set_character_sizes(c->format->text_size*scale_factor_);

        glyph_ptr glyph = faces->get_glyph(unsigned(c->c));
        FT_Face face = glyph->get_face()->get_face();

        matrix.xx = (FT_Fixed)( std::cos( angle ) * 0x10000L );
        matrix.xy = (FT_Fixed)(-std::sin( angle ) * 0x10000L );
        matrix.yx = (FT_Fixed)( std::sin( angle ) * 0x10000L );
        matrix.yy = (FT_Fixed)( std::cos( angle ) * 0x10000L );

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

template <typename T>
void text_renderer<T>::render(pixel_position const& pos)
{
    FT_Error  error;
    FT_Vector start;
    unsigned height = pixmap_.height();

    start.x =  static_cast<FT_Pos>(pos.x * (1 << 6));
    start.y =  static_cast<FT_Pos>((height - pos.y) * (1 << 6));

    // now render transformed glyphs
    typename glyphs_t::iterator itr;
    for (itr = glyphs_.begin(); itr != glyphs_.end(); ++itr)
    {
        double halo_radius = itr->properties->halo_radius * scale_factor_;
        //make sure we've got reasonable values.
        if (halo_radius <= 0.0 || halo_radius > 1024.0) continue;
        FT_Glyph g;
        error = FT_Glyph_Copy(itr->image, &g);
        if (!error)
        {
            FT_Glyph_Transform(g,0,&start);
            if (rasterizer_ == HALO_RASTERIZER_FULL)
            {
                stroker_ptr stk = font_manager_.get_stroker();
                stk->init(halo_radius);
                FT_Glyph_Stroke(&g,stk->get(),1);
                error = FT_Glyph_To_Bitmap( &g,FT_RENDER_MODE_NORMAL,0,1);
                if (!error)
                {
                    FT_BitmapGlyph bit = (FT_BitmapGlyph)g;
                    composite_bitmap(pixmap_,
                                     &bit->bitmap,
                                     itr->properties->halo_fill.rgba(),
                                     bit->left,
                                     height - bit->top,
                                     itr->properties->text_opacity,
                                     comp_op_);
                }
            }
            else
            {
                error = FT_Glyph_To_Bitmap( &g,FT_RENDER_MODE_NORMAL,0,1);
                if (!error)
                {
                    FT_BitmapGlyph bit = (FT_BitmapGlyph)g;
                    render_halo(pixmap_,
                                &bit->bitmap,
                                itr->properties->halo_fill.rgba(),
                                bit->left,
                                height - bit->top,
                                halo_radius,
                                itr->properties->text_opacity,
                                comp_op_);
                }
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
            composite_bitmap(pixmap_,
                             &bit->bitmap,
                             itr->properties->fill.rgba(),
                             bit->left,
                             height - bit->top,
                             itr->properties->text_opacity,
                             comp_op_
                );
        }
    }
}

template <typename T>
void text_renderer<T>::render_id(mapnik::value_integer feature_id,
                                 pixel_position const& pos)
{
    FT_Error  error;
    FT_Vector start;
    unsigned height = pixmap_.height();

    start.x =  static_cast<FT_Pos>(pos.x * (1 << 6));
    start.y =  static_cast<FT_Pos>((height - pos.y) * (1 << 6));

    // now render transformed glyphs
    typename glyphs_t::iterator itr;
    for (itr = glyphs_.begin(); itr != glyphs_.end(); ++itr)
    {
        FT_Glyph_Transform(itr->image,0,&start);
        error = FT_Glyph_To_Bitmap( &(itr->image),FT_RENDER_MODE_NORMAL,0,1);
        if ( ! error )
        {
            FT_BitmapGlyph bit = (FT_BitmapGlyph)itr->image;
            render_halo_id(pixmap_,
                           &bit->bitmap,
                           feature_id,
                           bit->left,
                           height - bit->top,
                           itr->properties->halo_radius);
        }
    }
}

#ifdef MAPNIK_THREADSAFE
boost::mutex freetype_engine::mutex_;
#endif
std::map<std::string,std::pair<int,std::string> > freetype_engine::name2file_;
std::map<std::string,std::string> freetype_engine::memory_fonts_;

template text_renderer<image_32>::text_renderer(image_32&,
                                                face_manager<freetype_engine>&,
                                                halo_rasterizer_e,
                                                composite_mode_e,
                                                double);
template box2d<double>text_renderer<image_32>::prepare_glyphs(text_path const&);
template void text_renderer<image_32>::render(pixel_position const&);
template void text_renderer<grid>::render_id(mapnik::value_integer,
                                             pixel_position const&);
template text_renderer<grid>::text_renderer(grid&,
                                            face_manager<freetype_engine>&,
                                            halo_rasterizer_e,
                                            composite_mode_e, double);
template box2d<double>text_renderer<grid>::prepare_glyphs(text_path const& );
}
