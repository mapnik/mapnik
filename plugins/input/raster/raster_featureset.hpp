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
#ifndef RASTER_FEATURESET_HPP
#define RASTER_FEATURESET_HPP

#include <vector>

#include "raster_datasource.hpp"
#include "raster_info.hpp"

// boost
#include <boost/utility.hpp>


class single_file_policy
{
    raster_info info_;
public:
    class const_iterator
    {
        enum iterator_e {start,end};
        bool status_;
        const single_file_policy* p_;
    public:
        explicit const_iterator(const single_file_policy* p)
            :status_(start),
             p_(p) {}

        const_iterator()
            :status_(end){}

        const_iterator(const const_iterator& other)
            :status_(other.status_),
             p_(other.p_) {}

        const_iterator& operator++()
        {
            status_=end;
            return *this;
        }

        const raster_info& operator*() const
        {
            return p_->info_;
        }

        const raster_info* operator->() const
        {
            return &(p_->info_);
        }

        bool operator!=(const const_iterator& itr)
        {
            return status_!=itr.status_;
        }
    };

    explicit single_file_policy(const raster_info& info)
        :info_(info) {}

    const_iterator begin()
    {
        return const_iterator(this);
    }

    const_iterator query(const Envelope<double>& box)
    {
        if (box.intersects(info_.envelope()))
        {
            return begin();
        }
        return end();
    }

    const_iterator end()
    {
        return const_iterator();
    }
};

class tiled_file_policy
{
public:
   
   typedef std::vector<raster_info>::const_iterator const_iterator;
   
   tiled_file_policy(std::string const& file, std::string const& format, unsigned tile_size, 
                     Envelope<double> extent, Envelope<double> bbox,unsigned width, unsigned height)
   {     
      
      double lox = extent.minx();
      double loy = extent.miny();
   
      int max_x = int(std::ceil(double(width)/double(tile_size)));
      int max_y = int(std::ceil(double(height)/double(tile_size)));

      double pixel_x = extent.width()/double(width);
      double pixel_y = extent.height()/double(height);

#ifdef MAPNIK_DEBUG 
      std::cout << "Raster Plugin: PIXEL SIZE("<< pixel_x << "," << pixel_y << ")\n";
#endif

      Envelope<double> e = bbox.intersect(extent);
      
      for (int x = 0 ; x < max_x ; ++x)
      {
         for (int y = 0 ; y < max_y ; ++y)
         {
            double x0 = lox + x*tile_size*pixel_x;
            double y0 = loy + y*tile_size*pixel_y;
            double x1 = x0 + tile_size*pixel_x;
            double y1 = y0 + tile_size*pixel_y;
            
            if (e.intersects(Envelope<double>(x0,y0,x1,y1)))
            {
               Envelope<double> tile_box = e.intersect(Envelope<double>(x0,y0,x1,y1));            
               raster_info info(file,format,tile_box,tile_size,tile_size);
               infos_.push_back(info);
            }
         }
      }
#ifdef MAPNIK_DEBUG 
      std::cout << "Raster Plugin: INFO SIZE=" << infos_.size() << " " << file << "\n";
#endif
   }
   
   const_iterator begin()
   {
      return infos_.begin();
   }
      
   const_iterator end()
   {
      return infos_.end();
   }
   
private:

   std::vector<raster_info> infos_;
};


template <typename LookupPolicy>
class raster_featureset : public mapnik::Featureset
{
   typedef typename LookupPolicy::const_iterator iterator_type;
   LookupPolicy policy_;
   size_t id_;
   mapnik::Envelope<double> extent_;
   mapnik::Envelope<double> bbox_;
   iterator_type curIter_;
   iterator_type endIter_;
public:
   raster_featureset(LookupPolicy const& policy,Envelope<double> const& exttent, mapnik::query const& q);
   virtual ~raster_featureset();
   mapnik::feature_ptr next();
};

#endif //RASTER_FEATURESET_HPP
