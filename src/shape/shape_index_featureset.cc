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

//$Id$

#include "shape_index_featureset.hh"

template <typename filterT>
ShapeIndexFeatureset<filterT>::ShapeIndexFeatureset(const filterT& filter,
						    const std::string& shape_file,
						    int srid)
    : srid_(srid),
      filter_(filter),
      shape_type_(0),
      shape_(shape_file),
      count_(0)

{
    shape_.shp().skip(100);
    std::string indexname(shape_file + ".index");
    std::ifstream file(indexname.c_str(),std::ios::in|std::ios::binary);
    if (file)
    {
        shp_index<filterT>::query(filter,file,ids_);
        file.close();
    }
    std::cout<< "query size=" << ids_.size() << "\n";
    itr_ = ids_.begin();
}

template <typename filterT>
Feature* ShapeIndexFeatureset<filterT>::next()
{
    VectorFeature *f=0;

    if (itr_!=ids_.end())
    {
        int pos=*itr_++;
	shape_.move_to(pos);
        int type=shape_.type();
        if (type==shape_io::shape_point)
        {
            double x=shape_.shp().read_double();
	    double y=shape_.shp().read_double();	    
	    geometry_ptr point(new point_impl(-1));
	    point->move_to(x,y);
            f=new VectorFeature(shape_.id_,point);
	    ++count_;
        }
	else if (type == shape_io::shape_pointz)
	{
	    double x=shape_.shp().read_double();
	    double y=shape_.shp().read_double();
	    double z=shape_.shp().read_double();
	    geometry_ptr point(new point_impl(-1));
	    point->move_to(x,y);
            f=new VectorFeature(shape_.id_,point);
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
	        case shape_io::shape_polyline:
		{
		    geometry_ptr line = shape_.read_polyline();
		    f=new VectorFeature(shape_.id_,line);
		    ++count_;
		    break;
		}
	        case shape_io::shape_polygon:
		{
		 
		    geometry_ptr poly = shape_.read_polygon();
		    f=new VectorFeature(shape_.id_,poly);
		    ++count_;
		    break;
		}
            }
            if (0)
            {
                //shape_.dbf().move_to(id);
                //for (int j=0;j<shape_.dbf().num_fields();++j) {
                //  shape_.dbf().add_attribute(j,f);//TODO optimize!!!
                //}
            }
        }
    }
    if (!f) std::cout<<count_<<" features\n";
    return f;
}


template <typename filterT>
ShapeIndexFeatureset<filterT>::~ShapeIndexFeatureset() {}

template class ShapeIndexFeatureset<filter_at_point>;
template class ShapeIndexFeatureset<filter_in_box>;

