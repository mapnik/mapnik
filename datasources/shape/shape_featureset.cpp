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

#include "shape_featureset.hpp"
#include <iostream>

template <typename filterT>
shape_featureset<filterT>::shape_featureset(const filterT& filter, 
					    const std::string& shape_file,
					    const std::set<std::string>& attribute_names,
					    long file_length )
    : filter_(filter),
      shape_type_(shape_io::shape_null),
      shape_(shape_file),
      query_ext_(),
      file_length_(file_length),
      count_(0)
{
    shape_.shp().skip(100);
    //attributes
    typename std::set<std::string>::const_iterator pos=attribute_names.begin();
    while (pos!=attribute_names.end())
    {
	for (int i=0;i<shape_.dbf().num_fields();++i)
	{
	    if (shape_.dbf().descriptor(i).name_ == *pos)
	    {
		attr_ids_.push_back(i);
		break;
	    }
	}
	++pos;
    }
}


template <typename filterT>
feature_ptr shape_featureset<filterT>::next()
{
    std::streampos pos=shape_.shp().pos();
    
    if (pos < std::streampos(file_length_ * 2))
    {
        shape_.move_to(pos);
	int type=shape_.type();
	feature_ptr feature(new Feature(shape_.id_));
	if (type == shape_io::shape_point)
	{
	    double x=shape_.shp().read_double();
	    double y=shape_.shp().read_double();
	    geometry_ptr point(new point_impl(-1));
	    point->move_to(x,y);
	    feature->set_geometry(point);
	    ++count_;
	}
	else if (type == shape_io::shape_pointm)
	{
	    double x=shape_.shp().read_double();
	    double y=shape_.shp().read_double();
	    shape_.shp().read_double();//m
	    geometry_ptr point(new point_impl(-1));
	    point->move_to(x,y);
	    feature->set_geometry(point);
	    ++count_;
	}
	else if (type == shape_io::shape_pointz)
	{
	    double x=shape_.shp().read_double();
	    double y=shape_.shp().read_double();
	    shape_.shp().read_double();//z
	    shape_.shp().read_double();//m
	    geometry_ptr point(new point_impl(-1));
	    point->move_to(x,y);
	    feature->set_geometry(point);
	    ++count_;
	}
	else
	{
	    while (!filter_.pass(shape_.current_extent()))
	    {	
		unsigned reclen=shape_.reclength_;
		shape_.move_to(long(shape_.shp().pos()) + 2 * reclen - 36);
		if ((long)shape_.shp().pos() >= file_length_ * 2)
		    return feature_ptr(0);
	    }
	    
	    switch (type)
            {
	  
	    case shape_io::shape_polyline:
		{
		    geometry_ptr line = shape_.read_polyline();
		    feature->set_geometry(line);
		    ++count_;
		    break;
		}
	    case shape_io::shape_polylinem:
		{
		    geometry_ptr line = shape_.read_polylinem();
		    feature->set_geometry(line);
		    ++count_;
		    break;
		}
	    case shape_io::shape_polylinez:
		{
		    geometry_ptr line = shape_.read_polylinez();
		    feature->set_geometry(line);
		    ++count_;
		    break;
		}
	    case shape_io::shape_polygon:
		{		 
		    geometry_ptr poly = shape_.read_polygon();
		    feature->set_geometry(poly);
		    ++count_;
		    break;
		}
	    case shape_io::shape_polygonm:
		{		 
		    geometry_ptr poly = shape_.read_polygonm();
		    feature->set_geometry(poly);
		    ++count_;
		    break;
		}
	    case shape_io::shape_polygonz:
		{
		    geometry_ptr poly = shape_.read_polygonz();
		    feature->set_geometry(poly);
		    ++count_;
		    break;
		}
	    default:
		return feature_ptr(0);
            }
	    
	    if (attr_ids_.size())
            {
                shape_.dbf().move_to(shape_.id_);
		typename std::vector<int>::const_iterator pos=attr_ids_.begin();
		while (pos!=attr_ids_.end())
		{
		    try 
		    {
			shape_.dbf().add_attribute(*pos,feature.get());//TODO optimize!!!
		    }
		    catch (...)
		    {
			//TODO
		    }
		    ++pos;
                }
            }
	}
	return feature;
    }
    else
    {
	std::cout<<" total shapes read="<<count_<<"\n";
	return feature_ptr(0);
    }
}


template <typename filterT>
shape_featureset<filterT>::~shape_featureset() {}

template class shape_featureset<filter_in_box>;
