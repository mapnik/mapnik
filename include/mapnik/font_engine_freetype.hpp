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

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
}

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/text_path.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/mutex.hpp>

// stl
#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace mapnik
{
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
	
        unsigned num_glyphs() const
        {
            return face_->num_glyphs;
        }

        FT_GlyphSlot glyph() const
        {
            return face_->glyph;
        }
	
        FT_Face get_face() const
        {
            return face_;
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
    	    std::clog << "clean up face:" << family_name()<<":" << style_name() << std::endl;
#endif
    	    FT_Done_Face(face_);
    	}
	
    private:
    	FT_Face face_;
    };
    
   typedef boost::shared_ptr<font_face> face_ptr;
   class MAPNIK_DECL freetype_engine  // : public mapnik::singleton<freetype_engine,mapnik::CreateStatic>,
         // private boost::noncopyable
    {
       // friend class mapnik::CreateStatic<freetype_engine>;
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
            ~glyph_t ()	{ FT_Done_Glyph(image);}
        };
	
        typedef boost::ptr_vector<glyph_t> glyphs_t;
        typedef std::pair<unsigned,unsigned> dimension_t;
        typedef T pixmap_type;
	
        text_renderer (pixmap_type & pixmap, face_ptr face)
            : pixmap_(pixmap),
              face_(face),
              fill_(0,0,0), 
              halo_fill_(255,255,255),
              halo_radius_(0) {}
    
        void set_pixel_size(unsigned size)
        {
            face_->set_pixel_sizes(size);
        }
    
        void set_fill(mapnik::Color const& fill)
        {
            fill_=fill;
        }
    
        void set_halo_fill(mapnik::Color const& halo)
        {
            halo_fill_=halo;
        }
    
        void set_halo_radius( int radius=1)
        {
            halo_radius_=radius;
        }

        Envelope<double> prepare_glyphs(text_path *path)
        {
            //clear glyphs
            glyphs_.clear();
	    
            FT_Matrix matrix;
            FT_Vector pen;
            FT_Error  error;
	    
            FT_Face face = face_->get_face();
            //            FT_GlyphSlot slot = face->glyph;
	    
            FT_BBox bbox;   
            bbox.xMin = bbox.yMin = 32000; 
            bbox.xMax = bbox.yMax = -32000; //hmm?? 
	    
            for (int i = 0; i < path->num_nodes(); i++) 
            {
                int c;
                double x, y, angle;
                
                path->vertex(&c, &x, &y, &angle);
//                std::clog << "   prepare_glyph: " << (unsigned char)c << "," << x << "," << y << "," << angle << std::endl;


                FT_BBox glyph_bbox; 
                FT_Glyph image;
		
                pen.x = int(x * 64);
                pen.y = int(y * 64);
	        	
                matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L ); 
                matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L ); 
                matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L ); 
                matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );
		
                FT_Set_Transform (face,&matrix,&pen);
		
                FT_UInt glyph_index = FT_Get_Char_Index( face, unsigned(c));
		
                error = FT_Load_Glyph (face,glyph_index, FT_LOAD_NO_HINTING); 
                if ( error )
                    continue;
		
                error = FT_Get_Glyph( face->glyph, &image);
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
      
        dimension_t character_dimensions(const unsigned c)
        {
            FT_Matrix matrix;
            FT_Vector pen;
            FT_Error  error;
            
            FT_Face face = face_->get_face();
            FT_GlyphSlot slot = face->glyph;
            
            pen.x = 0;
            pen.y = 0;
            
            FT_BBox glyph_bbox; 
            FT_Glyph image;
            
            matrix.xx = (FT_Fixed)( 1 * 0x10000L ); 
            matrix.xy = (FT_Fixed)( 0 * 0x10000L ); 
            matrix.yx = (FT_Fixed)( 0 * 0x10000L ); 
            matrix.yy = (FT_Fixed)( 1 * 0x10000L );
                    
            FT_Set_Transform (face,&matrix,&pen);
            
            FT_UInt glyph_index = FT_Get_Char_Index( face, c);
            
            error = FT_Load_Glyph (face,glyph_index,FT_LOAD_NO_HINTING); 
            if ( error )
                return dimension_t(0, 0);
            
            error = FT_Get_Glyph( face->glyph, &image);
            if ( error )
                return dimension_t(0, 0);
            
            FT_Glyph_Get_CBox(image,ft_glyph_bbox_pixels, &glyph_bbox); 
            FT_Done_Glyph(image); 
            return dimension_t(slot->advance.x >> 6, glyph_bbox.yMax - glyph_bbox.yMin);
        }
        
        void get_string_info(string_info *info)
        {
            unsigned width = 0;
            unsigned height = 0;

            std::wstring text = info->get_string();

            for (std::wstring::const_iterator i=text.begin();i!=text.end();++i)
            {
                dimension_t char_dim = character_dimensions(*i);
              
                info->add_info(*i, char_dim.first, char_dim.second);
              
                width += char_dim.first;
                height = char_dim.second > height ? char_dim.second : height;
                
            }
            info->set_dimensions(width, height);
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
                                pixmap_.blendPixel(i+m,j+n,rgba,gray);		        
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
                        pixmap_.blendPixel(i,j,rgba,gray);
                    }
                }
            }
        }
    
        pixmap_type & pixmap_;
        face_ptr face_;
        mapnik::Color fill_;
        mapnik::Color halo_fill_;
        int halo_radius_;
        unsigned text_ratio_;
        unsigned wrap_width_;
        glyphs_t glyphs_;
    }; 
}


#endif // FONT_ENGINE_FREETYPE_HPP
