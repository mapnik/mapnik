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

//$Id: attribute_container.hh 58 2004-10-31 16:21:26Z artem $

#ifndef ATTRIBUTE_CONTAINER_HH
#define ATTRIBUTE_CONTAINER_HH

#include <string>
#include <map>
#include "attribute.hh"

namespace mapnik
{
    class attribute_container
    {
        private:
            static const invalid_attribute invalid_;
            std::map<std::string,attribute_base*> attr_;
        public:
            attribute_container();
            virtual ~attribute_container();
            void add(const std::string& name,const attribute_base& a);
            const attribute_base& get(const std::string& name) const;
        private:
            attribute_container(const attribute_container&);
            attribute_container& operator=(const attribute_container&);
    };
}
#endif                                            //ATTRIBUTE_CONTAINER
