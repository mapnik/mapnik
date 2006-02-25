 /* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id$

#if !defined FONT_ENGINE_FREETYPE_HPP
#define FONT_ENGINE_FREETYPE_HPP

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <color.hpp>
#include <utils.hpp>

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

	/*
	unsigned glyph_index(unsigned charcode) const
	{
	    return FT_Get_Char_Index(face_, charcode );
	}
	
	void set_transform (FT_Matrix * m,FT_Vector *v)
	{
	    FT_Set_Transform(face_,m,v);
	}
	
	bool render_glyph (unsigned charcode) const
	{
	    unsigned glyph_index = FT_Get_Char_Index(face_, charcode );
	    FT_Error  error;
	    error = FT_Load_Glyph(face_,glyph_index,FT_LOAD_DEFAULT);
	    if (error == 0 )
	    {
		error = FT_Render_Glyph(face_->glyph, FT_RENDER_MODE_NORMAL );
		if (error == 0)
		{
		    return true;
		}
	    }
	    return false;
	}
	*/
	
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
    	    std::cout << "clean up face:" << family_name()<<":" << style_name() << std::endl;
    	    FT_Done_Face(face_);
    	}
	
    private:
    	FT_Face face_;
    };
    
    typedef boost::shared_ptr<font_face> face_ptr;
    
    class freetype_engine : public mapnik::singleton<freetype_engine,mapnik::CreateStatic>,
		       private boost::noncopyable
    {
	friend class mapnik::CreateStatic<freetype_engine>;
    public:
	
	static bool register_font(std::string const& file_name)
	{
	    mutex::scoped_lock lock(mapnik::singleton<freetype_engine, 
				    mapnik::CreateStatic>::mutex_);
	    FT_Face face;
	    FT_Error error = FT_New_Face (library_,file_name.c_str(),0,&face);
	    if ( !error )
	    {
		std::string name = std::string(face->family_name) + " " + std::string(face->style_name);
		name2file_.insert(std::make_pair(name,file_name));
		FT_Done_Face(face );
		return true;
	    }
	    return false;
	}
	
	static std::vector<std::string> face_names ()
	{
	    std::vector<std::string> names;
	    std::map<std::string,std::string>::const_iterator itr;
	    for (itr = name2file_.begin();itr!=name2file_.end();++itr)
	    {
		names.push_back(itr->first);
	    }
	    return names;
	}
	
	static face_ptr create_face(std::string const& family_name)
	{
	    mutex::scoped_lock lock(mapnik::singleton<freetype_engine, 
				    mapnik::CreateStatic>::mutex_);

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
	
    private:
	freetype_engine()
	{
	    FT_Error error = FT_Init_FreeType( &library_ );
	    if (error)
	    {
		throw std::runtime_error("can not load FreeType2 library");
	    }
	}
	virtual ~freetype_engine()
	{   
	    FT_Done_FreeType(library_);   
	}
	
        static FT_Library library_;
	static std::map<std::string,std::string> name2file_;
    }; 
    
    
    template <typename T>
    class face_manager : private boost::noncopyable
    {
	typedef T font_engine_type;
	typedef std::map<std::string,face_ptr> faces;
	
    public:
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
		face_ptr face = font_engine_type::instance()->create_face(name);
		if (face)
		{
		    faces_.insert(make_pair(name,face));
		}
		return face;	
	    }
	}
    private:
	faces faces_;
    };
        
    inline std::wstring to_unicode(std::string const& text)
    {
	std::wstring out;
	unsigned long code = 0;
	int expect = 0;
	std::string::const_iterator itr=text.begin();
	
	while ( itr != text.end())
	{
	    unsigned p = (*itr++) & 0xff;
	    if ( p >= 0xc0)
	    {
		if ( p < 0xe0)      // U+0080 - U+07ff
		{
		    expect = 1;
		    code = p & 0x1f;
		}
		else if ( p < 0xf0)  // U+0800 - U+ffff
		{
		    expect = 2;
		    code = p & 0x0f;
		}
		else if ( p  < 0xf8) // U+1000 - U+10ffff
		{
		    expect = 3;
		    code = p & 0x07;
		}
		continue;
	    }
	    else if (p >= 0x80)
	    {
		--expect;
		if (expect >= 0)
		{
		    code <<= 6;
		    code += p & 0x3f;
		}
		if (expect > 0)
		    continue;
		expect = 0;
	    }
	    else 
	    {
		code = p;            // U+0000 - U+007f (ascii)
	    }
	    out.push_back(code);
	}
	return out;
    }
    
    template <typename T>
    struct text_renderer : private boost::noncopyable
    {
	typedef T pixmap_type;
    
	text_renderer (pixmap_type & pixmap, face_ptr face)
	    : pixmap_(pixmap),
	      face_(face),
	      fill_(0,0,0), 
	      halo_fill_(255,255,255),
	      halo_radius_(0),
	      angle_(0.0) {}
    
	void set_pixel_size(unsigned size)
	{
	    face_->set_pixel_sizes(size);
	}
    
	void set_angle(float angle)
	{
	    angle_=angle;
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
    
	void render(std::string const& text, double x0, double y0)
	{
	    FT_Matrix matrix;
	    FT_Vector origin;
	    FT_Vector pen;
	    FT_Error  error;	
	    FT_Glyph glyph;
	
	    FT_Face face = face_->get_face();
	    FT_GlyphSlot slot = face->glyph;
	    FT_UInt glyph_index;
	    FT_Bool use_kerning;
	    FT_UInt previous = 0;
	
	    unsigned height = pixmap_.height();
	
	    origin.x = 0;
	    origin.y = 0;
	    pen.x = unsigned(x0 * 64);
	    pen.y = unsigned((height - y0) * 64);
        
	    use_kerning = FT_HAS_KERNING(face);

	    std::string::const_iterator i;
	    for (i=text.begin();i!=text.end();++i)
	    {
		matrix.xx = (FT_Fixed)( cos( angle_ ) * 0x10000L ); 
		matrix.xy = (FT_Fixed)(-sin( angle_ ) * 0x10000L ); 
		matrix.yx = (FT_Fixed)( sin( angle_ ) * 0x10000L ); 
		matrix.yy = (FT_Fixed)( cos( angle_ ) * 0x10000L );
	    	     
		FT_Set_Transform (face,&matrix,&pen);
	    
		glyph_index = FT_Get_Char_Index( face, unsigned(*i) & 0xff );
		if ( use_kerning && previous && glyph_index)
		{
		    FT_Vector delta;
		    FT_Get_Kerning(face,previous,glyph_index,
				   FT_KERNING_DEFAULT,&delta);
		    pen.x += delta.x;
		    pen.y += delta.y;
		}
	    
		error = FT_Load_Glyph (face,glyph_index,FT_LOAD_DEFAULT); 
		if ( error )
		    continue;

		error = FT_Get_Glyph( face->glyph, &glyph );
		if ( error )
		    continue;
	  
		//FT_Glyph_Transform(glyph,&matrix,&pen);
	    
		error = FT_Glyph_To_Bitmap( &glyph,FT_RENDER_MODE_NORMAL,0,1);
		if ( error )
		    continue;

		FT_BitmapGlyph bit = (FT_BitmapGlyph)glyph;
		if (halo_radius_)
		{
		    render_halo(&bit->bitmap, halo_fill_.rgba(), 
				bit->left,
				height - bit->top,halo_radius_);
		}
            	    
		render_bitmap(&bit->bitmap, fill_.rgba(), 
			      bit->left,
			      height - bit->top);

		FT_Done_Glyph(glyph);   
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	    
		previous = glyph_index;
	    
		//angle_ = sin ( 0.1 * count++);
	    }
	    std::cout << std::endl;
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
	float angle_;
    }; 
}


#endif // FONT_ENGINE_FREETYPE_HPP
