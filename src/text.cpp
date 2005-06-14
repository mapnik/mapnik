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

//$Id: text.cpp 17 2005-03-08 23:58:43Z pavlenko $

#include "text.hpp"

#ifdef __HAVE_FREETYPE2__
namespace mapnik
{
    using namespace std;
    template <class PixBuffer>
        void TextRasterizer<PixBuffer>::render(const char* text)
    {
        FT_Library library;
        FT_Face face;
        FT_Error  error;
        error = FT_Init_FreeType( &library );
        if (error)
        {
            cout<<"an error occurred during library initialization\n";
            return;
        }
        error = FT_New_Face( library, fontName_.c_str(), 0, &face );
        if (error == FT_Err_Unknown_File_Format )
        {
            cout<<"the font file could be opened and read, but it appears\n";
            cout<<"that its font format is unsupported"<<endl;
            return;
        }
        else if ( error )
        {
            cout<<"font file could not be opened or read, or simply that it is broken\n";
            return;
        }
        error = FT_Set_Pixel_Sizes(face,24,0);
        FT_GlyphSlot slot = face->glyph;
        int x=40,y=200;
        int len=strlen(text);
        for (int i=0;i<len;++i)
        {
            FT_UInt glyph_index;
            glyph_index = FT_Get_Char_Index(face,text[i]);
            error=FT_Load_Glyph(face,glyph_index,FT_LOAD_DEFAULT);
            if (error) continue;
            error=FT_Render_Glyph( face->glyph,FT_RENDER_MODE_NORMAL);
            if (error) continue;
            render_bitmap(&slot->bitmap,x+slot->bitmap_left,y-slot->bitmap_top);
            x+=slot->advance.x>>6;
            y+=slot->advance.y>>6;
        }
        FT_Done_Face(face);
        FT_Done_FreeType(library);
    }

    template <class PixBuffer>
        void TextRasterizer<PixBuffer>::render_bitmap(FT_Bitmap *bitmap,int x,int y)
    {
        int x_max=x+bitmap->width;
        int y_max=y+bitmap->rows;
        int i,p,j,q;

        Color c(255,200,120);
        for (i=x,p=0;i<x_max;++i,++p)
        {
            for (j=y,q=0;j<y_max;++j,++q)
            {
                int gray=bitmap->buffer[q*bitmap->width+p];
                if (gray)
                {
                    pixbuf_->blendPixel(i,j,c.rgba(),gray);
                }
            }
        }
    }

    template class TextRasterizer<Image32>;
}

#endif
