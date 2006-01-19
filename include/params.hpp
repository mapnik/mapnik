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

//$Id: params.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef PARAMS_HPP
#define PARAMS_HPP

#include <map>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/base_object.hpp>

namespace mapnik
{

    typedef std::pair<const std::string,std::string> parameter;
    typedef std::map<const std::string,std::string> param_map;

    class parameters : public param_map
    {
	friend class boost::serialization::access;
	
	template <typename Archive>
	void save(Archive & ar, const unsigned int /*version*/) const
	{
	    const size_t size = param_map::size();
	    ar & boost::serialization::make_nvp("count",size);
	    param_map::const_iterator itr;
	    for (itr=param_map::begin();itr!=param_map::end();++itr)
	    {
		ar & boost::serialization::make_nvp("name",itr->first);
		ar & boost::serialization::make_nvp("value",itr->second);
	    }
	}
	
	template <typename Archive>
	void load(Archive & ar, const unsigned int /*version*/)
	{	    
	    size_t size;
	    ar & boost::serialization::make_nvp("size",size);
	    for (size_t i=0;i<size;++i)
	    {
		std::string name;
		std::string value;
		ar & boost::serialization::make_nvp("name",name);
		ar & boost::serialization::make_nvp("value",value);
		param_map::insert(make_pair(name,value));
	    }
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
    public:
	parameters() {}
	const std::string get(std::string const& key) const
	{
	    param_map::const_iterator itr=find(key);
	    if (itr != end())
	    {
		return itr->second;
	    }
	    return std::string();
	}
    };
}

BOOST_CLASS_IMPLEMENTATION(mapnik::parameter, boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(mapnik::parameter, boost::serialization::track_never)

BOOST_CLASS_IMPLEMENTATION(mapnik::parameters, boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(mapnik::parameters, boost::serialization::track_never)

#endif //PARAMS_HPP
