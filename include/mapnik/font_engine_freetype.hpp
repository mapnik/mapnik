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
#include FT_STROKER_H
}

// boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
#endif


// stl
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>

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
    class dimension_t {
    public:
        dimension_t(unsigned width_, int ymax_, int ymin_) :  width(width_), height(ymax_-ymin_), ymin(ymin_) {}
        unsigned width, height;
        int ymin;
    };

    font_face_set(void)
        : faces_() {}

    void add(face_ptr face)
    {
        faces_.push_back(face);
        dimension_cache_.clear(); //Make sure we don't use old cached data
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

            if (g) return boost::make_shared<font_glyph>(*face, g);
        }

        // Final fallback to empty square if nothing better in any font
        return boost::make_shared<font_glyph>(*faces_.begin(), 0);
    }

    dimension_t character_dimensions(const unsigned c);

    void get_string_info(string_info & info);

    void set_pixel_sizes(unsigned size)
    {
        for (std::vector<face_ptr>::iterator face = faces_.begin(); face != faces_.end(); ++face)
        {
            (*face)->set_pixel_sizes(size);
        }
    }
private:
    std::vector<face_ptr> faces_;
    std::map<unsigned, dimension_t> dimension_cache_;
};

// FT_Stroker wrapper
class stroker : boost::noncopyable
{
public:
    explicit stroker(FT_Stroker s)
        : s_(s) {}
    
    void init(double radius)
    {
        FT_Stroker_Set(s_,radius * (1<<6), 
                       FT_STROKER_LINECAP_ROUND, 
                       FT_STROKER_LINEJOIN_ROUND, 
                       0);    
    }
    
    FT_Stroker const& get() const
    {
        return s_;
    }
    
    ~stroker()
    {
#ifdef MAPNIK_DEBUG
        std::clog << "~stroker: destroy stroker:" << s_ << std::endl;
#endif        
        FT_Stroker_Done(s_);
    }
private:
    FT_Stroker s_;
};



typedef boost::shared_ptr<font_face_set> face_set_ptr;
typedef boost::shared_ptr<stroker> stroker_ptr;

class MAPNIK_DECL freetype_engine
{
public:
    static bool is_font_file(std::string const& file_name);
    static bool register_font(std::string const& file_name);
    static bool register_fonts(std::string const& dir, bool recurse = false);
    static std::vector<std::string> face_names ();
    face_ptr create_face(std::string const& family_name);
    stroker_ptr create_stroker();
    virtual ~freetype_engine();
    freetype_engine();
private:
    FT_Library library_;
#ifdef MAPNIK_THREADSAFE
    static boost::mutex mutex_;
#endif
    static std::map<std::string,std::string> name2file_;
};

template <typename T>
class MAPNIK_DECL face_manager : private boost::noncopyable
{
    typedef T font_engine_type;
    typedef std::map<std::string,face_ptr> faces;

public:
    face_manager(T & engine)
        : engine_(engine),
        stroker_(engine_.create_stroker())  {}

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
        face_set_ptr face_set = boost::make_shared<font_face_set>();
        if (face_ptr face = get_face(name))
        {
            face_set->add(face);
        }
        return face_set;
    }

    face_set_ptr get_face_set(font_set const& fset)
    {
        std::vector<std::string> const& names = fset.get_face_names();
        face_set_ptr face_set = boost::make_shared<font_face_set>();
        for (std::vector<std::string>::const_iterator name = names.begin(); name != names.end(); ++name)
        {
            if (face_ptr face = get_face(*name))
            {
                face_set->add(face);
            }
        }
        return face_set;
    }

    stroker_ptr get_stroker()
    {
        return stroker_;
    }

private:
    faces faces_;
    font_engine_type & engine_;
    stroker_ptr stroker_;
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

    text_renderer (pixmap_type & pixmap, face_set_ptr faces, stroker & s)
        : pixmap_(pixmap),
          faces_(faces),
          stroker_(s),
          fill_(0,0,0),
          halo_fill_(255,255,255),
          halo_radius_(0.0),
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

    void set_halo_radius( double radius=1.0)
    {
        halo_radius_=radius;
    }

    void set_opacity( double opacity=1.0)
    {
        opacity_=opacity;
    }

    box2d<double> prepare_glyphs(text_path *path)
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

        return box2d<double>(bbox.xMin, bbox.yMin, bbox.xMax, bbox.yMax);
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
                                      height - bit->top);
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
                              height - bit->top);
            }
        }
    }

    void render_id(int feature_id,double x0, double y0, double min_radius=1.0)
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
    
private:

    // unused currently, stroker is the new method for drawing halos
    /*
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
    */

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

    void render_bitmap_id(FT_Bitmap *bitmap,int feature_id,int x,int y)
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
                    pixmap_.setPixel(i,j,feature_id);
                    //pixmap_.blendPixel2(i,j,rgba,gray,opacity_);
                }
            }
        }
    }

    pixmap_type & pixmap_;
    face_set_ptr faces_;
    stroker & stroker_;
    color fill_;
    color halo_fill_;
    double halo_radius_;
    glyphs_t glyphs_;
    double opacity_;
};
}

#endif // FONT_ENGINE_FREETYPE_HPP
