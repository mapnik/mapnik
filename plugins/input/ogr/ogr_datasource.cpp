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

#include <iostream>
#include <fstream>
#include <stdexcept>

#include "ogr_datasource.hpp"
#include "ogr_featureset.hpp"
#include "ogr_index_featureset.hpp"

// mapnik
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/geom_util.hpp>

// boost
#include <boost/algorithm/string.hpp>

using std::clog;
using std::endl;

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(ogr_datasource)

using mapnik::Envelope;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;
using mapnik::datasource_exception;
using mapnik::filter_in_box;
using mapnik::filter_at_point;


ogr_datasource::ogr_datasource(parameters const& params)
   : datasource(params),
     extent_(),
     type_(datasource::Vector),
     desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8")),
     indexed_(false)
{
   OGRRegisterAll();

   boost::optional<std::string> file = params.get<std::string>("file");
   if (!file) throw datasource_exception("missing <file> parameter");

   multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries",false);

   boost::optional<std::string> base = params.get<std::string>("base");
   if (base)
      dataset_name_ = *base + "/" + *file;
   else
      dataset_name_ = *file;

   // open ogr driver   
   dataset_ = OGRSFDriverRegistrar::Open ((dataset_name_).c_str(), FALSE);
   if (!dataset_) 
   {
      std::string err = CPLGetLastErrorMsg();
      if( err.size() == 0 )
      {
         throw datasource_exception("Connection failed: " + dataset_name_ + " was not found or is not a supported format");
      } else {
         throw datasource_exception(err);
      }
   } 

   // initialize layer
   boost::optional<std::string> layer = params.get<std::string>("layer");
   if (!layer) 
   {
      std::string s ("missing <layer> parameter, available layers are: ");
      unsigned num_layers = dataset_->GetLayerCount();
      for (unsigned i = 0; i < num_layers; ++i )
      {
         OGRLayer  *ogr_layer = dataset_->GetLayer(i);
         OGRFeatureDefn* def = ogr_layer->GetLayerDefn();
         if (def != 0) { 
            s += " '";
            s += def->GetName();
            s += "' ";
         } else {
            s += "No layers found!";
         }
      }
      throw datasource_exception(s);
   }
   
   layerName_ = *layer;  
   layer_ = dataset_->GetLayerByName (layerName_.c_str());
   if (! layer_) throw datasource_exception("cannot find <layer> in dataset");
   
   // initialize envelope
   OGREnvelope envelope;
   layer_->GetExtent (&envelope);
   extent_.init (envelope.MinX, envelope.MinY, envelope.MaxX, envelope.MaxY);

   // scan for index file
   size_t breakpoint = dataset_name_.find_last_of (".");
   if (breakpoint == std::string::npos) breakpoint = dataset_name_.length();
   index_name_ = dataset_name_.substr(0, breakpoint) + ".index";
   std::ifstream index_file (index_name_.c_str(), std::ios::in | std::ios::binary);
   if (index_file)
   {
      indexed_=true;
      index_file.close();
   }

   // deal with attributes descriptions
   OGRFeatureDefn* def = layer_->GetLayerDefn ();
   if (def != 0)
   {
       int fld_count = def->GetFieldCount ();
       for (int i = 0; i < fld_count; i++)
       {
           OGRFieldDefn* fld = def->GetFieldDefn (i);

           std::string fld_name = fld->GetNameRef ();
           OGRFieldType type_oid = fld->GetType ();

           switch (type_oid)
           {
           case OFTInteger:
               desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Integer));
               break;

           case OFTReal:
               desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Double));
               break;
                   
           case OFTString:
           case OFTWideString: // deprecated
               desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::String));
               break;
              
           case OFTBinary:
               desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Object));
               break;

           case OFTIntegerList:
           case OFTRealList:
           case OFTStringList:
           case OFTWideStringList: // deprecated !
#ifdef MAPNIK_DEBUG
               clog << "unhandled type_oid=" << type_oid << endl;
#endif
               break;

           case OFTDate:
           case OFTTime:
           case OFTDateTime: // unhandled !
#ifdef MAPNIK_DEBUG
               clog << "unhandled type_oid=" << type_oid << endl;
#endif
               desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Object));
               break;

           default: // unknown
#ifdef MAPNIK_DEBUG
               clog << "unknown type_oid=" << type_oid << endl;
#endif
               break;
           }
       }
   }
}

ogr_datasource::~ogr_datasource()
{
    OGRDataSource::DestroyDataSource (dataset_);
}

std::string ogr_datasource::name()
{
   return "ogr";
}

int ogr_datasource::type() const
{
   return type_;
}

Envelope<double> ogr_datasource::envelope() const
{
   return extent_;
}

layer_descriptor ogr_datasource::get_descriptor() const
{
   return desc_;
}

featureset_ptr ogr_datasource::features(query const& q) const
{
   if (dataset_ && layer_)
   {
#if 0
        std::ostringstream s;
            
        s << "select ";
        std::set<std::string> const& props=q.property_names();
        std::set<std::string>::const_iterator pos=props.begin();
        std::set<std::string>::const_iterator end=props.end();
        while (pos != end)
        {
           s <<",\""<<*pos<<"\"";
           ++pos;
        }	 
        s << " from " << layerName_ ;

        // execute existing SQL
        OGRLayer* layer = dataset_->ExecuteSQL (s.str(), poly);

        // layer must be freed
        dataset_->ReleaseResultSet (layer);
#endif

        if (indexed_)
        {
            filter_in_box filter(q.get_bbox());
            
            return featureset_ptr(new ogr_index_featureset<filter_in_box> (*dataset_,
                                                                           *layer_,
                                                                           filter,
                                                                           index_name_,
                                                                           desc_.get_encoding(),
                                                                           multiple_geometries_));
        }
        else
        {
            return featureset_ptr(new ogr_featureset (*dataset_,
                                                      *layer_,
                                                      q.get_bbox(),
                                                      desc_.get_encoding(),
                                                      multiple_geometries_));
        }
   }
   return featureset_ptr();
}

featureset_ptr ogr_datasource::features_at_point(coord2d const& pt) const
{
   if (dataset_ && layer_)
   {
        if (indexed_)
        {
            filter_at_point filter(pt);
            
            return featureset_ptr(new ogr_index_featureset<filter_at_point> (*dataset_,
                                                                             *layer_,
                                                                             filter,
                                                                             index_name_,
                                                                             desc_.get_encoding(),
                                                                             multiple_geometries_));
        }
        else
        {
            OGRPoint point;
	        point.setX (pt.x);
	        point.setY (pt.y);

            return featureset_ptr(new ogr_featureset (*dataset_,
                                                      *layer_,
                                                      point,
                                                      desc_.get_encoding(),
                                                      multiple_geometries_));
        }
   }
   return featureset_ptr();
}

