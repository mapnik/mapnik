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

#include "shape_featureset.hh"
#include <iostream>

template <typename filterT>
shape_featureset<filterT>::shape_featureset(const filterT& filter, 
					    const std::string& shape_file, 
					    long file_length )
    : filter_(filter),
      shape_type_(shape_io::shape_null),
      shape_(shape_file),
      query_ext_(),
      file_length_(file_length),
      count_(0)
{
    shape_.shp().skip(100);
}


template <typename filterT>
Feature* shape_featureset<filterT>::next()
{
    Feature* feature=0;
    std::streampos pos=shape_.shp().pos();
    
    if (pos < std::streampos(file_length_ * 2))
    {
        shape_.move_to(pos);
	int type=shape_.type();
	int id=shape_.id_;
	if (type == shape_io::shape_point)
	{
	    double x=shape_.shp().read_double();
	    double y=shape_.shp().read_double();
	    geometry_ptr point(new point_impl(-1));
	    point->move_to(x,y);
            feature=new Feature(id,point);
	    ++count_;
	}
	else if (type == shape_io::shape_pointz)
	{
	    double x=shape_.shp().read_double();
	    double y=shape_.shp().read_double();
	    double z=shape_.shp().read_double();
	    geometry_ptr point(new point_impl(-1));
	    point->move_to(x,y);
            feature=new Feature(id,point);
	    ++count_;
	}
	else
	{
	    while (!filter_.pass(shape_.current_extent()))
	    {	
		unsigned reclen=shape_.reclength_;
		shape_.move_to(long(shape_.shp().pos()) + 2 * reclen - 36);
		if ((unsigned long)shape_.shp().pos() >= file_length_ * 2)
		    return 0;
	    }
	    
	    switch (type)
            {
	    case shape_io::shape_polyline:
                {
                    geometry_ptr line = shape_.read_polyline();
                    feature=new Feature(id,line);
		    ++count_;
                    break;
                }
	    case shape_io::shape_polygon:
                {
                    geometry_ptr poly = shape_.read_polygon();
                    feature=new Feature(id,poly);
		    ++count_;
                    break;
                }
	    default:
		return 0;
            }
	    
            if (0)
            {
                shape_.dbf().move_to(id);
                for (int j=0;j<shape_.dbf().num_fields();++j) {
                  shape_.dbf().add_attribute(j,feature); //TODO optimize!!!
                }
            }
	}
    }
    if (!feature)
	std::cout<<" total shapes read="<<count_<<"\n";
    return feature;
}


template <typename filterT>
shape_featureset<filterT>::~shape_featureset() {}

template class shape_featureset<filter_in_box>;
