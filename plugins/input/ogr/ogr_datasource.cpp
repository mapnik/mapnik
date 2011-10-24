/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(ogr_datasource)

using mapnik::box2d;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;
using mapnik::datasource_exception;
using mapnik::filter_in_box;
using mapnik::filter_at_point;


ogr_datasource::ogr_datasource(parameters const& params, bool bind)
  : datasource(params),
    extent_(),
    type_(datasource::Vector),
    desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding", "utf-8")),
    indexed_(false)
{
    boost::optional<std::string> file = params.get<std::string>("file");
    boost::optional<std::string> string = params.get<std::string>("string");
    if (! file && ! string)
    {
        throw datasource_exception("missing <file> or <string> parameter");
    }

    multiple_geometries_ = *params.get<mapnik::boolean>("multiple_geometries", false);

    if (string)
    {
       dataset_name_ = *string;
    }
    else
    {
        boost::optional<std::string> base = params.get<std::string>("base");
        if (base)
        {
            dataset_name_ = *base + "/" + *file;
        }
        else
        {
            dataset_name_ = *file;
        }
    }

    if (bind)
    {
        this->bind();
    }
}

ogr_datasource::~ogr_datasource()
{
    if (is_bound_) 
    {
        // free layer before destroying the datasource
        layer_.free_layer();

        OGRDataSource::DestroyDataSource (dataset_);
    }
}

void ogr_datasource::bind() const
{
    if (is_bound_) return;

    // initialize ogr formats
    OGRRegisterAll();

    // open ogr driver
    dataset_ = OGRSFDriverRegistrar::Open((dataset_name_).c_str(), FALSE);
    if (! dataset_)
    {
        const std::string err = CPLGetLastErrorMsg();
        if (err.size() == 0)
        {
            throw datasource_exception("OGR Plugin: connection failed: " + dataset_name_ + " was not found or is not a supported format");
        }
        else
        {
            throw datasource_exception("OGR Plugin: " + err);
        }
    }

    // initialize layer
    boost::optional<std::string> layer_by_name = params_.get<std::string>("layer");
    boost::optional<unsigned> layer_by_index = params_.get<unsigned>("layer_by_index");
    boost::optional<std::string> layer_by_sql = params_.get<std::string>("layer_by_sql");

    int passed_parameters = 0;
    passed_parameters += layer_by_name ? 1 : 0;
    passed_parameters += layer_by_index ? 1 : 0;
    passed_parameters += layer_by_sql ? 1 : 0;

    if (passed_parameters > 1)
    {
        throw datasource_exception("OGR Plugin: you can only select an ogr layer by name "
                                   "('layer' parameter), by number ('layer_by_index' parameter), "
                                   "or by sql ('layer_by_sql' parameter), "
                                   "do not supply 2 or more of them at the same time" );
    }

    if (layer_by_name)
    {
        layer_name_ = *layer_by_name;
        layer_.layer_by_name(dataset_, layer_name_);
    }
    else if (layer_by_index)
    {
        const unsigned num_layers = dataset_->GetLayerCount();
        if (*layer_by_index >= num_layers)
        {
            std::ostringstream s;
            s << "OGR Plugin: only ";
            s << num_layers;
            s << " layer(s) exist, cannot find layer by index '" << *layer_by_index << "'";

            throw datasource_exception(s.str());
        }

        layer_.layer_by_index(dataset_, *layer_by_index);
        layer_name_ = layer_.layer_name();
    }
    else if (layer_by_sql)
    {
        layer_.layer_by_sql(dataset_, *layer_by_sql);
        layer_name_ = layer_.layer_name();
    }
    else
    {
        std::ostringstream s;
        s << "OGR Plugin: missing <layer> or <layer_by_index> or <layer_by_sql> "
          << "parameter, available layers are: ";

        unsigned num_layers = dataset_->GetLayerCount();
        bool layer_found = false;
        for (unsigned i = 0; i < num_layers; ++i )
        {
            OGRLayer* ogr_layer = dataset_->GetLayer(i);
            OGRFeatureDefn* ogr_layer_def = ogr_layer->GetLayerDefn();
            if (ogr_layer_def != 0)
            {
                layer_found = true;
                s << " '" << ogr_layer_def->GetName() << "' ";
            }
        }

        if (! layer_found)
        {
            s << "None (no layers were found in dataset)";
        }

        throw datasource_exception(s.str());
    }

    if (! layer_.is_valid())
    {
        std::string s("OGR Plugin: ");

        if (layer_by_name)
        {
            s += "cannot find layer by name '" + *layer_by_name;
        }
        else if (layer_by_index)
        {
            s += "cannot find layer by index number '" + *layer_by_index;
        }
        else if (layer_by_sql)
        {
            s += "cannot find layer by sql query '" + *layer_by_sql;
        }

        s += "' in dataset '" + dataset_name_ + "'";

        throw datasource_exception(s);
    }

    // work with real OGR layer
    OGRLayer* layer = layer_.layer();

    // initialize envelope
    OGREnvelope envelope;
    layer->GetExtent(&envelope);
    extent_.init(envelope.MinX, envelope.MinY, envelope.MaxX, envelope.MaxY);

    // scan for index file
    // TODO - layer names don't match dataset name, so this will break for
    // any layer types of ogr than shapefiles, etc
    // fix here and in ogrindex
    size_t breakpoint = dataset_name_.find_last_of(".");
    if (breakpoint == std::string::npos)
    {
        breakpoint = dataset_name_.length();
    }
    index_name_ = dataset_name_.substr(0, breakpoint) + ".ogrindex";

    std::ifstream index_file(index_name_.c_str(), std::ios::in | std::ios::binary);
    if (index_file)
    {
        indexed_ = true;
        index_file.close();
    }
#if 0
    // TODO - enable this warning once the ogrindex tool is a bit more stable/mature
    else
    {
      std::clog << "### Notice: no ogrindex file found for " << dataset_name_
                << ", use the 'ogrindex' program to build an index for faster rendering"
                << std::endl;
    }
#endif

    // deal with attributes descriptions
    OGRFeatureDefn* def = layer->GetLayerDefn();
    if (def != 0)
    {
        const int fld_count = def->GetFieldCount();
        for (int i = 0; i < fld_count; i++)
        {
            OGRFieldDefn* fld = def->GetFieldDefn(i);

            const std::string fld_name = fld->GetNameRef();
            const OGRFieldType type_oid = fld->GetType();

            switch (type_oid)
            {
            case OFTInteger:
                desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Integer));
                break;

            case OFTReal:
                desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Double));
                break;

            case OFTString:
            case OFTWideString: // deprecated
                desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::String));
                break;

            case OFTBinary:
                desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Object));
                break;

            case OFTIntegerList:
            case OFTRealList:
            case OFTStringList:
            case OFTWideStringList: // deprecated !
#ifdef MAPNIK_DEBUG
                std::clog << "OGR Plugin: unhandled type_oid=" << type_oid << std::endl;
#endif
                break;

            case OFTDate:
            case OFTTime:
            case OFTDateTime: // unhandled !
#ifdef MAPNIK_DEBUG
                std::clog << "OGR Plugin: unhandled type_oid=" << type_oid << std::endl;
#endif
                desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Object));
                break;

            default: // unknown
#ifdef MAPNIK_DEBUG
                std::clog << "OGR Plugin: unknown type_oid=" << type_oid << std::endl;
#endif
                break;
            }
        }
    }

    is_bound_ = true;
}

std::string ogr_datasource::name()
{
    return "ogr";
}

int ogr_datasource::type() const
{
    return type_;
}

box2d<double> ogr_datasource::envelope() const
{
    if (! is_bound_) bind();
    return extent_;
}

layer_descriptor ogr_datasource::get_descriptor() const
{
    if (! is_bound_) bind();
    return desc_;
}

featureset_ptr ogr_datasource::features(query const& q) const
{
    if (! is_bound_) bind();
   
    if (dataset_ && layer_.is_valid())
    {
        OGRLayer* layer = layer_.layer();

        if (indexed_)
        {
            filter_in_box filter(q.get_bbox());
            
            return featureset_ptr(new ogr_index_featureset<filter_in_box>(*dataset_,
                                                                          *layer,
                                                                          filter,
                                                                          index_name_,
                                                                          desc_.get_encoding(),
                                                                          multiple_geometries_));
        }
        else
        {
            return featureset_ptr(new ogr_featureset (*dataset_,
                                                      *layer,
                                                      q.get_bbox(),
                                                      desc_.get_encoding(),
                                                      multiple_geometries_));
        }
    }

    return featureset_ptr();
}

featureset_ptr ogr_datasource::features_at_point(coord2d const& pt) const
{
    if (!is_bound_) bind();
   
    if (dataset_ && layer_.is_valid())
    {
        OGRLayer* layer = layer_.layer();

        if (indexed_)
        {
            filter_at_point filter(pt);
            
            return featureset_ptr(new ogr_index_featureset<filter_at_point> (*dataset_,
                                                                             *layer,
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
                                                      *layer,
                                                      point,
                                                      desc_.get_encoding(),
                                                      multiple_geometries_));
        }
    }

    return featureset_ptr();
}
