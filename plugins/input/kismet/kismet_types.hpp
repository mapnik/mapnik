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

#ifndef KISMET_TYPES_HPP
#define KISMET_TYPES_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>

// boost
#include <boost/shared_ptr.hpp>

// this is a copy from packet.h from kismet 2007.10.R1
enum crypt_type
{
    crypt_none = 0,
    crypt_unknown = 1,
    crypt_wep = 2,
    crypt_layer3 = 4,
    // Derived from WPA headers
    crypt_wep40 = 8,
    crypt_wep104 = 16,
    crypt_tkip = 32,
    crypt_wpa = 64,
    crypt_psk = 128,
    crypt_aes_ocb = 256,
    crypt_aes_ccm = 512,
    // Derived from data traffic
    crypt_leap = 1024,
    crypt_ttls = 2048,
    crypt_tls = 4096,
    crypt_peap = 8192,
    crypt_isakmp = 16384,
    crypt_pptp = 32768,
    crypt_ccmp = 65536
};

class kismet_network_data
{
public:
    kismet_network_data()
        : bestlat_(0), bestlon_(0), crypt_(crypt_none)
    {
    }

    kismet_network_data(std::string ssid,
                        std::string bssid,
                        double bestlat,
                        double bestlon,
                        int crypt)
        : ssid_(ssid),
          bssid_(bssid),
          bestlat_(bestlat),
          bestlon_(bestlon),
          crypt_(crypt)
    {
    }

    std::string const& ssid() const
    {
        return ssid_;
    }

    std::string const& bssid() const
    {
        return bssid_;
    }

    double bestlat() const
    {
        return bestlat_;
    }

    double bestlon() const
    {
        return bestlon_;
    }

    int crypt() const
    {
        return crypt_;
    }

protected:
    std::string ssid_;
    std::string bssid_;
    double bestlat_;
    double bestlon_;
    int crypt_;
};

#endif // KISMET_TYPES_HPP
