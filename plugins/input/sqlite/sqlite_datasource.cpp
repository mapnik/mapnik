/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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

#include "sqlite_datasource.hpp"
#include "sqlite_featureset.hpp"

// mapnik
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/sql_utils.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/make_shared.hpp>

using boost::lexical_cast;
using boost::bad_lexical_cast;

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(sqlite_datasource)

using mapnik::box2d;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;
using mapnik::datasource_exception;


sqlite_datasource::sqlite_datasource(parameters const& params, bool bind)
   : datasource(params),
     extent_(),
     extent_initialized_(false),
     type_(datasource::Vector),
     table_(*params_.get<std::string>("table","")),
     fields_(*params_.get<std::string>("fields","*")),
     metadata_(*params_.get<std::string>("metadata","")),
     geometry_table_(*params_.get<std::string>("geometry_table","")),
     geometry_field_(*params_.get<std::string>("geometry_field","")),
     // index_table_ defaults to "idx_{geometry_table_}_{geometry_field_}"
     index_table_(*params_.get<std::string>("index_table","")),
     // http://www.sqlite.org/lang_createtable.html#rowid
     key_field_(*params_.get<std::string>("key_field","rowid")),
     row_offset_(*params_.get<int>("row_offset",0)),
     row_limit_(*params_.get<int>("row_limit",0)),
     desc_(*params_.get<std::string>("type"), *params_.get<std::string>("encoding","utf-8")),
     format_(mapnik::wkbGeneric)
{
    // TODO
    // - change param from 'file' to 'dbname'
    // - be able to calculate extent manually - like ogr?
    // - require wkb_format?
    // - split plugin into sqlite vs spatialite?
    // - conversion/sqlite initialization tool
    // - sync code with rasterlite

    boost::optional<std::string> file = params_.get<std::string>("file");
    if (!file) throw datasource_exception("Sqlite Plugin: missing <file> parameter");

    if (table_.empty()) throw mapnik::datasource_exception("Sqlite Plugin: missing <table> parameter");
    
    boost::optional<std::string> wkb = params_.get<std::string>("wkb_format");
    if (wkb)
    {
        if (*wkb == "spatialite")
            format_ = mapnik::wkbSpatiaLite;  
    }

    multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries",false);
    use_spatial_index_ = *params_.get<mapnik::boolean>("use_spatial_index",true);

    boost::optional<std::string> ext  = params_.get<std::string>("extent");
    if (ext) extent_initialized_ = extent_.from_string(*ext);

    boost::optional<std::string> base = params_.get<std::string>("base");
    if (base)
        dataset_name_ = *base + "/" + *file;
    else
        dataset_name_ = *file;

    // Populate init_statements_
    //   1. Build attach database statements from the "attachdb" parameter
    //   2. Add explicit init statements from "initdb" parameter
    // Note that we do some extra work to make sure that any attached
    // databases are relative to directory containing dataset_name_.  Sqlite
    // will default to attaching from cwd.  Typicaly usage means that the
    // map loader will produce full paths here.
    boost::optional<std::string> attachdb = params_.get<std::string>("attachdb");
    if (attachdb) {
       parse_attachdb(*attachdb);
    }
    
    boost::optional<std::string> initdb = params_.get<std::string>("initdb");
    if (initdb) {
       init_statements_.push_back(*initdb);
    }
    
    if (bind)
    {
        this->bind();
    }
}

void sqlite_datasource::parse_attachdb(std::string const& attachdb) {
    boost::char_separator<char> sep(",");
    boost::tokenizer<boost::char_separator<char> > tok(attachdb, sep);
    
    // The attachdb line is a comma sparated list of
    //    [dbname@]filename
    for (boost::tokenizer<boost::char_separator<char> >::iterator beg = tok.begin(); 
         beg != tok.end(); ++beg)
    {
        std::string const& spec(*beg);
        size_t atpos=spec.find('@');
        
        // See if it contains an @ sign
        if (atpos==spec.npos) {
            throw datasource_exception("attachdb parameter has syntax dbname@filename[,...]");
        }
        
        // Break out the dbname and the filename
        std::string dbname = boost::trim_copy(spec.substr(0, atpos));
        std::string filename = boost::trim_copy(spec.substr(atpos+1));
        
        // Normalize the filename and make it relative to dataset_name_
        if (filename.compare(":memory:") != 0) {

            boost::filesystem::path child_path(filename);
            
            // It is a relative path.  Fix it.
            if (!child_path.has_root_directory() && !child_path.has_root_name()) {
                boost::filesystem::path absolute_path(dataset_name_);
                filename = boost::filesystem::absolute(absolute_path.parent_path()/filename).string();
            }
        }
        
        // And add an init_statement_
        init_statements_.push_back("attach database '" + filename + "' as " + dbname);
    }
}

void sqlite_datasource::bind() const
{
    if (is_bound_) return;
    
    if (!boost::filesystem::exists(dataset_name_))
        throw datasource_exception("Sqlite Plugin: " + dataset_name_ + " does not exist");
          
    dataset_ = new sqlite_connection (dataset_name_);

    // Execute init_statements_
    for (std::vector<std::string>::const_iterator iter=init_statements_.begin(); iter!=init_statements_.end(); ++iter)
    {
#ifdef MAPNIK_DEBUG
        std::clog << "Sqlite Plugin: Execute init sql: " << *iter << std::endl;
#endif
        dataset_->execute(*iter);
    }
         
    
    if(geometry_table_.empty())
    {
        geometry_table_ = mapnik::table_from_sql(table_);
    }
    

    // should we deduce column names and types using PRAGMA?
    bool use_pragma_table_info = true;
    
    if (table_ != geometry_table_)
    {
        // if 'table_' is a subquery then we try to deduce names
        // and types from the first row returned from that query
        use_pragma_table_info = false;
    }

    if (!use_pragma_table_info)
    {
        std::ostringstream s;
        s << "SELECT " << fields_ << " FROM (" << table_ << ") LIMIT 1";

        boost::scoped_ptr<sqlite_resultset> rs (dataset_->execute_query (s.str()));
        if (rs->is_valid () && rs->step_next())
        {
            for (int i = 0; i < rs->column_count (); ++i)
            {
               const int type_oid = rs->column_type (i);
               const char* fld_name = rs->column_name (i);
               switch (type_oid)
               {
                  case SQLITE_INTEGER:
                     desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Integer));
                     break;
                     
                  case SQLITE_FLOAT:
                     desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Double));
                     break;
                     
                  case SQLITE_TEXT:
                     desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::String));
                     break;
                     
                  case SQLITE_NULL:
                     // sqlite reports based on value, not actual column type unless
                     // PRAGMA table_info is used so here we assume the column is a string
                     // which is a lesser evil than altogether dropping the column
                     desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::String));

                  case SQLITE_BLOB:
                        if (geometry_field_.empty() && 
                             (boost::algorithm::icontains(fld_name,"geom") ||
                              boost::algorithm::icontains(fld_name,"point") ||
                              boost::algorithm::icontains(fld_name,"linestring") ||
                              boost::algorithm::icontains(fld_name,"polygon"))
                            )
                            geometry_field_ = std::string(fld_name);
                     break;
                     
                  default:
#ifdef MAPNIK_DEBUG
                     std::clog << "Sqlite Plugin: unknown type_oid=" << type_oid << std::endl;
#endif
                     break;
                }
            }
        }
        else
        {
            // if we do not have at least a row and
            // we cannot determine the right columns types and names 
            // as all column_type are SQLITE_NULL
            // so we fallback to using PRAGMA table_info
            use_pragma_table_info = true;
        }
    }

    // TODO - ensure that the supplied key_field is a valid "integer primary key"
    desc_.add_descriptor(attribute_descriptor("rowid",mapnik::Integer));
    
    if (use_pragma_table_info)
    {
        std::ostringstream s;
        s << "PRAGMA table_info(" << geometry_table_ << ")";
        boost::scoped_ptr<sqlite_resultset> rs (dataset_->execute_query (s.str()));
        bool found_table = false;
        while (rs->is_valid () && rs->step_next())
        {
            found_table = true;
            const char* fld_name = rs->column_text(1);
            std::string fld_type(rs->column_text(2));
            boost::algorithm::to_lower(fld_type);

            // see 2.1 "Column Affinity" at http://www.sqlite.org/datatype3.html
            if (geometry_field_.empty() && 
                       (
                       (boost::algorithm::icontains(fld_name,"geom") ||
                        boost::algorithm::icontains(fld_name,"point") ||
                        boost::algorithm::icontains(fld_name,"linestring") ||
                        boost::algorithm::icontains(fld_name,"polygon"))
                       ||
                       (boost::algorithm::contains(fld_type,"geom") ||
                        boost::algorithm::contains(fld_type,"point") ||
                        boost::algorithm::contains(fld_type,"linestring") ||
                        boost::algorithm::contains(fld_type,"polygon"))
                       )
                    )
                    geometry_field_ = std::string(fld_name);
            else if (boost::algorithm::contains(fld_type,"int"))
            {
                desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Integer));
            }
            else if (boost::algorithm::contains(fld_type,"text") ||
                     boost::algorithm::contains(fld_type,"char") ||
                     boost::algorithm::contains(fld_type,"clob"))
            {
                desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::String));
            }
            else if (boost::algorithm::contains(fld_type,"real") ||
                     boost::algorithm::contains(fld_type,"float") ||
                     boost::algorithm::contains(fld_type,"double"))
            {
                desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Double));
            }
            else if (boost::algorithm::contains(fld_type,"blob") && !geometry_field_.empty())
            {
                 desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::String));
            }
#ifdef MAPNIK_DEBUG
            else
            {
                // "Column Affinity" says default to "Numeric" but for now we pass..
                //desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Double));
                std::clog << "Sqlite Plugin: column '" << std::string(fld_name) << "' unhandled due to unknown type: " << fld_type << std::endl;
            }
#endif
        }
        if (!found_table)
        {
            throw datasource_exception("Sqlite Plugin: could not query table '" + geometry_table_ + "' using 'pragma table_info(" + geometry_table_  + ")';");
        }
    }

    if (geometry_field_.empty()) {
        throw datasource_exception("Sqlite Plugin: cannot detect geometry_field, please supply the name of the geometry_field to use.");
    }
    
    if (use_spatial_index_)
    {
        if (index_table_.size() == 0) {
            // Generate implicit index_table name - need to do this after
            // we have discovered meta-data or else we don't know the column
            // name
            index_table_ = "idx_" + mapnik::unquote_sql(geometry_table_) + "_" + geometry_field_;
        }
        
        std::ostringstream s;
        s << "SELECT pkid,xmin,xmax,ymin,ymax FROM " << index_table_;
        s << " LIMIT 0";
        if (dataset_->execute_with_code(s.str()) != SQLITE_OK)
        {
            use_spatial_index_ = false;
            std::clog << "Sqlite Plugin: Warning, no suitable spatial index found for "
                << geometry_table_ << " (checked " << s.str() << "). Rendering will work but will be slow: set 'use_spatial_index' to false to silence this warning." << std::endl;
        }
    }

    if (metadata_ != "" && !extent_initialized_)
    {
        std::ostringstream s;
        s << "SELECT xmin, ymin, xmax, ymax FROM " << metadata_;
        s << " WHERE LOWER(f_table_name) = LOWER('" << geometry_table_ << "')";
        boost::scoped_ptr<sqlite_resultset> rs (dataset_->execute_query (s.str()));
        if (rs->is_valid () && rs->step_next())
        {
            double xmin = rs->column_double (0);
            double ymin = rs->column_double (1);
            double xmax = rs->column_double (2);
            double ymax = rs->column_double (3);

            extent_.init (xmin,ymin,xmax,ymax);
            extent_initialized_ = true;
        }
    }
    
    if (!extent_initialized_ && use_spatial_index_)
    {
        std::ostringstream s;
        s << "SELECT MIN(xmin), MIN(ymin), MAX(xmax), MAX(ymax) FROM " 
        << index_table_;
        boost::scoped_ptr<sqlite_resultset> rs (dataset_->execute_query (s.str()));
        if (rs->is_valid () && rs->step_next())
        {
            if (!rs->column_isnull(0)) {
                try 
                {
                    double xmin = lexical_cast<double>(rs->column_double(0));
                    double ymin = lexical_cast<double>(rs->column_double(1));
                    double xmax = lexical_cast<double>(rs->column_double(2));
                    double ymax = lexical_cast<double>(rs->column_double(3));
                    extent_.init (xmin,ymin,xmax,ymax);
                    extent_initialized_ = true;
    
                }
                catch (bad_lexical_cast &ex)
                {
                    std::clog << boost::format("SQLite Plugin: warning: could not determine extent from query: %s\nError was: '%s'\n") % s.str() % ex.what() << std::endl;
                }
            }
        }    
    }
    
    /* TODO - instead of throwing here we should:
      - perhaps warn to std::clog, then..
      - form up a query of the whole table
      - loop over geoms, assuming wkb format
      - parse geoms and collect cumulative extent using box2d::expand_to_include
      - this would match the postgis behavior
    */

    if (!extent_initialized_) {
        std::ostringstream s;
        s << "Sqlite Plugin: extent could not be determined for table '" 
          << geometry_table_ << "' and geometry field '" << geometry_field_ << "'"
          << " because an rtree spatial index is missing or empty."
          << " - either set the table 'extent' or create an rtree spatial index";
        throw datasource_exception(s.str());
    }
    
    is_bound_ = true;
}

sqlite_datasource::~sqlite_datasource()
{
    if (is_bound_) 
    {
        delete dataset_;
    }
}

std::string sqlite_datasource::name()
{
   return "sqlite";
}

int sqlite_datasource::type() const
{
   return type_;
}

box2d<double> sqlite_datasource::envelope() const
{
   if (!is_bound_) bind();
   return extent_;
}

layer_descriptor sqlite_datasource::get_descriptor() const
{
   if (!is_bound_) bind();
   return desc_;
}

featureset_ptr sqlite_datasource::features(query const& q) const
{
   if (!is_bound_) bind();
   if (dataset_)
   {
        mapnik::box2d<double> const& e = q.get_bbox();

        std::ostringstream s;
        
        s << "SELECT " << geometry_field_ << "," << key_field_;
        std::set<std::string> const& props = q.property_names();
        std::set<std::string>::const_iterator pos = props.begin();
        std::set<std::string>::const_iterator end = props.end();
        while (pos != end)
        {
            s << ",\"" << *pos << "\"";
            ++pos;
        }       
        
        s << " FROM "; 
        
        std::string query (table_); 
        
        if (use_spatial_index_)
        {
           std::ostringstream spatial_sql;
           spatial_sql << std::setprecision(16);
           spatial_sql << " WHERE " << key_field_ << " IN (SELECT pkid FROM " << index_table_;
           spatial_sql << " WHERE xmax>=" << e.minx() << " AND xmin<=" << e.maxx() ;
           spatial_sql << " AND ymax>=" << e.miny() << " AND ymin<=" << e.maxy() << ")";
           if (boost::algorithm::ifind_first(query, "WHERE"))
           {
              boost::algorithm::ireplace_first(query, "WHERE", spatial_sql.str() + " AND ");
           }
           else if (boost::algorithm::ifind_first(query, geometry_table_))  
           {
              boost::algorithm::ireplace_first(query, table_, table_ + " " + spatial_sql.str());
           }
        }
        
        s << query ;
        
        if (row_limit_ > 0) {
            s << " LIMIT " << row_limit_;
        }

        if (row_offset_ > 0) {
            s << " OFFSET " << row_offset_;
        }

#ifdef MAPNIK_DEBUG
        std::clog << "Sqlite Plugin: " << s.str() << std::endl;
#endif

        boost::shared_ptr<sqlite_resultset> rs (dataset_->execute_query (s.str()));

        return boost::make_shared<sqlite_featureset>(rs, desc_.get_encoding(), format_, multiple_geometries_);
   }

   return featureset_ptr();
}

featureset_ptr sqlite_datasource::features_at_point(coord2d const& pt) const
{
   if (!is_bound_) bind();

   if (dataset_)
   {
        // TODO - need tolerance
        mapnik::box2d<double> const e(pt.x,pt.y,pt.x,pt.y);

        std::ostringstream s;
        s << "SELECT " << geometry_field_ << "," << key_field_;
        std::vector<attribute_descriptor>::const_iterator itr = desc_.get_descriptors().begin();
        std::vector<attribute_descriptor>::const_iterator end = desc_.get_descriptors().end();
        while (itr != end)
        {
            std::string fld_name = itr->get_name();
            if (fld_name != key_field_)
                s << ",\"" << itr->get_name() << "\"";
            ++itr;
        }
        
        s << " FROM "; 
        
        std::string query (table_); 
        
        if (use_spatial_index_)
        {
           std::ostringstream spatial_sql;
           spatial_sql << std::setprecision(16);
           spatial_sql << " WHERE " << key_field_ << " IN (SELECT pkid FROM " << index_table_;
           spatial_sql << " WHERE xmax>=" << e.minx() << " AND xmin<=" << e.maxx() ;
           spatial_sql << " AND ymax>=" << e.miny() << " AND ymin<=" << e.maxy() << ")";
           if (boost::algorithm::ifind_first(query, "WHERE"))
           {
              boost::algorithm::ireplace_first(query, "WHERE", spatial_sql.str() + " AND ");
           }
           else if (boost::algorithm::ifind_first(query, geometry_table_))  
           {
              boost::algorithm::ireplace_first(query, table_, table_ + " " + spatial_sql.str());
           }
        }
        
        s << query ;
        
        if (row_limit_ > 0) {
            s << " LIMIT " << row_limit_;
        }

        if (row_offset_ > 0) {
            s << " OFFSET " << row_offset_;
        }

#ifdef MAPNIK_DEBUG
        std::clog << "Sqlite Plugin: " << s.str() << std::endl;
#endif

        boost::shared_ptr<sqlite_resultset> rs (dataset_->execute_query (s.str()));

        return boost::make_shared<sqlite_featureset>(rs, desc_.get_encoding(), format_, multiple_geometries_);
   }
      
   return featureset_ptr();
}

