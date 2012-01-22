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
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text_placements.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/grid/grid.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <sstream>

// icu
#include <unicode/ubidi.h>
#include <unicode/ushape.h>
#include <unicode/schriter.h>
#include <unicode/uversion.h> 

namespace mapnik
{
freetype_engine::freetype_engine()
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
    /** only accept files that will be matched by freetype2's `figurefiletype()` */
    std::string const& fn = boost::algorithm::to_lower_copy(file_name);
    return boost::algorithm::ends_with(fn,std::string(".ttf")) ||
        boost::algorithm::ends_with(fn,std::string(".otf")) ||
        boost::algorithm::ends_with(fn,std::string(".ttc")) ||
        boost::algorithm::ends_with(fn,std::string(".pfa")) ||
        boost::algorithm::ends_with(fn,std::string(".pfb")) ||
        boost::algorithm::ends_with(fn,std::string(".ttc")) ||
        /** Plus OSX custom ext */
        boost::algorithm::ends_with(fn,std::string(".dfont"));
}

bool freetype_engine::register_font(std::string const& file_name)
{
    if (!boost::filesystem::is_regular_file(file_name) || !is_font_file(file_name)) return false;
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    FT_Library library;
    FT_Error error = FT_Init_FreeType(&library);
    if (error)
    {
        throw std::runtime_error("Failed to initialize FreeType2 library");
    }
      
    FT_Face face = 0;
    // some font files have multiple fonts in a file
    // the count is in the 'root' face library[0]
    // see the FT_FaceRec in freetype.h
    for ( int i = 0; face == 0 || i < face->num_faces; i++ ) {
        // if face is null then this is the first face
        error = FT_New_Face (library,file_name.c_str(),i,&face);
        if (error)
        {
            FT_Done_FreeType(library);
            return false;
        }
        // some fonts can lack names, skip them
        // http://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html#FT_FaceRec
        if (face->family_name && face->style_name) {
            std::string name = std::string(face->family_name) + " " + std::string(face->style_name);
            name2file_.insert(std::make_pair(name, std::make_pair(i,file_name)));
            FT_Done_Face(face);
            //FT_Done_FreeType(library);
            //return true;
        } else {
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            std::ostringstream s;
            s << "Error: unable to load invalid font file which lacks identifiable family and style name: '"
            << file_name << "'";
            throw std::runtime_error(s.str());
        }
    }
    FT_Done_FreeType(library);
    return true;
}

bool freetype_engine::register_fonts(std::string const& dir, bool recurse)
{
    boost::filesystem::path path(dir);
    
    if (!boost::filesystem::exists(path))
        return false;

    if (!boost::filesystem::is_directory(path))
        return mapnik::freetype_engine::register_font(dir); 
    
    boost::filesystem::directory_iterator end_itr;
    for (boost::filesystem::directory_iterator itr(dir); itr != end_itr; ++itr)
    {
        if (boost::filesystem::is_directory(*itr) && recurse)
        {
#if (BOOST_FILESYSTEM_VERSION == 3) 
            if (!register_fonts(itr->path().string(), true)) return false;
#else // v2
            if (!register_fonts(itr->string(), true)) return false;
#endif
        }
        else 
        {
#if (BOOST_FILESYSTEM_VERSION == 3) 
            mapnik::freetype_engine::register_font(itr->path().string());
#else // v2
            mapnik::freetype_engine::register_font(itr->string());  
#endif
        }
    }
    return true;
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
    std::map<std::string, std::pair<int,std::string> >::iterator itr;
    itr = name2file_.find(family_name);
    if (itr != name2file_.end())
    {
        FT_Face face;
        FT_Error error = FT_New_Face (library_, 
                                      itr->second.second.c_str(), 
                                      itr->second.first, 
                                      &face);
        if (!error)
        {           
            return face_ptr (new font_face(face));
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
        return stroker_ptr(new stroker(s));
    }
    return stroker_ptr();
}

char_info font_face_set::character_dimensions(const unsigned c)
{
    //Check if char is already in cache
    std::map<unsigned, char_info>::const_iterator itr;
    itr = dimension_cache_.find(c);
    if (itr != dimension_cache_.end()) {
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

    char_info dim(c, tempx, glyph_bbox.yMax, glyph_bbox.yMin, face->size->metrics.height/64.0 /* >> 6 */);
    dimension_cache_.insert(std::pair<unsigned, char_info>(c, dim));
    return dim;
}

void font_face_set::get_string_info(string_info & info, UnicodeString const& ustr, char_properties *format)
{
    double avg_height = character_dimensions('X').height();
    unsigned width = 0;
    unsigned height = 0;
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
            width += char_dim.width;
            height = (char_dim.height() > height) ? char_dim.height() : height;
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
    info.set_dimensions(width, height);
}

template <typename T>
text_renderer<T>::text_renderer (pixmap_type & pixmap, face_set_ptr faces, stroker & s)
    : pixmap_(pixmap),
      faces_(faces),
      stroker_(s),
      fill_(0,0,0),
      halo_fill_(255,255,255),
      halo_radius_(0.0),
      opacity_(1.0)
{

}

#if 0
template <typename T>
text_renderer<T>::text_renderer (pixmap_type & pixmap, face_manager<freetype_engine> &font_manager_, stroker & s)
    : pixmap_(pixmap),
      font_manager_(font_manager_),
      stroker_(s)
{

}
#endif

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
        int c;
        double x, y, angle;
        char_properties *properties;

        path->vertex(&c, &x, &y, &angle, &properties);

#ifdef MAPNIK_DEBUG
        // TODO Enable when we have support for setting verbosity
        //std::clog << "prepare_glyphs: " << c << "," << x <<
        //    "," << y << "," << angle << std::endl;
#endif

        FT_BBox glyph_bbox;
        FT_Glyph image;

        pen.x = int(x * 64);
        pen.y = int(y * 64);

        glyph_ptr glyph = faces_->get_glyph(unsigned(c));
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
        glyphs_.push_back(new glyph_t(image, 0));
    }

    return box2d<double>(bbox.xMin, bbox.yMin, bbox.xMax, bbox.yMax);
}

template <typename T>
void text_renderer<T>::render(double x0, double y0)
{
    FT_Error  error;
    FT_Vector start;
    unsigned height = pixmap_.height();

    start.x =  static_cast<FT_Pos>(x0 * (1 << 6));
    start.y =  static_cast<FT_Pos>((height - y0) * (1 << 6));

    // now render transformed glyphs
    typename glyphs_t::iterator pos;

    //make sure we've got reasonable values.
    if (halo_radius_ > 0.0 && halo_radius_ < 1024.0)
    {
        stroker_.init(halo_radius_);
        for ( pos = glyphs_.begin(); pos != glyphs_.end();++pos)
        {
            FT_Glyph g;
            error = FT_Glyph_Copy(pos->image, &g);
            if (!error)
            {
                FT_Glyph_Transform(g,0,&start);
                FT_Glyph_Stroke(&g,stroker_.get(),1);
                error = FT_Glyph_To_Bitmap( &g,FT_RENDER_MODE_NORMAL,0,1);
                if ( ! error )
                {

                    FT_BitmapGlyph bit = (FT_BitmapGlyph)g;
                    render_bitmap(&bit->bitmap, halo_fill_.rgba(),
                                  bit->left,
                                  height - bit->top, opacity_);
                }
            }
            FT_Done_Glyph(g);
        }
    }
    //render actual text
    for ( pos = glyphs_.begin(); pos != glyphs_.end();++pos)
    {

        FT_Glyph_Transform(pos->image,0,&start);

        error = FT_Glyph_To_Bitmap( &(pos->image),FT_RENDER_MODE_NORMAL,0,1);
        if ( ! error )
        {

            FT_BitmapGlyph bit = (FT_BitmapGlyph)pos->image;
            render_bitmap(&bit->bitmap, fill_.rgba(),
                          bit->left,
                          height - bit->top, opacity_);
        }
    }
}


template <typename T>
void text_renderer<T>::render_id(int feature_id,double x0, double y0, double min_radius)
{
    FT_Error  error;
    FT_Vector start;
    unsigned height = pixmap_.height();

    start.x =  static_cast<FT_Pos>(x0 * (1 << 6));
    start.y =  static_cast<FT_Pos>((height - y0) * (1 << 6));

    // now render transformed glyphs
    typename glyphs_t::iterator pos;

    stroker_.init(std::max(halo_radius_,min_radius));
    for ( pos = glyphs_.begin(); pos != glyphs_.end();++pos)
    {
        FT_Glyph g;
        error = FT_Glyph_Copy(pos->image, &g);
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
}

#ifdef MAPNIK_THREADSAFE
boost::mutex freetype_engine::mutex_;
#endif
std::map<std::string,std::pair<int,std::string> > freetype_engine::name2file_;
template void text_renderer<image_32>::render(double, double);
template text_renderer<image_32>::text_renderer(image_32&, face_set_ptr, stroker&);
template box2d<double>text_renderer<image_32>::prepare_glyphs(text_path*);

template void text_renderer<grid>::render_id(int, double, double, double);
template text_renderer<grid>::text_renderer(grid&, face_set_ptr, stroker&);
template box2d<double>text_renderer<grid>::prepare_glyphs(text_path*);
}
