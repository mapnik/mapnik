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

#include "ogr_datasource.hpp"
#include "ogr_featureset.hpp"
#include <mapnik/ptree_helpers.hpp>

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


ogr_datasource::ogr_datasource(parameters const& params)
   : datasource(params),
     extent_(),
     type_(datasource::Vector),
     desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8"))
{
   OGRRegisterAll();

   boost::optional<std::string> file = params.get<std::string>("file");
   if (!file) throw datasource_exception("missing <file> paramater");

   boost::optional<std::string> layer = params.get<std::string>("layer");
   if (!layer) throw datasource_exception("missing <layer> paramater");

   layerName_ = *layer;
   multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries",false);

   dataset_ = OGRSFDriverRegistrar::Open ((*file).c_str(), FALSE);
   if (!dataset_) throw datasource_exception(CPLGetLastErrorMsg());

   layer_ = dataset_->GetLayerByName (layerName_.c_str());
   if (! layer_) throw datasource_exception("cannot find <layer> in dataset");
   
   OGREnvelope envelope;
   layer_->GetExtent (&envelope);
   extent_.init (envelope.MinX, envelope.MinY, envelope.MaxX, envelope.MaxY);

   OGRFeatureDefn* def = layer_->GetLayerDefn ();
   if (def != 0)
   {
       int fld_count = def->GetFieldCount ();
       for (int i = 0; i < fld_count; i++)
       {
           OGRFieldDefn* fld = def->GetFieldDefn (i);

           std_string fld_name = fld->GetNameRef ();
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

std::string const ogr_datasource::name_="ogr";

std::string ogr_datasource::name()
{
   return name_;
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
        mapnik::Envelope<double> const& query_extent = q.get_bbox();

        layer_->SetSpatialFilterRect (query_extent.minx(),
                                      query_extent.miny(),
                                      query_extent.maxx(),
                                      query_extent.maxy());

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

        return featureset_ptr(new ogr_featureset(*dataset_, *layer_, desc_.get_encoding(), multiple_geometries_));
   }
   return featureset_ptr();
}

featureset_ptr ogr_datasource::features_at_point(coord2d const& pt) const
{
   if (dataset_ && layer_)
   {
        OGRPoint point;
	    point.setX (pt.x);
	    point.setY (pt.y);

        layer_->SetSpatialFilter (&point);

        return featureset_ptr(new ogr_featureset(*dataset_, *layer_, desc_.get_encoding(), multiple_geometries_));
   }
   return featureset_ptr();
}

