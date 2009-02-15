
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

#include <iostream>
#include "osm_featureset.hpp"
#include <mapnik/geometry.hpp>

using mapnik::Feature;
using mapnik::feature_ptr;
using mapnik::geometry2d;
using mapnik::point_impl;
using mapnik::line_string_impl;
using mapnik::polygon_impl;

using std::cerr;
using std::endl;

template <typename filterT>
osm_featureset<filterT>::osm_featureset(const filterT& filter, 
                                           	osm_dataset * dataset, 
                                            const std::set<std::string>& 
													attribute_names,
                                            std::string const& encoding)
   : filter_(filter),
     query_ext_(),
     tr_(new transcoder(encoding)),
     count_(0),
	dataset_ (dataset),
	attribute_names_ (attribute_names)
{
	dataset_->rewind();
}


template <typename filterT>
feature_ptr osm_featureset<filterT>::next()
{
	osm_item * cur_item = dataset_->next_item();
	feature_ptr feature;
	bool success=false;
	if(cur_item != NULL)
	{
		if(dataset_->current_item_is_node())
		{
			feature= feature_ptr(new Feature(count_++));
			double lat = static_cast<osm_node*>(cur_item)->lat;
			double lon = static_cast<osm_node*>(cur_item)->lon;
			geometry2d * point = new point_impl;
			point->move_to(lon,lat);
			feature->add_geometry(point);
			success=true;
		} 
		else if (dataset_->current_item_is_way())
		{
			bounds b = static_cast<osm_way*>(cur_item)->get_bounds();
		
			// Loop until we find a feature which passes the filter
			while(cur_item != NULL &&
					!filter_.pass(Envelope<double>(b.w,b.s,b.e,b.n)))
			{
				cur_item = dataset_->next_item();
				if(cur_item!=NULL)
					b = static_cast<osm_way*>(cur_item)->get_bounds();
			}	
				
			if(cur_item != NULL)
			{
				if(static_cast<osm_way*>(cur_item)->nodes.size())
				{
					feature=feature_ptr(new Feature(count_++));
					geometry2d *geom;
					if(static_cast<osm_way*>(cur_item)->is_polygon())
						geom=new polygon_impl;
					else
						geom=new line_string_impl;

					geom->set_capacity(static_cast<osm_way*>(cur_item)->
									nodes.size());
					geom->move_to(static_cast<osm_way*>(cur_item)->
										nodes[0]->lon,
								static_cast<osm_way*>(cur_item)->
										nodes[0]->lat);
				
					for(unsigned int count=1; count<static_cast<osm_way*>(cur_item)
								->nodes.size(); count++)
					{
						geom->line_to(static_cast<osm_way*>(cur_item)
									->nodes[count]->lon,
								static_cast<osm_way*>(cur_item)
									->nodes[count]->lat);
					}
					feature->add_geometry(geom);
					success=true;
				}
			}
		}
	
		// can feature_ptr be compared to NULL? - no 
		if(success)
		{
			std::map<std::string,std::string>::iterator i=
				cur_item->keyvals.begin();

			// add the keyvals to the feature. the feature seems to be a map
			// of some sort so this should work - see dbf_file::add_attribute()
			while(i != cur_item->keyvals.end())
			{	
				//only add if in the specified set of attribute names
				if(attribute_names_.find(i->first) != attribute_names_.end())
					(*feature)[i->first] = tr_->transcode(i->second.c_str());
				i++;
			}
			return feature;
		}
	}   
	return feature_ptr();
}


template <typename filterT>
osm_featureset<filterT>::~osm_featureset() {}

template class osm_featureset<mapnik::filter_in_box>;
template class osm_featureset<mapnik::filter_at_point>;

         
