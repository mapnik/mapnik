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

//$Id: style_cache.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef STYLE_CACHE_HPP
#define STYLE_CACHE_HPP

#include "utils.hpp"
#include "style.hpp"
#include <map>
#include "feature_type_style.hpp"

namespace mapnik {
      
    class named_style_cache : public singleton <named_style_cache,CreateStatic>
    {
	friend class CreateStatic<named_style_cache>;
    private:
	static std::map<std::string,feature_type_style> styles_;  
	named_style_cache();
	~named_style_cache();
	named_style_cache(const named_style_cache&);
	named_style_cache& operator=(const named_style_cache&);
    public:
	static bool insert(const std::string& name,const feature_type_style& style);
	static void remove(const std::string& name);
	static feature_type_style find(const std::string& name);
    }; 
}


#endif //STYLE_CACHE_HPP
