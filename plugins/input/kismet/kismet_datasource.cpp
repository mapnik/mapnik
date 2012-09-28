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

// network
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

// mapnik
#include <mapnik/boolean.hpp>
#include <mapnik/debug.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include "kismet_datasource.hpp"
#include "kismet_featureset.hpp"

#define MAX_TCP_BUFFER 4096 // maximum accepted TCP data block size

// If you change this also change the according kismet command length !
#define MAX_KISMET_LINE 1024 // maximum length of a kismet command (assumed)
#define KISMET_COMMAND  "*NETWORK: \001%1024[^\001]\001 %1024s %d %lf %lf"

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(kismet_datasource)

using mapnik::box2d;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;
using mapnik::datasource_exception;

boost::mutex knd_list_mutex;
std::list<kismet_network_data> knd_list;
const unsigned int queue_size = 20;

kismet_datasource::kismet_datasource(parameters const& params, bool bind)
    : datasource(params),
      extent_(),
      extent_initialized_(false),
      type_(datasource::Vector),
      srs_("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"),
      desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8"))
{
    boost::optional<std::string> host = params_.get<std::string>("host");
    if (host)
    {
        host_ = *host;
    }
    else
    {
        throw datasource_exception("Kismet Plugin: missing <host> parameter");
    }

    boost::optional<unsigned int> port = params_.get<unsigned int>("port", 2501);
    if (port)
    {
        port_ = *port;
    }

    boost::optional<std::string> srs = params_.get<std::string>("srs");
    if (srs)
    {
        srs_ = *srs;
    }

    boost::optional<std::string> ext = params_.get<std::string>("extent");
    if (ext)
    {
        extent_initialized_ = extent_.from_string(*ext);
    }

    kismet_thread.reset(new boost::thread(boost::bind(&kismet_datasource::run, this, host_, port_)));

    if (bind)
    {
        this->bind();
    }
}

void kismet_datasource::bind() const
{
    if (is_bound_) return;

    is_bound_ = true;
}

kismet_datasource::~kismet_datasource()
{
}

const char * kismet_datasource::name()
{
    return "kismet";
}

mapnik::datasource::datasource_t kismet_datasource::type() const
{
    return type_;
}

box2d<double> kismet_datasource::envelope() const
{
    if (! is_bound_) bind();
    return extent_;
}

boost::optional<mapnik::datasource::geometry_t> kismet_datasource::get_geometry_type() const
{
    return boost::optional<mapnik::datasource::geometry_t>(mapnik::datasource::Point);
}

layer_descriptor kismet_datasource::get_descriptor() const
{
    return desc_;
}

featureset_ptr kismet_datasource::features(query const& q) const
{
    if (! is_bound_) bind();

    MAPNIK_LOG_DEBUG(kismet) << "kismet_datasource::features()";

    // TODO: use box2d to filter bbox before adding to featureset_ptr
    // mapnik::box2d<double> const& e = q.get_bbox();

    boost::mutex::scoped_lock lock(knd_list_mutex);
    return boost::make_shared<kismet_featureset>(knd_list,
                                                 srs_,
                                                 desc_.get_encoding());

    // TODO: if illegal:
    // return featureset_ptr();
}

featureset_ptr kismet_datasource::features_at_point(coord2d const& pt, double tol) const
{
    if (! is_bound_) bind();

    MAPNIK_LOG_DEBUG(kismet) << "kismet_datasource::features_at_point()";

    return featureset_ptr();
}

void kismet_datasource::run(std::string const& ip_host, const unsigned int port)
{
    MAPNIK_LOG_DEBUG(kismet) << "kismet_datasource: Enter run";

    int sockfd, n;
    struct sockaddr_in sock_addr;
    struct in_addr inadr;
    struct hostent* host;
    char buffer[MAX_TCP_BUFFER]; // TCP data send from kismet_server
    std::string command;

    if (inet_aton(ip_host.c_str(), &inadr))
    {
        host = gethostbyaddr((char*)&inadr, sizeof(inadr), AF_INET);
    }
    else
    {
        host = gethostbyname(ip_host.c_str());
    }

    if (host == NULL)
    {
        MAPNIK_LOG_ERROR(kismet) << "Kismet Plugin: error while searching host";
        return;
    }

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    memcpy(&sock_addr.sin_addr, host->h_addr_list[0], sizeof(sock_addr.sin_addr));

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        MAPNIK_LOG_ERROR(kismet) << "Kismet Plugin: error while creating socket";
        return;
    }

    if (connect(sockfd, (struct sockaddr*) &sock_addr, sizeof(sock_addr)))
    {
        MAPNIK_LOG_ERROR(kismet) << "Kismet Plugin: Error while connecting";
        return;
    }

    command = "!1 ENABLE NETWORK ssid,bssid,wep,bestlat,bestlon\n";

    if (write(sockfd, command.c_str(), command.length()) != (signed)command.length())
    {
        MAPNIK_LOG_ERROR(kismet) << "Kismet Plugin: Error sending command to " << ip_host;

        close(sockfd);
        return;
    }

    char ssid[MAX_KISMET_LINE] = {};
    char bssid[MAX_KISMET_LINE] = {};
    double bestlat = 0;
    double bestlon = 0;
    int crypt = crypt_none;

    // BUG: if kismet_server is active sending after mapnik was killed and then restarted the
    // assert is called. Needs to be analyzed!
    while ((n = read(sockfd, buffer, sizeof(buffer))) > 0)
    {
        assert(n < MAX_TCP_BUFFER);

        buffer[n] = '\0';
        std::string bufferObj(buffer); // TCP data send from kismet_server as STL string

        MAPNIK_LOG_DEBUG(kismet) << "kismet_datasource: buffer_obj=" << bufferObj;

        std::string::size_type found = 0;
        std::string::size_type search_start = 0;
        std::string kismet_line; // contains a line from kismet_server
        do
        {
            found = bufferObj.find('\n', search_start);
            if (found != std::string::npos)
            {
                kismet_line.assign(bufferObj, search_start, found - search_start);

                MAPNIK_LOG_DEBUG(kismet) << "kismet_datasource: line=" << kismet_line;

                int param_number = 5; // the number of parameters to parse

                // Attention: string length specified to the constant!
                if (sscanf (kismet_line.c_str(),
                            KISMET_COMMAND,
                            ssid,
                            bssid,
                            &crypt,
                            &bestlat,
                            &bestlon) == param_number)
                {
                    MAPNIK_LOG_DEBUG(kismet) << "kismet_datasource: ssid=" << ssid
                                             << ", bssid=" << bssid
                                             << ", crypt=" << crypt
                                             << ", bestlat=" << bestlat
                                             << ", bestlon=" << bestlon;

                    kismet_network_data knd(ssid, bssid, bestlat, bestlon, crypt);

                    boost::mutex::scoped_lock lock(knd_list_mutex);

                    // the queue only grows to a max size
                    if (knd_list.size () >= queue_size)
                    {
                        knd_list.pop_front();
                    }

                    knd_list.push_back(knd);
                }
                else
                {
                    // do nothing if not matched!
                }

                search_start = found + 1;
            }
        }
        while (found != std::string::npos);
    }

    if (n < 0)
    {
        MAPNIK_LOG_ERROR(kismet) << "Kismet Plugin: error while reading from socket";
    }

    close(sockfd);

    MAPNIK_LOG_DEBUG(kismet) << "kismet_datasource: Exit run";
}

