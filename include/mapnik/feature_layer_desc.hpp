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

//$Id$

#ifndef FEATURE_LAYER_DESC_HPP
#define FEATURE_LAYER_DESC_HPP

#include <string>
#include <vector>
#include <iostream>

namespace mapnik
{
    
    using std::string;
    using std::vector;
    using std::clog;
    using std::endl;
    
    enum {
        Integer=1,
        Float  =2,
        Double =3,
        String =4,
        Geometry=5,
        Object=6 
    };
    
    class attribute_descriptor
    {
    public:
        attribute_descriptor(string const& name,unsigned type,
                             bool primary_key=false,
                             int size=-1,
                             int precision=-1)
            : name_(name),
              type_(type),
              primary_key_(primary_key),
              size_(size),
              precision_(precision) {}
	      
        attribute_descriptor(attribute_descriptor const& other)
            : name_(other.name_),
              type_(other.type_),
              primary_key_(other.primary_key_),
              size_(other.size_),
              precision_(other.precision_) {}

        attribute_descriptor& operator=(attribute_descriptor const& other)
        {
            if (this == &other)
                return *this;	    
            name_=other.name_;
            type_=other.type_;
            primary_key_=other.primary_key_;
            size_=other.size_;
            precision_=other.precision_;
            return *this;
        }
        string const& get_name() const
        {
            return name_;
        }
        unsigned  get_type() const
        {
            return type_;
        }
        bool is_primary_key() const
        {
            return primary_key_;
        }
        int get_size() const
        {
            return size_;
        } 
	
        int get_precision() const
        {
            return precision_;
        }
    private:
        string name_;
        int type_;
        bool primary_key_;
        int size_;
        int precision_;
    };
     
    template <typename charT,typename traits>
    inline std::basic_ostream<charT,traits>&
    operator << (std::basic_ostream<charT,traits>& out,
                 attribute_descriptor const& ad)
    {
        out << "name=" << ad.get_name() << endl;
        out << "type=" << ad.get_type() << endl;
        out << "size=" << ad.get_size() << endl;
        return out;
    }

    class layer_descriptor 
    {
    public:
        layer_descriptor(string const& name,int srid=-1)
            : name_(name),
              srid_(srid) {}

        layer_descriptor(layer_descriptor const& other)
            : name_(other.name_),
              srid_(other.srid_),
              desc_ar_(other.desc_ar_) {}
	
        void set_name(string const& name)
        {
            name_=name;
        }
        string const& get_name() const
        {
            return name_;
        }
	
        void set_srid(int srid) 
        {
            srid_=srid;
        }
	
        int get_srid() const
        {
            return srid_;
        }

        void add_descriptor(attribute_descriptor const& desc)
        {
            desc_ar_.push_back(desc);
        }
	
        vector<attribute_descriptor> const& get_descriptors() const
        {
            return desc_ar_;
        }	
        vector<attribute_descriptor>& get_descriptors()
        {
            return desc_ar_;
        }
    private:
        string name_;
        int srid_;
        vector<attribute_descriptor> desc_ar_;
    };
    
    template <typename charT,typename traits>
    inline std::basic_ostream<charT,traits>&
    operator << (std::basic_ostream<charT,traits>& out,
                 layer_descriptor const& ld)
    {
        out << "name=" << ld.get_name() << endl;
        out << "srid=" << ld.get_srid() << endl;
        vector<attribute_descriptor> const& desc_ar=ld.get_descriptors();
        vector<attribute_descriptor>::const_iterator pos=desc_ar.begin();
        while (pos != desc_ar.end())
        {
            out << *pos++ << endl;
	    
        }
        return out;
    }
    /*    
          bool layer_descriptor_to_wkb(layer_descriptor const& desc,array<>& wkb)
          {
          //srid	
          int srid = desc.get_srid();
          wkb.write(&srid,sizeof(int));

          //attribute descriptors
          vector<attribute_descriptor> const& desc_ar = desc.get_descriptors();
          vector<attribute_descriptor>::const_iterator itr=desc_ar.begin();
          size_t num_desc = desc_ar.size();
          wkb.write(&num_desc,sizeof(int));
	
          while (itr != desc_ar.end())
          {
          string name = itr->get_name();
          wkb.write(name.c_str(),name.size()+1);
	       
          unsigned type = static_cast<int>(itr->get_type());
          wkb.write(&type,sizeof(unsigned));

          bool prim_key = itr->is_primary_key();
          wkb.write(&prim_key,sizeof(bool));

          int size = itr->get_size();
          wkb.write(&size,sizeof(int));
            
          ++itr;
          }	
          return true;
          }
    
          bool layer_descriptor_from_wkb(const char* wkb, layer_descriptor &desc)
          {
          unsigned pos=0;
          int srid;
	
          memcpy(&srid,wkb+pos,sizeof(int));
          desc.set_srid(srid);
          pos+=sizeof(int);
	
          int num_desc;
          memcpy(&num_desc,wkb+pos,sizeof(int));
          pos+=sizeof(int);
	
          for (int i=0;i<num_desc;++i)
          {
          string name = wkb+pos;
          pos += name.size()+1;
          //std::clog<<"name="<<name<<"\n";
	    
          int type;
          memcpy(&type,wkb+pos,sizeof(int));
          pos += sizeof(int);
          attribute_descriptor ad(name,type);
          desc.add_descriptor(ad);
          //todo!!!
          pos += 4+1;
          }
          return true;
          }
    */    
}

#endif //FEATURE_LAYER_DESC_HPP
