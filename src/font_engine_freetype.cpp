/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
//$Id$

// mapnik
#include <mapnik/font_engine_freetype.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

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
      
    FT_Face face;
    error = FT_New_Face (library,file_name.c_str(),0,&face);
    if (error)
    {
        FT_Done_FreeType(library);
        return false;
    }
    std::string name = std::string(face->family_name) + " " + std::string(face->style_name);
    name2file_.insert(std::make_pair(name,file_name));
    FT_Done_Face(face );   
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
    std::map<std::string,std::string>::const_iterator itr;
    for (itr = name2file_.begin();itr!=name2file_.end();++itr)
    {
        names.push_back(itr->first);
    }
    return names;
}

face_ptr freetype_engine::create_face(std::string const& family_name)
{
    std::map<std::string,std::string>::iterator itr;
    itr = name2file_.find(family_name);
    if (itr != name2file_.end())
    {
        FT_Face face;
        FT_Error error = FT_New_Face (library_,itr->second.c_str(),0,&face);

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

font_face_set::dimension_t font_face_set::character_dimensions(const unsigned c)
{
    std::map<char, dimension_t>::const_iterator itr;
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
        return dimension_t(0, 0, 0);

    error = FT_Get_Glyph(face->glyph, &image);
    if ( error )
        return dimension_t(0, 0, 0);

    FT_Glyph_Get_CBox(image, ft_glyph_bbox_pixels, &glyph_bbox);
    FT_Done_Glyph(image);

    unsigned tempx = face->glyph->advance.x >> 6;

    //std::clog << "glyph: " << glyph_index << " x: " << tempx << " y: " << tempy << std::endl;
    dimension_t dim(tempx, glyph_bbox.yMax, glyph_bbox.yMin);
    //dimension_cache_[c] = dim; would need an default constructor for dimension_t
    dimension_cache_.insert(std::pair<unsigned, dimension_t>(c, dim));
    return dim;
}

void font_face_set::get_string_info(string_info & info)
{
    unsigned width = 0;
    unsigned height = 0;
    UErrorCode err = U_ZERO_ERROR;
    UnicodeString const& ustr = info.get_string();
    const UChar * text = ustr.getBuffer();
    UBiDi * bidi = ubidi_openSized(ustr.length(),0,&err);

    if (U_SUCCESS(err))
    {
        ubidi_setPara(bidi,text,ustr.length(), UBIDI_DEFAULT_LTR,0,&err);

        if (U_SUCCESS(err))
        {
            int32_t count = ubidi_countRuns(bidi,&err);
            int32_t logicalStart;
            int32_t length;

            for (int32_t i=0; i< count;++i)
            {
                if (UBIDI_LTR == ubidi_getVisualRun(bidi,i,&logicalStart,&length))
                {
                    do {
                        UChar ch = text[logicalStart++];
                        dimension_t char_dim = character_dimensions(ch);
                        info.add_info(ch, char_dim.width, char_dim.height);
                        width += char_dim.width;
                        height = char_dim.height > height ? char_dim.height : height;

                    } while (--length > 0);
                }
                else
                {
                    logicalStart += length;

                    int32_t j=0,i=length;
                    UnicodeString arabic;
                    UChar * buf = arabic.getBuffer(length);
                    do {
                        UChar ch = text[--logicalStart];
                        buf[j++] = ch;
                    } while (--i > 0);

                    arabic.releaseBuffer(length);
                    if ( *arabic.getBuffer() >= 0x0600 && *arabic.getBuffer() <= 0x06ff)
                    {
                        UnicodeString shaped;
                        u_shapeArabic(arabic.getBuffer(),arabic.length(),shaped.getBuffer(arabic.length()),arabic.length(),
                                      U_SHAPE_LETTERS_SHAPE|U_SHAPE_LENGTH_FIXED_SPACES_NEAR|
                                      U_SHAPE_TEXT_DIRECTION_VISUAL_LTR
                                      ,&err);

                        shaped.releaseBuffer(arabic.length());

                        if (U_SUCCESS(err))
                        {
                            for (int j=0;j<shaped.length();++j)
                            {
                                dimension_t char_dim = character_dimensions(shaped[j]);
                                info.add_info(shaped[j], char_dim.width, char_dim.height);
                                width += char_dim.width;
                                height = char_dim.height > height ? char_dim.height : height;
                            }
                        }
                    } else {
                        // Non-Arabic RTL
                        for (int j=0;j<arabic.length();++j)
                        {
                            dimension_t char_dim = character_dimensions(arabic[j]);
                            info.add_info(arabic[j], char_dim.width, char_dim.height);
                            width += char_dim.width;
                            height = char_dim.height > height ? char_dim.height : height;
                        }
                    }
                }
            }
        }
        ubidi_close(bidi);
    }

    info.set_dimensions(width, height);
}

#ifdef MAPNIK_THREADSAFE
boost::mutex freetype_engine::mutex_;
#endif
std::map<std::string,std::string> freetype_engine::name2file_;
}
