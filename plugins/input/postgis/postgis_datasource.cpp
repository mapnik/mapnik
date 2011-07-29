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
#include <mapnik/sql_utils.hpp>

#ifdef MAPNIK_DEBUG
//#include <mapnik/wall_clock_timer.hpp>
#endif

#include "connection_manager.hpp"
#include "postgis_datasource.hpp"
#include "postgis_featureset.hpp"

// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>

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

using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::shared_ptr;

using mapnik::PoolGuard;
using mapnik::attribute_descriptor;

postgis_datasource::postgis_datasource(parameters const& params, bool bind)
    : datasource(params),
      table_(*params_.get<std::string>("table","")),
      schema_(""),
      geometry_table_(*params_.get<std::string>("geometry_table","")),
      geometry_field_(*params_.get<std::string>("geometry_field","")),
      key_field_(*params_.get<std::string>("key_field","")),
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
               params.get<std::string>("password"),
               params.get<std::string>("connect_timeout","4")),
      bbox_token_("!bbox!"),
      scale_denom_token_("!scale_denominator!"),
      persist_connection_(*params_.get<mapnik::boolean>("persist_connection",true)),
      extent_from_subquery_(*params_.get<mapnik::boolean>("extent_from_subquery",false)),
      // params below are for testing purposes only (will likely be removed at any time)
      force2d_(*params_.get<mapnik::boolean>("force_2d",false)),
      st_(*params_.get<mapnik::boolean>("st_prefix",false))
      //show_queries_(*params_.get<mapnik::boolean>("show_queries",false))
{   
    if (table_.empty()) throw mapnik::datasource_exception("Postgis Plugin: missing <table> parameter");

    multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries",false);
   
    boost::optional<std::string> ext  = params_.get<std::string>("extent");
    if (ext) extent_initialized_ = extent_.from_string(*ext);

    if (bind)
    {
        this->bind();
    }
}

void postgis_datasource::bind() const
{
    if (is_bound_) return;
    
    boost::optional<int> initial_size = params_.get<int>("initial_size",1);
    boost::optional<int> max_size = params_.get<int>("max_size",10);

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
                geometry_table_ = mapnik::table_from_sql(table_);
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
                s << GEOMETRY_COLUMNS <<" WHERE f_table_name='" << mapnik::unquote_sql(geometry_table_) <<"'";
             
                if (schema_.length() > 0) 
                    s << " AND f_table_schema='" << mapnik::unquote_sql(schema_) << "'";
            
                if (geometry_field_.length() > 0)
                    s << " AND f_geometry_column='" << mapnik::unquote_sql(geometry_field_) << "'";

                /*
                if (show_queries_)
                {
                    std::clog << boost::format("PostGIS: sending query: %s\n") % s.str();
                }
                */
                 
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
                            std::clog << "Postgis Plugin: SRID=" << rs->getValue("srid") << " " << ex.what() << std::endl;
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
                    s << "SELECT ST_SRID(\"" << geometryColumn_ << "\") AS srid FROM ";
                    s << populate_tokens(table_) << " WHERE \"" << geometryColumn_ << "\" IS NOT NULL LIMIT 1;";

                    /*
                    if (show_queries_)
                    {
                        std::clog << boost::format("PostGIS: sending query: %s\n") % s.str();
                    }
                    */

                    shared_ptr<ResultSet> rs=conn->executeQuery(s.str());
                    if (rs->next())
                    {
                        try
                        {
                            srid_ = lexical_cast<int>(rs->getValue("srid"));
                        }
                        catch (bad_lexical_cast &ex)
                        {
                            std::clog << "Postgis Plugin: SRID=" << rs->getValue("srid") << " " << ex.what() << std::endl;
                        }
                    }
                    rs->close();
                }
            }
         
            if (srid_ == 0)
            {
                srid_ = -1;
                std::clog << "Postgis Plugin: SRID warning, using srid=-1" << std::endl;
            }

            // At this point the geometry_field may still not be known
            // but we'll catch that where more useful...         
#ifdef MAPNIK_DEBUG
            std::clog << "Postgis Plugin: using SRID=" << srid_ << std::endl;
            std::clog << "Postgis Plugin: using geometry_column=" << geometryColumn_ << std::endl;
#endif

            // collect attribute desc         
            std::ostringstream s;
            s << "SELECT * FROM " << populate_tokens(table_) << " LIMIT 0";

            
            /*
            if (show_queries_)
            {
                std::clog << boost::format("PostGIS: sending query: %s\n") % s.str();
            }
            */
            

            shared_ptr<ResultSet> rs=conn->executeQuery(s.str());
            int count = rs->getNumFields();
            bool found_key_field = false;
            for (int i=0;i<count;++i)
            {
                std::string fld_name=rs->getFieldName(i);
                int type_oid = rs->getTypeOID(i);

                // validate type of key_field
                if (!found_key_field && !key_field_.empty() && fld_name == key_field_)
                {
                    found_key_field = true;
                    if (type_oid == 20 || type_oid == 21 || type_oid == 23)
                    {
                        desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Integer));
                    }
                    else
                    {
                        std::ostringstream error_s;
                        error_s << "invalid type '";
                        std::ostringstream type_s;
                        type_s << "SELECT oid, typname FROM pg_type WHERE oid = " << type_oid;
                        shared_ptr<ResultSet> rs_oid = conn->executeQuery(type_s.str());
                        if (rs_oid->next())
                        {
                            error_s << rs_oid->getValue("typname")
                              << "' (oid:" << rs_oid->getValue("oid") << ")";
                        }
                        else
                        {
                            error_s << "oid:" << type_oid << "'";           
                        }
                        rs_oid->close();
                        error_s << " for key_field '" << fld_name << "' - "
                          << "must be an integer primary key";
                        rs->close();
                        throw mapnik::datasource_exception( error_s.str() );
                    }
                }
                else
                {
                    switch (type_oid)
                    {
                    case 16:    // bool
                        desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Boolean));
                        break;
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
                        s << "SELECT oid, typname FROM pg_type WHERE oid = " << type_oid;
    
                        /*
                        if (show_queries_)
                        {
                            std::clog << boost::format("PostGIS: sending query: %s\n") % s.str();
                        }
                        */
    
                        shared_ptr<ResultSet> rs_oid = conn->executeQuery(s.str());
                        if (rs_oid->next())
                        {
                            std::clog << "Postgis Plugin: unknown type = " << rs_oid->getValue("typname")
                                << " (oid:" << rs_oid->getValue("oid") << ")" << std::endl;
                        }
                        else
                        {
                            std::clog << "Postgis Plugin: unknown oid type =" << type_oid << std::endl;              
                        }
                        rs_oid->close();
    #endif
                        break;
                    }
                }
            }
            rs->close();
        }
    }
    
    is_bound_ = true;
}

std::string postgis_datasource::name()
{
    return "postgis";
}

int postgis_datasource::type() const
{
    return type_;
}

layer_descriptor postgis_datasource::get_descriptor() const
{
    if (!is_bound_) bind();
    return desc_;
}


std::string postgis_datasource::sql_bbox(box2d<double> const& env) const
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
        box2d<double> max_env(-1 * FMAX,-1 * FMAX,FMAX,FMAX);
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

std::string postgis_datasource::populate_tokens(const std::string& sql, double const& scale_denom, box2d<double> const& env) const
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


boost::shared_ptr<IResultSet> postgis_datasource::get_resultset(boost::shared_ptr<Connection> const &conn, const std::string &sql) const
{
    if (cursor_fetch_size_ > 0)
    {
        // cursor
        std::ostringstream csql;
        std::string cursor_name = conn->new_cursor_name();

        csql << "DECLARE " << cursor_name << " BINARY INSENSITIVE NO SCROLL CURSOR WITH HOLD FOR " << sql << " FOR READ ONLY";

        /*
        if (show_queries_)
        {
            std::clog << boost::format("PostGIS: sending query: %s\n") % csql.str();
        }
        */

        if (!conn->execute(csql.str()))
            throw mapnik::datasource_exception("Postgis Plugin: error creating cursor for data select." );

        return boost::make_shared<CursorResultSet>(conn, cursor_name, cursor_fetch_size_);

    }
    else
    {
        // no cursor

        /*
        if (show_queries_)
        {
            std::clog << boost::format("PostGIS: sending query: %s\n") % sql;
        }
        */

        return conn->executeQuery(sql,1);
    }
}

featureset_ptr postgis_datasource::features(const query& q) const
{
    if (!is_bound_) bind();
    
#ifdef MAPNIK_DEBUG
    //mapnik::wall_clock_progress_timer timer(clog, "end feature query: ");
#endif

    box2d<double> const& box = q.get_bbox();
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
                s_error << "PostGIS: geometry name lookup failed for table '";
                if (schema_.length() > 0)
                {
                    s_error << schema_ << ".";
                }
                s_error << geometry_table_
                        << "'. Please manually provide the 'geometry_field' parameter or add an entry "
                        << "in the geometry_columns for '";
                if (schema_.length() > 0)
                {
                    s_error << schema_ << ".";
                }
                s_error << geometry_table_ << "'.";
                throw mapnik::datasource_exception("Postgis Plugin: " + s_error.str());
            }

            std::ostringstream s;
            s << "SELECT ";
            if (st_)
                s << "ST_";
            if (force2d_)
                s << "AsBinary(ST_Force_2D(\"" << geometryColumn_ << "\")) AS geom";
            else
                s << "AsBinary(\"" << geometryColumn_ << "\") AS geom";

            if (!key_field_.empty()) 
                mapnik::quote_attr(s,key_field_);

            std::set<std::string> const& props=q.property_names();
            std::set<std::string>::const_iterator pos=props.begin();
            std::set<std::string>::const_iterator end=props.end();
            while (pos != end)
            {
                mapnik::quote_attr(s,*pos);
                ++pos;
            }       

            std::string table_with_bbox = populate_tokens(table_,scale_denom,box);

            s << " from " << table_with_bbox;

            if (row_limit_ > 0) {
                s << " LIMIT " << row_limit_;
            }
         
            boost::shared_ptr<IResultSet> rs = get_resultset(conn, s.str());
            unsigned num_attr = props.size();
            if (!key_field_.empty())
                ++num_attr;
            return boost::make_shared<postgis_featureset>(rs,desc_.get_encoding(),multiple_geometries_,!key_field_.empty(),props.size());
        }
        else 
        {
            throw mapnik::datasource_exception("Postgis Plugin: bad connection");
        }
    }
    return featureset_ptr();
}

featureset_ptr postgis_datasource::features_at_point(coord2d const& pt) const
{
    if (!is_bound_) bind();
    
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
                s_error << "PostGIS: geometry name lookup failed for table '";
                if (schema_.length() > 0)
                {
                    s_error << schema_ << ".";
                }
                s_error << "." << geometry_table_
                        << "'. Please manually provide the 'geometry_field' parameter or add an entry "
                        << "in the geometry_columns for '";
                if (schema_.length() > 0)
                {
                    s_error << schema_ << ".";
                }
                s_error << geometry_table_ << "'.";
                throw mapnik::datasource_exception(s_error.str());
            }
                    

            s << "SELECT ";
            if (st_)
                s << "ST_";
            if (force2d_)
                s << "AsBinary(ST_Force_2D(\"" << geometryColumn_ << "\")) AS geom";
            else
                s << "AsBinary(\"" << geometryColumn_ << "\") AS geom";
            
            if (!key_field_.empty())
                mapnik::quote_attr(s,key_field_);

            std::vector<attribute_descriptor>::const_iterator itr = desc_.get_descriptors().begin();
            std::vector<attribute_descriptor>::const_iterator end = desc_.get_descriptors().end();
            unsigned size=0;
            while (itr != end)
            {
                mapnik::quote_attr(s,itr->get_name());
                ++itr;
                ++size;
            }

            box2d<double> box(pt.x,pt.y,pt.x,pt.y);
            std::string table_with_bbox = populate_tokens(table_,FMAX,box);

            s << " from " << table_with_bbox;
         
            if (row_limit_ > 0) {
                s << " LIMIT " << row_limit_;
            }
         
            boost::shared_ptr<IResultSet> rs = get_resultset(conn, s.str());
            return boost::make_shared<postgis_featureset>(rs,desc_.get_encoding(),multiple_geometries_, !key_field_.empty(), size);
        }
    }
    return featureset_ptr();
}

box2d<double> postgis_datasource::envelope() const
{
    if (extent_initialized_) return extent_;
    if (!is_bound_) bind();
    
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
                s_error << "PostGIS: unable to query the layer extent of table '";
                if (schema_.length() > 0)
                {
                    s_error << schema_ << ".";
                }
                s_error << geometry_table_ << "' because we cannot determine the geometry field name."
                        << "\nPlease provide either an 'extent' parameter to skip this query, "
                        << "a 'geometry_field' and/or 'geometry_table' parameter, or add a "
                        << "record to the 'geometry_columns' for your table.";
                throw mapnik::datasource_exception("Postgis Plugin: " + s_error.str());
            }

            if (estimate_extent && *estimate_extent)
            {
                s << "SELECT ST_XMin(ext),ST_YMin(ext),ST_XMax(ext),ST_YMax(ext)"
                  << " FROM (SELECT ST_Estimated_Extent('";

                if (schema_.length() > 0)
                {
                    s << mapnik::unquote_sql(schema_) << "','";
                }

                s << mapnik::unquote_sql(geometry_table_) << "','"
                  << mapnik::unquote_sql(geometryColumn_) << "') as ext) as tmp";
            }
            else
            {
                s << "SELECT ST_XMin(ext),ST_YMin(ext),ST_XMax(ext),ST_YMax(ext)"
                  << " FROM (SELECT ST_Extent(" <<geometryColumn_<< ") as ext from ";
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

            /*
            if (show_queries_)
            {
                std::clog << boost::format("PostGIS: sending query: %s\n") % s.str();
            }
            */
            
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
                    std::clog << boost::format("Postgis Plugin: warning: could not determine extent from query: %s\nError was: '%s'\n") % s.str() % ex.what() << std::endl;
                }
            }
            rs->close();
        }
    }
    return extent_;
}

postgis_datasource::~postgis_datasource() 
{
    if (is_bound_ && !persist_connection_)
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
