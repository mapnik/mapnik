/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2007 Artem Pavlenko
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

#ifndef KISMET_TYPES_HPP
#define KISMET_TYPES_HPP

// mapnik
#include <mapnik/datasource.hpp>

// boost
#include <boost/shared_ptr.hpp>

class kismet_network_data
{
    public:
      kismet_network_data() : bestlat_(0), bestlon_(0) {}
      kismet_network_data(std::string ssid, std::string bssid, 
                          double bestlat, double bestlon) :
                          ssid_(ssid), bssid_(bssid), 
                          bestlat_(bestlat), bestlon_(bestlon) {}
    
      std::string ssid_;
      std::string bssid_;
      double bestlat_;
      double bestlon_;
};

#endif //KISMET_TYPES_HPP

