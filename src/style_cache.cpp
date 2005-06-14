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

//$Id: style_cache.cpp 37 2005-04-07 17:09:38Z pavlenko $

#include "style_cache.hpp"
#include "line_symbolizer.hpp"

namespace mapnik 
{
    
    style_cache::style_cache() {}
    style_cache::~style_cache() {}
    
    std::map<std::string,Style> style_cache::styles_;

    bool style_cache::insert(const std::string& name,const Style& style) 
    {
	Lock lock(&mutex_);
	return styles_.insert(make_pair(name,style)).second;
    }
    
    void style_cache::remove(const std::string& name) 
    {
	Lock lock(&mutex_);
	styles_.erase(name);
    }
    
    const Style& style_cache::find(const std::string& name) 
    {
	Lock lock(&mutex_);
	std::map<std::string,Style>::iterator itr=styles_.find(name);
	if (itr!=styles_.end()) 
	{
	    return itr->second;
	}
	static const Style default_style(ref_ptr<symbolizer>(new line_symbolizer(Color(255,0,0))));
	return default_style;
    }
    ////////////////////////////////////////////////////////////////////////////    
    named_style_cache::named_style_cache() {}
    named_style_cache::~named_style_cache() {}
    
    std::map<std::string,feature_type_style> named_style_cache::styles_;

    bool named_style_cache::insert(const std::string& name,const feature_type_style& style) 
    {
	Lock lock(&mutex_);
	return styles_.insert(make_pair(name,style)).second;
    }
    
    void named_style_cache::remove(const std::string& name) 
    {
	Lock lock(&mutex_);
	styles_.erase(name);
    }
    
    feature_type_style named_style_cache::find(const std::string& name)
    {
	Lock lock(&mutex_);
	std::map<std::string,feature_type_style>::iterator itr=styles_.find(name);
	if (itr!=styles_.end()) 
	{
	    return itr->second;
	}
	return feature_type_style();
    }
}
