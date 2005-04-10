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

#ifndef TEXT_HH
#define TEXT_HH

#ifdef __HAVE_FREETYPE2__ //TODO:: fix configure.ac AP_CHECK_FREETYPE2
 
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <string>
#include "style.hh"
#include "graphics.hh"

namespace mapnik
{
    template <typename PixBuffer> class TextRasterizer
    {
        private:
            PixBuffer* pixbuf_;
            std::string fontName_;
        public:
            TextRasterizer(PixBuffer& pixbuf,const char* fontName)
                :pixbuf_(&pixbuf),
                fontName_(fontName) {}
            void render(const char* text);
        private:
            TextRasterizer(const TextRasterizer&);
            TextRasterizer& operator=(const TextRasterizer&);
            void render_bitmap(FT_Bitmap *bitmap,int x,int y);
    };
}
#endif
#endif                                            //TEXT_HH
