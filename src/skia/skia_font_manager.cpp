/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#include <mapnik/skia/skia_font_manager.hpp>
#include <mapnik/skia/skia_typeface_cache.hpp>
#include <mapnik/unicode.hpp>

#include <SkTypeface.h>

#include <iostream>
#include <vector>

namespace mapnik {

void skia_font_manager::test(std::string const& family_name, UnicodeString & ustr)
{
    SkTypeface * typeface = cache_.create(family_name);
//SkTypeface * typeface = SkTypeface::CreateFromName(family_name.c_str(),SkTypeface::kNormal);
    if (typeface)
    {
        std::cerr << "ustr.length()=" << ustr.length() << std::endl;
        std::vector<uint16_t> glyph_ids;
        glyph_ids.resize(ustr.length());
        std::cerr << (char*)ustr.getTerminatedBuffer() << std::endl;
        int num_ids = typeface->charsToGlyphs((void*)ustr.getTerminatedBuffer(), SkTypeface::kUTF16_Encoding,
                                              &glyph_ids[0], glyph_ids.size());

        std::cerr << "num_ids = " << num_ids << std::endl;
        std::vector<uint32_t> glyph_32_ids;
        for (std::size_t i = 0; i < glyph_ids.size() ;++i)
        {
            glyph_32_ids.push_back(glyph_ids[i]);
        }
        //const uint32_t glyph_id = 0;
//        SkAdvancedTypefaceMetrics * metrics =
        //          typeface->getAdvancedTypefaceMetrics(SkAdvancedTypefaceMetrics::kHAdvance_PerGlyphInfo, &glyph_32_ids[0], glyph_32_ids.size());


        //auto const& advances = metrics->fGlyphWidths->fAdvance;
        //std::cerr << "count = " << advances.count();
        //for ( unsigned j=0; j< num_ids; ++j)
        //{
        //   std::cerr << advances[j] << ",";
        //}
        //std::cerr << std::endl;
        //for (size_t i = 0; i<metrics.fGlyphWidths.
        //std::cerr << "SkAdvancedTypefaceMetrics: fFontName=" << metrics->fFontName.c_str() << std::endl;
        //std::cerr << "SkAdvancedTypefaceMetrics: fAscent=" << metrics->fAscent << std::endl;
        //std::cerr << "SkAdvancedTypefaceMetrics: fCapHeight=" << metrics->fCapHeight << std::endl;


    }
    else
    {
        std::cerr << "FAIL" << std::endl;
    }
}

}
