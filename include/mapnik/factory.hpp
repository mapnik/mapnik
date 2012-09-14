/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_FACTORY_HPP
#define MAPNIK_FACTORY_HPP

// mapnik
#include <mapnik/utils.hpp>

// stl
#include <stdexcept>
#include <map>

namespace mapnik {
template <typename key_type,
          typename product_type>
class default_factory_error
{
public:
    struct factory_exception : public std::exception
    {
        const char* what() const throw()
        {
            return "uknown object type";
        }
    };
    static product_type* on_unknown_type(const key_type&)
    {
        return 0;
    }
};

template
<
typename product_type,
typename key_type,
typename product_creator=product_type* (*)(),
template <typename,typename> class factory_error_policy=default_factory_error
>
class factory : public singleton<factory <product_type,
                                          key_type,
                                          product_creator,factory_error_policy> >,
                factory_error_policy <key_type,product_type>
{
private:
    typedef std::map<key_type,product_creator> product_map;
    product_map map_;
public:

    bool register_product(const key_type& key,product_creator creator)
    {
        return map_.insert(typename product_map::value_type(key,creator)).second;
    }

    bool unregister_product(const key_type& key)
    {
        return map_.erase(key)==1;
    }

    product_type* create_object(const key_type& key,std::string const& file)
    {
        typename product_map::const_iterator pos=map_.find(key);
        if (pos!=map_.end())
        {
            return (pos->second)(file);
        }
        return factory_error_policy<key_type,product_type>::on_unknown_type(key);
    }
};
}

#endif // MAPNIK_FACTORY_HPP
