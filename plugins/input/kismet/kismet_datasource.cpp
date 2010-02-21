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
// $Id$

#include "kismet_datasource.hpp"
#include "kismet_featureset.hpp"

// network
#include  <netdb.h>
#include  <arpa/inet.h>
#include  <netinet/in.h>
#include  <sys/socket.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <fcntl.h>
#include  <unistd.h>
#include  <stdio.h>

// mapnik
#include <mapnik/ptree_helpers.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/bind.hpp>

#define MAX_TCP_BUFFER 4096 // maximum accepted TCP data block size

// If you change this also change the according sscanf line!
#define MAX_KISMET_LINE 1024 // maximum length of a kismet command (assumed)

using boost::lexical_cast;
using boost::bad_lexical_cast;

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(kismet_datasource)

using mapnik::Envelope;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;
using mapnik::datasource_exception;

using namespace std;

boost::mutex knd_list_mutex;
std::list<kismet_network_data> knd_list;
const unsigned int queue_size = 20;

kismet_datasource::kismet_datasource(parameters const& params)
   : datasource(params),
     extent_(),
     extent_initialized_(false),
     type_(datasource::Vector),
     desc_(*params.get<std::string>("type"), 
     *params.get<std::string>("encoding","utf-8"))
{
    //cout << "kismet_datasource::kismet_datasource()" << endl;
  
    boost::optional<std::string> host = params.get<std::string>("host");
    if (!host) throw datasource_exception("missing <host> parameter");
  
    boost::optional<std::string> port = params.get<std::string>("port");
    if (!port) throw datasource_exception("missing <port> parameter");
  
    unsigned int portnr = atoi ((*port).c_str () );
    kismet_thread.reset (new boost::thread (boost::bind (&kismet_datasource::run, this, *host, portnr)));

    boost::optional<std::string> ext  = params_.get<std::string>("extent");
    if (ext)
    {
        boost::char_separator<char> sep(",");
        boost::tokenizer<boost::char_separator<char> > tok(*ext,sep);
        unsigned i = 0;
        bool success = false;
        double d[4];
        for (boost::tokenizer<boost::char_separator<char> >::iterator beg=tok.begin(); 
             beg!=tok.end();++beg)
        {
           try 
           {
               d[i] = boost::lexical_cast<double>(*beg);
           }
           catch (boost::bad_lexical_cast & ex)
           {
              std::clog << ex.what() << "\n";
              break;
           }
           if (i==3) 
           {
              success = true;
              break;
           }
           ++i;
        }
        if (success)
        {
           extent_.init(d[0],d[1],d[2],d[3]);
           extent_initialized_ = true;
        }
    }
}

kismet_datasource::~kismet_datasource()
{
}

std::string const kismet_datasource::name_="kismet";

std::string kismet_datasource::name()
{
   return name_;
}

int kismet_datasource::type() const
{
   return type_;
}

Envelope<double> kismet_datasource::envelope() const
{
   //cout << "kismet_datasource::envelope()" << endl;
   return extent_;
}

layer_descriptor kismet_datasource::get_descriptor() const
{
   return desc_;
}

featureset_ptr kismet_datasource::features(query const& q) const
{
    //cout << "kismet_datasource::features()" << endl;
    
    // TODO: use Envelope to filter bbox before adding to featureset_ptr
    //mapnik::Envelope<double> const& e = q.get_bbox();

    boost::mutex::scoped_lock lock(knd_list_mutex);
    return featureset_ptr (new kismet_featureset(knd_list, desc_.get_encoding()));

    // TODO: if illegal:
    // return featureset_ptr();
}

featureset_ptr kismet_datasource::features_at_point(coord2d const& pt) const
{
    //cout << "kismet_datasource::features_at_point()" << endl;

#if 0
   if (dataset_ && layer_)
   {
        OGRPoint point;
        point.setX (pt.x);
        point.setY (pt.y);
        
        layer_->SetSpatialFilter (&point);
        
        return featureset_ptr(new ogr_featureset(*dataset_, *layer_, desc_.get_encoding()));
   }
#endif

   return featureset_ptr();
}

void kismet_datasource::run (const std::string &ip_host, const unsigned int port)
{
  //cout << "+run" << endl;
  
  int                 sockfd, n;
  struct sockaddr_in  sock_addr;
  struct in_addr      inadr;
  struct hostent      *host;
  char                buffer[MAX_TCP_BUFFER]; // TCP data send from kismet_server
  string command;

  if (inet_aton(ip_host.c_str (), &inadr))
  {
    host = gethostbyaddr((char *) &inadr, sizeof(inadr), AF_INET);
  }  
  else
  {
    host = gethostbyname(ip_host.c_str ());
  }

  if (host == NULL)
  {
    herror ("plugins/input/kismet: Error while searching host");
    return;
  }

  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port   = htons(port);
  memcpy(&sock_addr.sin_addr, host->h_addr_list[0],
         sizeof(sock_addr.sin_addr));

  if ( (sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    cerr << "plugins/input/kismet: Error while creating socket" << endl;
    return;
  }

  if (connect(sockfd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)))
  {
    cerr << "plugins/input/kismet: Error while connecting" << endl;
    return;
  }

  command = "!1 ENABLE NETWORK ssid,bssid,wep,bestlat,bestlon\n";

  if (write(sockfd, command.c_str (), command.length ()) != (signed) command.length ())
  {
    cerr << "plugins/input/kismet: Error sending command to " << ip_host << endl;
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
  while ( (n = read(sockfd, buffer, sizeof(buffer))) > 0)
  {
    assert (n < MAX_TCP_BUFFER);
    
    buffer[n] = '\0';
    string bufferObj (buffer); // TCP data send from kismet_server as STL string
    
    //cout << "BufferObj: " << endl << bufferObj << "[END]" << endl;
    
    string::size_type found = 0;
    string::size_type search_start = 0;
    string kismet_line; // contains a line from kismet_server
    do 
    {
      found = bufferObj.find ('\n', search_start);
      if (found != string::npos)
      {
        kismet_line.assign (bufferObj, search_start, found - search_start);
        
        //cout << "Line: " << kismet_line << "[ENDL]" << endl;
        
        int param_number = 5; // the number of parameters to parse
        
        // Attention: string length specified to the constant!
        if (sscanf (kismet_line.c_str (), "*NETWORK: \001%1024[^\001]\001 %1024s %d %lf %lf", ssid, bssid, &crypt, &bestlat, &bestlon) == param_number)
        {
          //printf ("ssid=%s, bssid=%s, crypt=%d, bestlat=%f, bestlon=%f\n", ssid, bssid, crypt, bestlat, bestlon);

          kismet_network_data knd (ssid, bssid, bestlat, bestlon, crypt);
          
          boost::mutex::scoped_lock lock(knd_list_mutex);
                      
          // the queue only grows to a max size
          if (knd_list.size () >= queue_size)
          {
            knd_list.pop_front ();
          }
          
          knd_list.push_back (knd);
        }  
        else
        {
          // do nothing if not matched!
        }
        
        search_start = found + 1;      
      }
    }
    while (found != string::npos);
  }

  if (n < 0)
  {
    cerr << "plugins/input/kismet: Error while reading from socket" << endl;
  }

  close(sockfd);
  
  //cout << "-run" << endl;
}
