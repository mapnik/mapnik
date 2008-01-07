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

#ifndef MAPNIK_CONFIG_ERROR_INCLUDED
#define MAPNIK_CONFIG_ERROR_INCLUDED

#include <iostream>
#include <sstream>

namespace mapnik {

    class config_error : public std::exception
    {
        public:
            config_error() {}

            config_error( const std::string & what ) :
                what_( what )
            {
            }
            virtual ~config_error() throw() {};

            virtual const char * what() const throw()
            {
                return what_.c_str();    
            }

            void append_context(const std::string & ctx) const
            {
                what_ += " " + ctx;
            }

        protected:
            mutable std::string what_;
    };
}

#endif // MAPNIK_CONFIG_ERROR_INCLUDED
