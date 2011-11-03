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

#include "sqlite_datasource.hpp"
#include "sqlite_featureset.hpp"
#include "sqlite_resultset.hpp"
#include "sqlite_utils.hpp"

// mapnik
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/sql_utils.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/make_shared.hpp>
#include <boost/scoped_array.hpp>

using mapnik::box2d;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;
using mapnik::datasource_exception;
using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(sqlite_datasource)

sqlite_datasource::sqlite_datasource(parameters const& params, bool bind)
  : datasource(params),
    extent_(),
    extent_initialized_(false),
    type_(datasource::Vector),
    table_(*params_.get<std::string>("table", "")),
    fields_(*params_.get<std::string>("fields", "*")),
    metadata_(*params_.get<std::string>("metadata", "")),
    geometry_table_(*params_.get<std::string>("geometry_table", "")),
    geometry_field_(*params_.get<std::string>("geometry_field", "")),
    index_table_(*params_.get<std::string>("index_table", "")),
    key_field_(*params_.get<std::string>("key_field", "")),
    row_offset_(*params_.get<int>("row_offset", 0)),
    row_limit_(*params_.get<int>("row_limit", 0)),
    desc_(*params_.get<std::string>("type"), *params_.get<std::string>("encoding", "utf-8")),
    format_(mapnik::wkbAuto)
{
    /* TODO
      - throw if no primary key but spatial index is present?
      - remove auto-indexing
    */
    
    boost::optional<std::string> file = params_.get<std::string>("file");
    if (! file) throw datasource_exception("Sqlite Plugin: missing <file> parameter");

    if (table_.empty())
    {
        throw mapnik::datasource_exception("Sqlite Plugin: missing <table> parameter");
    }
    
    if (bind)
    {
        this->bind();
    }
}

void sqlite_datasource::bind() const
{
    if (is_bound_) return;
    
    boost::optional<std::string> file = params_.get<std::string>("file");
    if (! file) throw datasource_exception("Sqlite Plugin: missing <file> parameter");

    boost::optional<std::string> base = params_.get<std::string>("base");
    if (base)
        dataset_name_ = *base + "/" + *file;
    else
        dataset_name_ = *file;

    if ((dataset_name_.compare(":memory:") != 0) && (!boost::filesystem::exists(dataset_name_)))
    {
        throw datasource_exception("Sqlite Plugin: " + dataset_name_ + " does not exist");
    }    

    multiple_geometries_ = *params_.get<mapnik::boolean>("multiple_geometries", false);
    use_spatial_index_ = *params_.get<mapnik::boolean>("use_spatial_index", true);
    
    // TODO - remove this option once all datasources have an indexing api
    bool auto_index = *params_.get<mapnik::boolean>("auto_index", true);

    boost::optional<std::string> ext  = params_.get<std::string>("extent");
    if (ext) extent_initialized_ = extent_.from_string(*ext);
        
    boost::optional<std::string> wkb = params_.get<std::string>("wkb_format");
    if (wkb)
    {
        if (*wkb == "spatialite")
        {
            format_ = mapnik::wkbSpatiaLite;  
        }
        else if (*wkb == "generic")
        {
            format_ = mapnik::wkbGeneric;
        }
        else
        {
            format_ = mapnik::wkbAuto;
        }
    }

    // Populate init_statements_
    //   1. Build attach database statements from the "attachdb" parameter
    //   2. Add explicit init statements from "initdb" parameter
    // Note that we do some extra work to make sure that any attached
    // databases are relative to directory containing dataset_name_.  Sqlite
    // will default to attaching from cwd.  Typicaly usage means that the
    // map loader will produce full paths here.
    boost::optional<std::string> attachdb = params_.get<std::string>("attachdb");
    if (attachdb)
    {
        parse_attachdb(*attachdb);
    }
    
    boost::optional<std::string> initdb = params_.get<std::string>("initdb");
    if (initdb)
    {
        init_statements_.push_back(*initdb);
    }

    if (geometry_table_.empty())
    {
        geometry_table_ = mapnik::sql_utils::table_from_sql(table_);
    }

    // if 'table_' is a subquery then we try to deduce names
    // and types from the first row returned from that query
    using_subquery_ = false;
    if (table_ != geometry_table_)
    {
        using_subquery_ = true;
    }

    // now actually create the connection and start executing setup sql
    dataset_ = boost::make_shared<sqlite_connection>(dataset_name_);

    // Execute init_statements_
    for (std::vector<std::string>::const_iterator iter = init_statements_.begin();
         iter != init_statements_.end(); ++iter)
    {
#ifdef MAPNIK_DEBUG
        std::clog << "Sqlite Plugin: Execute init sql: " << *iter << std::endl;
#endif
        dataset_->execute(*iter);
    }
    
    bool found_types_via_subquery = false;
    if (using_subquery_)
    {
        std::ostringstream s;
        s << "SELECT " << fields_ << " FROM (" << table_ << ") LIMIT 1";
        found_types_via_subquery = sqlite_utils::detect_types_from_subquery(s.str(),geometry_field_,desc_,dataset_);
    }

    // TODO - consider removing this
    if (key_field_ == "rowid")
    {
        desc_.add_descriptor(attribute_descriptor("rowid", mapnik::Integer));
    }
    
    bool found_table = sqlite_utils::table_info(key_field_,
                                            found_types_via_subquery,
                                            geometry_field_,
                                            geometry_table_,
                                            desc_,
                                            dataset_);

    if (! found_table)
    {
        std::ostringstream s;
        s << "Sqlite Plugin: could not query table '" << geometry_table_ << "' ";
        if (using_subquery_) s << " from subquery '" << table_ << "' ";
        s << "using 'PRAGMA table_info(" << geometry_table_  << ")' ";

        std::string sq_err = std::string(sqlite3_errmsg(*(*dataset_)));
        if (sq_err != "unknown error") s << ": " << sq_err;

        throw datasource_exception(s.str());
    }

    if (geometry_field_.empty())
    {
        throw datasource_exception("Sqlite Plugin: cannot detect geometry_field, please supply the name of the geometry_field to use.");
    }

    if (index_table_.empty())
    {
        // Generate implicit index_table name - need to do this after
        // we have discovered meta-data or else we don't know the column name
        index_table_ = sqlite_utils::index_for_table(geometry_table_,geometry_field_);
    }

    std::string index_db = sqlite_utils::index_for_db(dataset_name_);
 
    has_spatial_index_ = false;
    if (use_spatial_index_)
    {
        if (boost::filesystem::exists(index_db))
        {
            dataset_->execute("attach database '" + index_db + "' as " + index_table_);
        }
        
        has_spatial_index_ = sqlite_utils::has_rtree(index_table_,dataset_);
    }


    if (! extent_initialized_ 
        && !has_spatial_index_
        && auto_index)
    {
            if (! key_field_.empty())
            {
                std::ostringstream query;
                query << "SELECT " 
                      << geometry_field_
                      << "," << key_field_
                      << " FROM ("
                      << geometry_table_ << ")";
                boost::shared_ptr<sqlite_resultset> rs = dataset_->execute_query(query.str());
                sqlite_utils::create_spatial_index(index_db,index_table_,rs,extent_);
                extent_initialized_ = true;
            }
            else
            {
                std::ostringstream s;
                s << "Sqlite Plugin: key_field is empty for " 
                  << geometry_field_
                  << " and "
                  << geometry_table_;
                throw datasource_exception(s.str());
            }
    }

    if (! extent_initialized_)
    {

        // TODO - clean this up - reducing arguments
        if (!sqlite_utils::detect_extent(dataset_,
                                         has_spatial_index_,
                                         extent_,
                                         index_table_,
                                         metadata_,
                                         geometry_field_,
                                         geometry_table_,
                                         key_field_,
                                         table_))
        {
            std::ostringstream s;
            s << "Sqlite Plugin: extent could not be determined for table '" 
              << geometry_table_ << "' and geometry field '" << geometry_field_ << "'"
              << " because an rtree spatial index is missing or empty."
              << " - either set the table 'extent' or create an rtree spatial index";
    
            throw datasource_exception(s.str());
        }
    }
    is_bound_ = true;
}

sqlite_datasource::~sqlite_datasource()
{
}

#if (BOOST_FILESYSTEM_VERSION <= 2)
namespace boost {
namespace filesystem {
  path read_symlink(const path& p)
  {
    path symlink_path;

#ifdef BOOST_POSIX_API
    for (std::size_t path_max = 64;; path_max *= 2)// loop 'til buffer is large enough
    {
      boost::scoped_array<char> buf(new char[path_max]);
      ssize_t result;
      if ((result=::readlink(p.string().c_str(), buf.get(), path_max))== -1)
      {
        throw std::runtime_error("could not read symlink");
      }
      else
      {
        if(result != static_cast<ssize_t>(path_max))
        {
          symlink_path.assign(buf.get(), buf.get() + result);
          break;
        }
      }
    }
#endif
    return symlink_path;
  }
}
}
#endif

void sqlite_datasource::parse_attachdb(std::string const& attachdb) const
{
    boost::char_separator<char> sep(",");
    boost::tokenizer<boost::char_separator<char> > tok(attachdb, sep);

    // The attachdb line is a comma sparated list of [dbname@]filename
    for (boost::tokenizer<boost::char_separator<char> >::iterator beg = tok.begin();
         beg != tok.end(); ++beg)
    {
        std::string const& spec(*beg);
        size_t atpos = spec.find('@');

        // See if it contains an @ sign
        if (atpos == spec.npos)
        {
            throw datasource_exception("attachdb parameter has syntax dbname@filename[,...]");
        }

        // Break out the dbname and the filename
        std::string dbname = boost::trim_copy(spec.substr(0, atpos));
        std::string filename = boost::trim_copy(spec.substr(atpos + 1));

        // Normalize the filename and make it relative to dataset_name_
        if (filename.compare(":memory:") != 0)
        {
            boost::filesystem::path child_path(filename);

            // It is a relative path.  Fix it.
            if (! child_path.has_root_directory() && ! child_path.has_root_name())
            {
                boost::filesystem::path absolute_path(dataset_name_);

                // support symlinks
                if (boost::filesystem::is_symlink(absolute_path))
                {
                    absolute_path = boost::filesystem::read_symlink(absolute_path);
                }

            #if (BOOST_FILESYSTEM_VERSION == 3)
                filename = boost::filesystem::absolute(absolute_path.parent_path() / filename).string();
            #else
                filename = boost::filesystem::complete(absolute_path.branch_path() / filename).normalize().string();
            #endif
            }
        }

        // And add an init_statement_
        init_statements_.push_back("attach database '" + filename + "' as " + dbname);
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
    if (! is_bound_) bind();

    return extent_;
}

layer_descriptor sqlite_datasource::get_descriptor() const
{
    if (! is_bound_) bind();

    return desc_;
}

featureset_ptr sqlite_datasource::features(query const& q) const
{
    if (! is_bound_) bind();

    if (dataset_)
    {
        mapnik::box2d<double> const& e = q.get_bbox();

        std::ostringstream s;
        
        s << "SELECT " << geometry_field_;
        if (!key_field_.empty())
            s << "," << key_field_;
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
        
        if (! key_field_.empty() && has_spatial_index_)
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
        
        if (row_limit_ > 0)
        {
            s << " LIMIT " << row_limit_;
        }

        if (row_offset_ > 0)
        {
            s << " OFFSET " << row_offset_;
        }

#ifdef MAPNIK_DEBUG
        std::clog << "Sqlite Plugin: table: " << table_ << "\n\n";
        std::clog << "Sqlite Plugin: query:" << s.str() << "\n\n";
#endif

        boost::shared_ptr<sqlite_resultset> rs(dataset_->execute_query(s.str()));

        return boost::make_shared<sqlite_featureset>(rs,
                                                     desc_.get_encoding(),
                                                     format_,
                                                     multiple_geometries_,
                                                     using_subquery_);
   }

   return featureset_ptr();
}

featureset_ptr sqlite_datasource::features_at_point(coord2d const& pt) const
{
    if (! is_bound_) bind();

    if (dataset_)
    {
        // TODO - need tolerance
        mapnik::box2d<double> const e(pt.x, pt.y, pt.x, pt.y);

        std::ostringstream s;
        s << "SELECT " << geometry_field_;
        if (!key_field_.empty())
            s << "," << key_field_;
        std::vector<attribute_descriptor>::const_iterator itr = desc_.get_descriptors().begin();
        std::vector<attribute_descriptor>::const_iterator end = desc_.get_descriptors().end();
        while (itr != end)
        {
            std::string fld_name = itr->get_name();
            if (fld_name != key_field_)
            {
                s << ",\"" << itr->get_name() << "\"";
            }

            ++itr;
        }
        
        s << " FROM "; 
        
        std::string query(table_);
        
        if (! key_field_.empty() && has_spatial_index_)
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
        
        if (row_limit_ > 0)
        {
            s << " LIMIT " << row_limit_;
        }

        if (row_offset_ > 0)
        {
            s << " OFFSET " << row_offset_;
        }

#ifdef MAPNIK_DEBUG
        std::clog << "Sqlite Plugin: " << s.str() << std::endl;
#endif

        boost::shared_ptr<sqlite_resultset> rs(dataset_->execute_query(s.str()));

        return boost::make_shared<sqlite_featureset>(rs,
                                                     desc_.get_encoding(),
                                                     format_,
                                                     multiple_geometries_,
                                                     using_subquery_);
   }
      
   return featureset_ptr();
}
