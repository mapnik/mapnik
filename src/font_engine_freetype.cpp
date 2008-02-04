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

#include <mapnik/font_engine_freetype.hpp>

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
   
   bool freetype_engine::register_font(std::string const& file_name)
   {
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
#ifdef MAPNIK_THREADSAFE
   boost::mutex freetype_engine::mutex_;
#endif
   std::map<std::string,std::string> freetype_engine::name2file_;
}
