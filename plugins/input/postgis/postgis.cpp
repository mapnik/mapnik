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

//$Id: postgis.cc 44 2005-04-22 18:53:54Z pavlenko $

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/ptree_helpers.hpp>

/*
#ifdef MAPNIK_DEBUG
#include <mapnik/wall_clock_timer.hpp>
#endif
*/

#include "connection_manager.hpp"
#include "postgis.hpp"

// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>

// stl
#include <string>
#include <algorithm>
#include <set>
#include <sstream>
#include <iomanip>

#define FMAX std::numeric_limits<double>::max()

DATASOURCE_PLUGIN(postgis_datasource)

const std::string postgis_datasource::GEOMETRY_COLUMNS="geometry_columns";
const std::string postgis_datasource::SPATIAL_REF_SYS="spatial_ref_system";

using std::clog;
using std::endl;

using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::shared_ptr;

using mapnik::PoolGuard;
using mapnik::attribute_descriptor;

postgis_datasource::postgis_datasource(parameters const& params)
   : datasource(params),
     table_(*params_.get<std::string>("table","")),
     schema_(""),
     geometry_table_(*params_.get<std::string>("geometry_table","")),
     geometry_field_(*params_.get<std::string>("geometry_field","")),
     cursor_fetch_size_(*params_.get<int>("cursor_size",0)),
     row_limit_(*params_.get<int>("row_limit",0)),
     type_(datasource::Vector),
     srid_(*params_.get<int>("srid",0)),
     extent_initialized_(false),
     desc_(*params_.get<std::string>("type"),"utf-8"),
     creator_(params.get<std::string>("host"),
              params.get<std::string>("port"),
              params.get<std::string>("dbname"),
              params.get<std::string>("user"),
              params.get<std::string>("password")),
     bbox_token_("!bbox!"),
     scale_denom_token_("!scale_denominator!"),
     persist_connection_(*params_.get<mapnik::boolean>("persist_connection",true)),
     extent_from_subquery_(*params_.get<mapnik::boolean>("extent_from_subquery",false))
     //show_queries_(*params_.get<mapnik::boolean>("show_queries",false))
     
{   

   if (table_.empty()) throw mapnik::datasource_exception("PostGIS: missing <table> parameter");

   boost::optional<int> initial_size = params_.get<int>("inital_size",1);
   boost::optional<int> max_size = params_.get<int>("max_size",10);

   multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries",false);
   
   boost::optional<std::string> ext  = params_.get<std::string>("extent");
   if (ext)
   {
      boost::char_separator<char> sep(",");
      boost::tokenizer<boost::char_separator<char> > tok(*ext,sep);
      unsigned i = 0;
      bool success = false;
      double d[4];
      for (boost::tokenizer<boost::char_separator<char> >::iterator beg=tok.begin(); 
           beg!=tok.end();++beg)
      {
         try 
         {
            d[i] = boost::lexical_cast<double>(boost::trim_copy(*beg));
         }
         catch (boost::bad_lexical_cast & ex)
         {
            clog << *beg << " : " << ex.what() << "\nAre your coordinates each separated by commas?\n";
            break;
         }
         if (i==3) 
         {
            success = true;
            break;
         }
         ++i;
      }
      if (success)
      {
         extent_.init(d[0],d[1],d[2],d[3]);
         extent_initialized_ = true;
      }
   } 

   ConnectionManager *mgr=ConnectionManager::instance();   
   mgr->registerPool(creator_, *initial_size, *max_size);
    
   shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
   if (pool)
   {      
      shared_ptr<Connection> conn = pool->borrowObject();
      if (conn && conn->isOK())
      {
         PoolGuard<shared_ptr<Connection>,
            shared_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);
         
         desc_.set_encoding(conn->client_encoding());

         if(geometry_table_.empty())
         {
             geometry_table_ = table_from_sql(table_);
         }
         std::string::size_type idx = geometry_table_.find_last_of('.');
         if (idx!=std::string::npos)
         {
            schema_ = geometry_table_.substr(0,idx);
            geometry_table_ = geometry_table_.substr(idx+1);
         }
         else
         {
            geometry_table_ = geometry_table_.substr(0);
         }

         // If we do not know both the geometry_field and the srid
         // then first attempt to fetch the geometry name from a geometry_columns entry.
         // This will return no records if we are querying a bogus table returned
         // from the simplistic table parsing in table_from_sql() or if
         // the table parameter references a table, view, or subselect not
         // registered in the geometry columns.
         geometryColumn_ = geometry_field_;
         if (!geometryColumn_.length() > 0 || srid_ == 0)
         {
             std::ostringstream s;
             s << "SELECT f_geometry_column, srid FROM ";
             s << GEOMETRY_COLUMNS <<" WHERE f_table_name='" << unquote(geometry_table_) <<"'";
             
             if (schema_.length() > 0) 
                s << " AND f_table_schema='" << unquote(schema_) << "'";
            
             if (geometry_field_.length() > 0)
                s << " AND f_geometry_column='" << unquote(geometry_field_) << "'";

             /*if (show_queries_)
             {
                 clog << boost::format("PostGIS: sending query: %s\n") % s.str();
             }*/
                 
             shared_ptr<ResultSet> rs=conn->executeQuery(s.str());
             if (rs->next())
             {
                geometryColumn_ = rs->getValue("f_geometry_column");

                if (srid_ == 0)
                {
                    try 
                    {
                        srid_ = lexical_cast<int>(rs->getValue("srid"));
                    }
                    catch (bad_lexical_cast &ex)
                    {
                       clog << "SRID: " << rs->getValue("srid") << ":" << ex.what() << endl;
                    }
                }
             }
             rs->close();
             
             // If we still do not know the srid then we can try to fetch
             // it from the 'table_' parameter, which should work even if it is
             // a subselect as long as we know the geometry_field to query
             if (geometryColumn_.length() && srid_ <= 0)
             {
                s.str("");
                s << "SELECT SRID(\"" << geometryColumn_ << "\") AS srid FROM ";
                s << populate_tokens(table_) << " WHERE \"" << geometryColumn_ << "\" IS NOT NULL LIMIT 1;";

                /*if (show_queries_)
                {
                   clog << boost::format("PostGIS: sending query: %s\n") % s.str();
                }*/

                shared_ptr<ResultSet> rs=conn->executeQuery(s.str());
                if (rs->next())
                {
                    try
                    {
                        srid_ = lexical_cast<int>(rs->getValue("srid"));
                    }
                    catch (bad_lexical_cast &ex)
                    {
                        clog << "SRID: " << rs->getValue("srid") << ":" << ex.what() << endl;
                    }
                }
                rs->close();
            }
         }
         
         if (srid_ == 0)
         {
            srid_ = -1;
            clog << "PostGIS: SRID warning, using srid=-1" << endl;
         }

         // At this point the geometry_field may still not be known
         // but we'll catch that where more useful...         
#ifdef MAPNIK_DEBUG
         clog << "PostGIS: using SRID=" << srid_ << endl;
         clog << "PostGIS: using geometry_column=" << geometryColumn_ << endl;
#endif

         // collect attribute desc         
         std::ostringstream s;
         s << "select * from " << populate_tokens(table_) << " limit 0";

         /*if (show_queries_)
         {
             clog << boost::format("PostGIS: sending query: %s\n") % s.str();
         }*/

         shared_ptr<ResultSet> rs=conn->executeQuery(s.str());
         int count = rs->getNumFields();
         for (int i=0;i<count;++i)
         {
            std::string fld_name=rs->getFieldName(i);
            int type_oid = rs->getTypeOID(i);
            switch (type_oid)
            {
               case 20:    // int8
               case 21:    // int2
               case 23:    // int4
                  desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Integer));
                  break;
               case 700:   // float4 
               case 701:   // float8
               case 1700:  // numeric ??
                  desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Double));
               case 1042:  // bpchar
               case 1043:  // varchar
               case 25:    // text
                  desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::String));
                  break;
               default: // should not get here
#ifdef MAPNIK_DEBUG
                  s.str("");
                  s << "select oid, typname from pg_type where oid = " << type_oid;

                  /*if (show_queries_)
                  {
                     clog << boost::format("PostGIS: sending query: %s\n") % s.str();
                  }*/

                  shared_ptr<ResultSet> rs_oid = conn->executeQuery(s.str());
                  if (rs_oid->next())
                  {
                      clog << "PostGIS: unknown type = " << rs_oid->getValue("typname") << " (oid:" << rs_oid->getValue("oid") << ")\n";
                  }
                  else
                  {
                      clog << "PostGIS: unknown oid type =" << type_oid << endl;              
                  }
                  rs_oid->close();
#endif
                  break;
            }
         }
         rs->close();
      }
   }
}

std::string const postgis_datasource::name_="postgis";

std::string postgis_datasource::name()
{
   return name_;
}

int postgis_datasource::type() const
{
   return type_;
}

layer_descriptor postgis_datasource::get_descriptor() const
{
   return desc_;
}


std::string postgis_datasource::sql_bbox(Envelope<double> const& env) const
{
    std::ostringstream b;
    if (srid_ > 0)
        b << "SetSRID(";
    b << "'BOX3D(";
    b << std::setprecision(16);
    b << env.minx() << " " << env.miny() << ",";
    b << env.maxx() << " " << env.maxy() << ")'::box3d";
    if (srid_ > 0)
        b << ", " << srid_ << ")";
    return b.str();
}

std::string postgis_datasource::populate_tokens(const std::string& sql) const
{
    std::string populated_sql = sql;
    
    if ( boost::algorithm::icontains(sql,bbox_token_) )
    {
        Envelope<double> max_env(-1 * FMAX,-1 * FMAX,FMAX,FMAX);
        std::string max_box = sql_bbox(max_env);
        boost::algorithm::replace_all(populated_sql,bbox_token_,max_box);
    } 
    if ( boost::algorithm::icontains(sql,scale_denom_token_) )
    {
        std::string max_denom = lexical_cast<std::string>(FMAX);
        boost::algorithm::replace_all(populated_sql,scale_denom_token_,max_denom);
    }
    return populated_sql;
}

std::string postgis_datasource::populate_tokens(const std::string& sql, double const& scale_denom, Envelope<double> const& env) const
{
    std::string populated_sql = sql;
    std::string box = sql_bbox(env);
    
    if ( boost::algorithm::icontains(populated_sql,scale_denom_token_) )
    {
        std::string max_denom = lexical_cast<std::string>(scale_denom);
        boost::algorithm::replace_all(populated_sql,scale_denom_token_,max_denom);
    }
    
    if ( boost::algorithm::icontains(populated_sql,bbox_token_) )
    {
        boost::algorithm::replace_all(populated_sql,bbox_token_,box);
        return populated_sql;
    }
    else
    {
        std::ostringstream s;
        s << " WHERE \"" << geometryColumn_ << "\" && " << box;
        return populated_sql + s.str();    
    }
}


std::string postgis_datasource::unquote(const std::string& sql)
{
    std::string table_name = boost::algorithm::to_lower_copy(sql);  
    boost::algorithm::trim_if(table_name,boost::algorithm::is_any_of("\""));
    return table_name;
}

// TODO - make smarter and potentially move to reusable utilities 
// available to other SQL-based plugins
std::string postgis_datasource::table_from_sql(const std::string& sql)
{
   std::string table_name = boost::algorithm::to_lower_copy(sql);
   boost::algorithm::replace_all(table_name,"\n"," ");
   
   std::string::size_type idx = table_name.rfind(" from ");
   if (idx!=std::string::npos)
   {
      
      idx=table_name.find_first_not_of(" ",idx+5);
      if (idx != std::string::npos)
      {
         table_name=table_name.substr(idx);
      }
      idx=table_name.find_first_of(" )");
      if (idx != std::string::npos)
      {
         table_name = table_name.substr(0,idx);
      }
   }
   return table_name;
}

boost::shared_ptr<IResultSet> postgis_datasource::get_resultset(boost::shared_ptr<Connection> const &conn, const std::string &sql) const
{
     if (cursor_fetch_size_ > 0) {
         // cursor
         std::ostringstream csql;
         std::string cursor_name = conn->new_cursor_name();

         csql << "DECLARE " << cursor_name << " BINARY INSENSITIVE NO SCROLL CURSOR WITH HOLD FOR " << sql << " FOR READ ONLY";

         /*if (show_queries_)
         {
             clog << boost::format("PostGIS: sending query: %s\n") % csql.str();
         }*/

         if (!conn->execute(csql.str())) {
            throw mapnik::datasource_exception( "PSQL Error: Creating cursor for data select." );
         }
         return shared_ptr<CursorResultSet>(new CursorResultSet(conn, cursor_name, cursor_fetch_size_));

     } else {
         // no cursor

         /*if (show_queries_)
         {
             clog << boost::format("PostGIS: sending query: %s\n") % sql;
         }*/

         return conn->executeQuery(sql,1);
     }
}

featureset_ptr postgis_datasource::features(const query& q) const
{

/*
#ifdef MAPNIK_DEBUG
    mapnik::wall_clock_progress_timer timer(clog, "end feature query: ");
#endif
*/

   Envelope<double> const& box = q.get_bbox();
   double scale_denom = q.scale_denominator();
   ConnectionManager *mgr=ConnectionManager::instance();
   shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
   if (pool)
   {
      shared_ptr<Connection> conn = pool->borrowObject();
      if (conn && conn->isOK())
      {       
         PoolGuard<shared_ptr<Connection>,shared_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);

         if (!geometryColumn_.length() > 0)
         {
             std::ostringstream s_error;
             s_error << "PostGIS: geometry name lookup failed for table '" << schema_ << "." << geometry_table_
             << "'. Please manually provide the 'geometry_field' parameter or add an entry "
             << "in the geometry_columns for '" << schema_ << "." << geometry_table_ << "'.";
             throw mapnik::datasource_exception(s_error.str());
         }

         std::ostringstream s;
         s << "SELECT AsBinary(\"" << geometryColumn_ << "\") AS geom";
         std::set<std::string> const& props=q.property_names();
         std::set<std::string>::const_iterator pos=props.begin();
         std::set<std::string>::const_iterator end=props.end();
         while (pos != end)
         {
            s <<",\""<<*pos<<"\"";
            ++pos;
         }	 

         std::string table_with_bbox = populate_tokens(table_,scale_denom,box);

         s << " from " << table_with_bbox;

         if (row_limit_ > 0) {
             s << " LIMIT " << row_limit_;
         }
         
         boost::shared_ptr<IResultSet> rs = get_resultset(conn, s.str());
         return featureset_ptr(new postgis_featureset(rs,desc_.get_encoding(),multiple_geometries_,props.size()));
      }
   }
   return featureset_ptr();
}

featureset_ptr postgis_datasource::features_at_point(coord2d const& pt) const
{
   ConnectionManager *mgr=ConnectionManager::instance();
   shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
   if (pool)
   {
      shared_ptr<Connection> conn = pool->borrowObject();
      if (conn && conn->isOK())
      {       
         PoolGuard<shared_ptr<Connection>,shared_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);
         std::ostringstream s;

         if (!geometryColumn_.length() > 0)
         {
             std::ostringstream s_error;
             s_error << "PostGIS: geometry name lookup failed for table '" << schema_ << "." << geometry_table_
             << "'. Please manually provide the 'geometry_field' parameter or add an entry "
             << "in the geometry_columns for '" << schema_ << "." << geometry_table_ << "'.";
             throw mapnik::datasource_exception(s_error.str());
         }
                    
         s << "SELECT AsBinary(\"" << geometryColumn_ << "\") AS geom";
            
         std::vector<attribute_descriptor>::const_iterator itr = desc_.get_descriptors().begin();
         std::vector<attribute_descriptor>::const_iterator end = desc_.get_descriptors().end();
         unsigned size=0;
         while (itr != end)
         {
            s << ",\"" << itr->get_name() << "\"";
            ++itr;
            ++size;
         }

         Envelope<double> box(pt.x,pt.y,pt.x,pt.y);
         std::string table_with_bbox = populate_tokens(table_,FMAX,box);

         s << " from " << table_with_bbox;
         
         if (row_limit_ > 0) {
             s << " LIMIT " << row_limit_;
         }
         
         boost::shared_ptr<IResultSet> rs = get_resultset(conn, s.str());
         return featureset_ptr(new postgis_featureset(rs,desc_.get_encoding(),multiple_geometries_, size));
      }
   }
   return featureset_ptr();
}

Envelope<double> postgis_datasource::envelope() const
{
   if (extent_initialized_) return extent_;
    
   ConnectionManager *mgr=ConnectionManager::instance();
   shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
   if (pool)
   {
      shared_ptr<Connection> conn = pool->borrowObject();
      if (conn && conn->isOK())
      {
         PoolGuard<shared_ptr<Connection>,shared_ptr<Pool<Connection,ConnectionCreator> > > guard(conn,pool);
         std::ostringstream s;
         boost::optional<mapnik::boolean> estimate_extent = params_.get<mapnik::boolean>("estimate_extent",false);

         if (!geometryColumn_.length() > 0)
         {
             std::ostringstream s_error;
             s_error << "PostGIS: unable to query the layer extent of table '"
             << schema_ << "." << geometry_table_ << "' because we cannot determine the geometry field name."
             << "\nPlease provide either 1) an 'extent' parameter to skip this query, "
             << "2) a 'geometry_field' and/or 'geometry_table' parameter, or 3) add a "
             << "record to the 'geometry_columns' for your table.";
             throw mapnik::datasource_exception(s_error.str());
         }

         if (estimate_extent && *estimate_extent)
         {
             s << "select xmin(ext),ymin(ext),xmax(ext),ymax(ext)"
               << " from (select estimated_extent('";

             if (schema_.length() > 0)
             {
                 s << unquote(schema_) << "','";
             }

             s << unquote(geometry_table_) << "','"
               << unquote(geometryColumn_) << "') as ext) as tmp";
         }
         else
         {
            s << "select xmin(ext),ymin(ext),xmax(ext),ymax(ext)"
              << " from (select extent(" <<geometryColumn_<< ") as ext from ";
              if (extent_from_subquery_)
              {
                  // if a subselect limits records then calculating the extent upon the
                  // subquery will be faster and the bounds will be more accurate
                  s << populate_tokens(table_) << ") as tmp";
              }
              else
              {
                  if (schema_.length() > 0)
                  {
                      s << schema_ << ".";
                  }

                  // but if the subquery does not limit records then querying the
                  // actual table will be faster as indexes can be used
                  s << geometry_table_ << ") as tmp";
              }
         }

         /*if (show_queries_)
         {
             clog << boost::format("PostGIS: sending query: %s\n") % s.str();
         }*/
            
         shared_ptr<ResultSet> rs=conn->executeQuery(s.str());
         if (rs->next())
         {
            try 
            {
               double lox=lexical_cast<double>(rs->getValue(0));
               double loy=lexical_cast<double>(rs->getValue(1));
               double hix=lexical_cast<double>(rs->getValue(2));
               double hiy=lexical_cast<double>(rs->getValue(3));		    
               extent_.init(lox,loy,hix,hiy);
               extent_initialized_ = true;
            }
            catch (bad_lexical_cast &ex)
            {
                clog << boost::format("PostGIS: warning: could not determine extent from query: %s\nError was: '%s'\n") % s.str() % ex.what();
            }
         }
         rs->close();
      }
   }
   return extent_;
}

postgis_datasource::~postgis_datasource() 
{
    if (!persist_connection_)
    {
        ConnectionManager *mgr=ConnectionManager::instance();
        shared_ptr<Pool<Connection,ConnectionCreator> > pool=mgr->getPool(creator_.id());
        if (pool)
        {
            shared_ptr<Connection> conn = pool->borrowObject();
            if (conn)
            {
                conn->close();
            }
        }
    }
}
