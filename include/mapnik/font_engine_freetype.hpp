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

#ifndef FONT_ENGINE_FREETYPE_HPP
#define FONT_ENGINE_FREETYPE_HPP
// mapnik
#include <mapnik/color.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/text_path.hpp>
#include <mapnik/font_set.hpp>

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
}

// boost
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/mutex.hpp>

// stl
#include <string>
#include <vector>
#include <map>
#include <iostream>

// icu
#include <unicode/ubidi.h>
#include <unicode/ushape.h>

namespace mapnik
{
    class font_face;

    typedef boost::shared_ptr<font_face> face_ptr;

    class MAPNIK_DECL font_glyph : private boost::noncopyable
    {
    public:
        font_glyph(face_ptr face, unsigned index)
           : face_(face), index_(index) {}

        face_ptr get_face() const
        {
            return face_;
        }

        unsigned get_index() const
        {
            return index_;
        }
    private:
        face_ptr face_;
        unsigned index_;
    };

    typedef boost::shared_ptr<font_glyph> glyph_ptr;

    class font_face : boost::noncopyable
    {
    public:
        font_face(FT_Face face)
           : face_(face) {}

        std::string  family_name() const
        {
            return std::string(face_->family_name);
        }

        std::string  style_name() const
        {
            return std::string(face_->style_name);
        }

        FT_GlyphSlot glyph() const
        {
            return face_->glyph;
        }

        FT_Face get_face() const
        {
            return face_;
        }

        unsigned get_char(unsigned c) const
        {
            return FT_Get_Char_Index(face_, c);
        }

        bool set_pixel_sizes(unsigned size)
        {
            if (! FT_Set_Pixel_Sizes( face_, 0, size ))
                return true;

            return false;
        }

        ~font_face()
        {
#ifdef MAPNIK_DEBUG
        std::clog << "~font_face: Clean up face \"" << family_name()
            << " " << style_name() << "\"" << std::endl;
#endif
        FT_Done_Face(face_);
        }

    private:
    FT_Face face_;
    };

    class MAPNIK_DECL font_face_set : private boost::noncopyable
    {
    public:
        typedef std::pair<unsigned,unsigned> dimension_t;

        font_face_set(void)
           : faces_() {}

        void add(face_ptr face)
        {
            faces_.push_back(face);
        }

        unsigned size() const
        {
            return faces_.size();
        }

        glyph_ptr get_glyph(unsigned c) const
        {
            for (std::vector<face_ptr>::const_iterator face = faces_.begin(); face != faces_.end(); ++face)
            {
               FT_UInt g = (*face)->get_char(c);

               if (g) return glyph_ptr(new font_glyph(*face, g));
            }

            // Final fallback to empty square if nothing better in any font
            return glyph_ptr(new font_glyph(*faces_.begin(), 0));
        }

        dimension_t character_dimensions(const unsigned c)
        {
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
                return dimension_t(0, 0);

            error = FT_Get_Glyph(face->glyph, &image);
            if ( error )
                return dimension_t(0, 0);

            FT_Glyph_Get_CBox(image, ft_glyph_bbox_pixels, &glyph_bbox);
            FT_Done_Glyph(image);

            unsigned tempx = face->glyph->advance.x >> 6;
            unsigned tempy = glyph_bbox.yMax - glyph_bbox.yMin;

            //std::clog << "glyph: " << glyph_index << " x: " << tempx << " y: " << tempy << std::endl;

            return dimension_t(tempx, tempy);
        }

        void get_string_info(string_info & info)
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
                           info.add_info(ch, char_dim.first, char_dim.second);
                           width += char_dim.first;
                           height = char_dim.second > height ? char_dim.second : height;

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
                                 info.add_info(shaped[j], char_dim.first, char_dim.second);
                                 width += char_dim.first;
                                 height = char_dim.second > height ? char_dim.second : height;
                              }
                           }
                        } else {
                           // Non-Arabic RTL
                           for (int j=0;j<arabic.length();++j)
                           {
                              dimension_t char_dim = character_dimensions(arabic[j]);
                              info.add_info(arabic[j], char_dim.first, char_dim.second);
                              width += char_dim.first;
                              height = char_dim.second > height ? char_dim.second : height;
                           }
                        }
                     }
                  }
               }
               ubidi_close(bidi);
            }

            info.set_dimensions(width, height);
        }

        void set_pixel_sizes(unsigned size)
        {
            for (std::vector<face_ptr>::iterator face = faces_.begin(); face != faces_.end(); ++face)
            {
                (*face)->set_pixel_sizes(size);
            }
        }
    private:
        std::vector<face_ptr> faces_;
    };

    typedef boost::shared_ptr<font_face_set> face_set_ptr;

    class MAPNIK_DECL freetype_engine
    {
      public:
        static bool register_font(std::string const& file_name);
        static std::vector<std::string> face_names ();
        face_ptr create_face(std::string const& family_name);
        virtual ~freetype_engine();
        freetype_engine();
      private:
        FT_Library library_;
        static boost::mutex mutex_;
        static std::map<std::string,std::string> name2file_;
    };

    template <typename T>
    class MAPNIK_DECL face_manager : private boost::noncopyable
    {
        typedef T font_engine_type;
        typedef std::map<std::string,face_ptr> faces;

    public:
        face_manager(T & engine)
           : engine_(engine) {}

        face_ptr get_face(std::string const& name)
        {
            typename faces::iterator itr;
            itr = faces_.find(name);
            if (itr != faces_.end())
            {
                return itr->second;
            }
            else
            {
                face_ptr face = engine_.create_face(name);
                if (face)
                {
                    faces_.insert(make_pair(name,face));
                }
                return face;
            }
        }

        face_set_ptr get_face_set(std::string const& name)
        {
            face_set_ptr face_set(new font_face_set);
            if (face_ptr face = get_face(name))
            {
                face_set->add(face);
            }
            return face_set;
        }

        face_set_ptr get_face_set(FontSet const& fontset)
        {
            std::vector<std::string> const& names = fontset.get_face_names();
            face_set_ptr face_set(new font_face_set);
            for (std::vector<std::string>::const_iterator name = names.begin(); name != names.end(); ++name)
            {
                if (face_ptr face = get_face(*name))
                {
                    face_set->add(face);
                }
            }
            return face_set;
        }
    private:
        faces faces_;
        font_engine_type & engine_;
    };

    template <typename T>
    struct text_renderer : private boost::noncopyable
    {
        struct glyph_t : boost::noncopyable
        {
            FT_Glyph image;
            glyph_t(FT_Glyph image_) : image(image_) {}
            ~glyph_t () { FT_Done_Glyph(image);}
        };

        typedef boost::ptr_vector<glyph_t> glyphs_t;
        typedef T pixmap_type;

        text_renderer (pixmap_type & pixmap, face_set_ptr faces)
            : pixmap_(pixmap),
              faces_(faces),
              fill_(0,0,0),
              halo_fill_(255,255,255),
              halo_radius_(0),
              opacity_(1.0) {}

        void set_pixel_size(unsigned size)
        {
            faces_->set_pixel_sizes(size);
        }

        void set_fill(mapnik::color const& fill)
        {
            fill_=fill;
        }

        void set_halo_fill(mapnik::color const& halo)
        {
            halo_fill_=halo;
        }

        void set_halo_radius( int radius=1)
        {
            halo_radius_=radius;
        }

        void set_opacity( double opacity=1.0)
        {
            opacity_=opacity;
        }

        Envelope<double> prepare_glyphs(text_path *path)
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

                path->vertex(&c, &x, &y, &angle);

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
                glyphs_.push_back(new glyph_t(image));
            }

            return Envelope<double>(bbox.xMin, bbox.yMin, bbox.xMax, bbox.yMax);
        }

        void render(double x0, double y0)
        {
            FT_Error  error;
            FT_Vector start;
            unsigned height = pixmap_.height();

            start.x =  static_cast<FT_Pos>(x0 * (1 << 6));
            start.y =  static_cast<FT_Pos>((height - y0) * (1 << 6));

            // now render transformed glyphs
            typename glyphs_t::iterator pos;

            //make sure we've got reasonable values.
            if (halo_radius_ > 0 && halo_radius_ < 256)
            {
                //render halo
                for ( pos = glyphs_.begin(); pos != glyphs_.end();++pos)
                {
                    FT_Glyph_Transform(pos->image,0,&start);

                    error = FT_Glyph_To_Bitmap( &(pos->image),FT_RENDER_MODE_NORMAL,0,1);
                    if ( ! error )
                    {

                        FT_BitmapGlyph bit = (FT_BitmapGlyph)pos->image;
                        render_halo(&bit->bitmap, halo_fill_.rgba(),
                                    bit->left,
                                    height - bit->top,halo_radius_);
                    }
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
                                  height - bit->top);
                }
            }
        }

    private:

        void render_halo(FT_Bitmap *bitmap,unsigned rgba,int x,int y,int radius)
        {
            int x_max=x+bitmap->width;
            int y_max=y+bitmap->rows;
            int i,p,j,q;

            for (i=x,p=0;i<x_max;++i,++p)
            {
                for (j=y,q=0;j<y_max;++j,++q)
                {
                    int gray = bitmap->buffer[q*bitmap->width+p];
                    if (gray)
                    {
                        for (int n=-halo_radius_; n <=halo_radius_; ++n)
                            for (int m=-halo_radius_;m <= halo_radius_; ++m)
                                pixmap_.blendPixel2(i+m,j+n,rgba,gray,opacity_);
                    }
                }
            }
        }

        void render_bitmap(FT_Bitmap *bitmap,unsigned rgba,int x,int y)
        {
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
                        pixmap_.blendPixel2(i,j,rgba,gray,opacity_);
                    }
                }
            }
        }

        pixmap_type & pixmap_;
        face_set_ptr faces_;
        mapnik::color fill_;
        mapnik::color halo_fill_;
        int halo_radius_;
        unsigned text_ratio_;
        unsigned wrap_width_;
        glyphs_t glyphs_;
        double opacity_;
    };
}

#endif // FONT_ENGINE_FREETYPE_HPP
