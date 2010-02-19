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

//$Id: postgis.hpp 44 2005-04-22 18:53:54Z pavlenko $

#ifndef POSTGIS_HPP
#define POSTGIS_HPP


#include <mapnik/datasource.hpp>
#include <mapnik/envelope.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <set>

#include "connection_manager.hpp"
#include "resultset.hpp"
#include "cursorresultset.hpp"

using mapnik::transcoder;
using mapnik::datasource;
using mapnik::Envelope;
using mapnik::layer_descriptor;
using mapnik::featureset_ptr;
using mapnik::feature_ptr;
using mapnik::query;
using mapnik::parameters;
using mapnik::coord2d;

class transcoder;
class postgis_datasource : public datasource
{
      static const std::string GEOMETRY_COLUMNS;
      static const std::string SPATIAL_REF_SYS;
      const std::string uri_;
      const std::string username_;
      const std::string password_;
      const std::string table_;
      mutable std::string schema_;
      mutable std::string geometry_table_;
      const std::string geometry_field_;
      const int cursor_fetch_size_;
      const int row_limit_;
      mutable std::string geometryColumn_;
      int type_;
      int srid_;
      mutable bool extent_initialized_;
      mutable mapnik::Envelope<double> extent_;
      layer_descriptor desc_;
      ConnectionCreator<Connection> creator_;
      bool multiple_geometries_;
      static const std::string name_;
      const std::string bbox_token_;
      const std::string scale_denom_token_;
      bool persist_connection_;
      bool extent_from_subquery_;
      //bool show_queries_;
   public:
      static std::string name();
      int type() const;
      featureset_ptr features(const query& q) const;
      featureset_ptr features_at_point(coord2d const& pt) const;
      mapnik::Envelope<double> envelope() const;
      layer_descriptor get_descriptor() const;
      postgis_datasource(const parameters &params);
      ~postgis_datasource();
   private:
      std::string sql_bbox(Envelope<double> const& env) const;
      std::string populate_tokens(const std::string& sql, double const& scale_denom, Envelope<double> const& env) const;
      std::string populate_tokens(const std::string& sql) const;
      static std::string unquote(const std::string& sql);
      static std::string table_from_sql(const std::string& sql);
      boost::shared_ptr<IResultSet> get_resultset(boost::shared_ptr<Connection> const &conn, const std::string &sql) const;
      postgis_datasource(const postgis_datasource&);
      postgis_datasource& operator=(const postgis_datasource&);
};

class postgis_featureset : public mapnik::Featureset
{
   private:
      boost::shared_ptr<IResultSet> rs_;
      bool multiple_geometries_;
      unsigned num_attrs_;
      boost::scoped_ptr<mapnik::transcoder> tr_;
      mutable int totalGeomSize_;
      mutable int count_;
   public:
      postgis_featureset(boost::shared_ptr<IResultSet> const& rs,
                         std::string const& encoding,
                         bool multiple_geometries,
                         unsigned num_attrs);
      feature_ptr next();
      ~postgis_featureset();
   private:
      postgis_featureset(const postgis_featureset&);
      const postgis_featureset& operator=(const postgis_featureset&);
};

#endif                                            //POSTGIS_HPP
