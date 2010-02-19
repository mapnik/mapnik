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

//$Id: shape_index_featureset.cc 36 2005-04-05 14:32:18Z pavlenko $

#include <mapnik/feature_factory.hpp>
// boost
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include "shape_index_featureset.hpp"

using namespace boost::iostreams;

template <typename filterT>
shape_index_featureset<filterT>::shape_index_featureset(const filterT& filter,
                                                        const std::string& shape_file,
                                                        const std::set<std::string>& attribute_names,
                                                        std::string const& encoding)
    : filter_(filter),
      shape_type_(0),
      shape_(shape_file),
      tr_(new transcoder(encoding)),
      count_(0)

{
    shape_.shp().skip(100);
    stream<mapped_file_source> file(shape_file + ".index");
    if (file)
    {
	shp_index<filterT,stream<mapped_file_source> >::query(filter,file,ids_);
	file.close();
    }
    std::sort(ids_.begin(),ids_.end());    
    
#ifdef MAPNIK_DEBUG
    std::clog<< "query size=" << ids_.size() << "\n";
#endif

    itr_ = ids_.begin();

    // deal with attributes
    std::set<std::string>::const_iterator pos=attribute_names.begin();
    while (pos!=attribute_names.end())
    {
        for (int i=0;i<shape_.dbf().num_fields();++i)
        {
            if (shape_.dbf().descriptor(i).name_ == *pos)
            {
                attr_ids_.insert(i);
                break;
            }
        }
        ++pos;
    }
}

template <typename filterT>
feature_ptr shape_index_featureset<filterT>::next()
{   
    using mapnik::feature_factory;
    using mapnik::point_impl;
    if (itr_!=ids_.end())
    {
        int pos=*itr_++;
        shape_.move_to(pos);
        int type=shape_.type();
	
        feature_ptr feature(feature_factory::create(shape_.id_));
        if (type == shape_io::shape_point)
	{
            double x=shape_.shp().read_double();
            double y=shape_.shp().read_double();	    
            geometry2d * point = new point_impl;
            point->move_to(x,y);
            feature->add_geometry(point);
            ++count_;
        }
  
        else if (type == shape_io::shape_pointm)
        {
            double x=shape_.shp().read_double();
            double y=shape_.shp().read_double();
            shape_.shp().skip(8);// skip m
            geometry2d * point = new point_impl;
            point->move_to(x,y);
            feature->add_geometry(point);
            ++count_;
        }
        else if (type == shape_io::shape_pointz)
        {
            double x=shape_.shp().read_double();
            double y=shape_.shp().read_double();
            // skip z
            shape_.shp().skip(8);

            //skip m if exists
            if ( shape_.reclength_ == 8 + 36) 
            {
                shape_.shp().skip(8);
            }
            geometry2d * point = new point_impl;
            point->move_to(x,y);
            feature->add_geometry(point);
            ++count_;
        }	
        else
        {
            while(!filter_.pass(shape_.current_extent()) && 
		  itr_!=ids_.end())
            {
                pos=*itr_++;
                shape_.move_to(pos);
            }
	    
            switch (type)
            {
	    case shape_io::shape_multipoint:
	    case shape_io::shape_multipointm:
	    case shape_io::shape_multipointz:
	    {
		int num_points = shape_.shp().read_ndr_integer();
		for (int i=0; i< num_points;++i)
		{ 
		    double x=shape_.shp().read_double();
		    double y=shape_.shp().read_double();
		    geometry2d * point = new point_impl;
		    point->move_to(x,y);
		    feature->add_geometry(point);
		}
		// ignore m and z for now 
		++count_;
		break;
	    }
	    case shape_io::shape_polyline:
	    {
		geometry2d * line = shape_.read_polyline();
		feature->add_geometry(line);
		++count_;
		break;
	    }
	    case shape_io::shape_polylinem:
	    {
		geometry2d * line = shape_.read_polylinem();
		feature->add_geometry(line);
		++count_;
		break;
	    }
	    case shape_io::shape_polylinez:
	    {
		geometry2d * line = shape_.read_polylinez();
		feature->add_geometry(line);
		++count_;
		break;
	    }
	    case shape_io::shape_polygon:
	    { 
		geometry2d * poly = shape_.read_polygon();
		feature->add_geometry(poly);
		++count_;
		break;
	    }
	    case shape_io::shape_polygonm:
	    { 
		geometry2d * poly = shape_.read_polygonm();
		feature->add_geometry(poly);
		++count_;
		break;
	    }
	    case shape_io::shape_polygonz:
	    {
		geometry2d * poly = shape_.read_polygonz();
		feature->add_geometry(poly);
		++count_;
		break;
	    }
            }
        }
        if (attr_ids_.size())
        {
            shape_.dbf().move_to(shape_.id_);
            std::set<int>::const_iterator pos=attr_ids_.begin();
            while (pos!=attr_ids_.end())
            {
                try 
                {
                    shape_.dbf().add_attribute(*pos,*tr_,*feature);
                }
                catch (...)
                {
                    std::clog<<"exception caught\n";
                }
                ++pos;
            }
        }
        return feature;
    }
    else
    {

#ifdef MAPNIK_DEBUG
        std::clog<<count_<<" features\n";
#endif
        return feature_ptr();
    }
}


template <typename filterT>
shape_index_featureset<filterT>::~shape_index_featureset() {}

template class shape_index_featureset<mapnik::filter_in_box>;
template class shape_index_featureset<mapnik::filter_at_point>;

