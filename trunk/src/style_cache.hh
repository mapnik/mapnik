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

#ifndef STYLE_CACHE_HH
#define STYLE_CACHE_HH

#include "utils.hh"
#include "ptr.hh"
#include "style.hh"
#include <map>

namespace mapnik {
    
    class style_cache :  public singleton <style_cache,CreateStatic>
    {
	friend class CreateStatic<style_cache>;
    private:
	static std::map<std::string,Style > styles_;  
	style_cache();
	~style_cache();
	style_cache(const style_cache&);
	style_cache& operator=(const style_cache&);
    public:
	static bool insert(const std::string& name,const Style& style);
	static void remove(const std::string& name);
	static const Style& find(const std::string& name);
    };
}


#endif //STYLE_CACHE_HH
