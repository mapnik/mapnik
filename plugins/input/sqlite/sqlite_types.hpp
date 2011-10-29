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

#ifndef SQLITE_TYPES_HPP
#define SQLITE_TYPES_HPP

// stl
#include <string.h>

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/sql_utils.hpp>
#include <mapnik/feature_factory.hpp> // to enable extent fallback hack


// boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

// sqlite
extern "C" {
  #include <sqlite3.h>
}



//==============================================================================

class sqlite_resultset
{
public:

    sqlite_resultset (sqlite3_stmt* stmt)
        : stmt_(stmt)
    {
    }

    ~sqlite_resultset ()
    {
        if (stmt_)
        {
            sqlite3_finalize (stmt_);
        }
    }

    bool is_valid ()
    {
        return stmt_ != 0;
    }

    bool step_next ()
    {
        const int status = sqlite3_step (stmt_);
        if (status != SQLITE_ROW && status != SQLITE_DONE)
        {
            std::ostringstream s;
            s << "SQLite Plugin: retrieving next row failed";
            std::string msg(sqlite3_errmsg(sqlite3_db_handle(stmt_)));
            if (msg != "unknown error")
            {
                s << ": " << msg;
            }

            throw mapnik::datasource_exception(s.str());
        }
        return status == SQLITE_ROW;
    }

    int column_count ()
    {
        return sqlite3_column_count (stmt_);
    }

    int column_type (int col)
    {
        return sqlite3_column_type (stmt_, col);
    }
    
    const char* column_name (int col)
    {
        return sqlite3_column_name (stmt_, col);
    }

    bool column_isnull (int col)
    {
        return sqlite3_column_type (stmt_, col) == SQLITE_NULL;
    }

    int column_integer (int col)
    {
        return sqlite3_column_int (stmt_, col);
    }

    int column_integer64 (int col)
    {
        return sqlite3_column_int64 (stmt_, col);
    }

    double column_double (int col)
    {
        return sqlite3_column_double (stmt_, col);
    }

    const char* column_text (int col, int& len)
    {
        len = sqlite3_column_bytes (stmt_, col);
        return (const char*) sqlite3_column_text (stmt_, col);
    }

    const char* column_text (int col)
    {
        return (const char*) sqlite3_column_text (stmt_, col);
    }

    const void* column_blob (int col, int& bytes)
    {
        bytes = sqlite3_column_bytes (stmt_, col);
        return (const char*) sqlite3_column_blob (stmt_, col);
    }

    sqlite3_stmt* get_statement()
    {
        return stmt_;
    }

private:

    sqlite3_stmt* stmt_;
};


//==============================================================================

class sqlite_connection
{
public:

    sqlite_connection (const std::string& file)
        : db_(0),
          file_(file)
    {
#if SQLITE_VERSION_NUMBER >= 3005000
        int mode = SQLITE_OPEN_READWRITE;
        #if SQLITE_VERSION_NUMBER >= 3006018
            // shared cache flag not available until >= 3.6.18
            mode |= SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE;
        #endif
        const int rc = sqlite3_open_v2 (file_.c_str(), &db_, mode, 0);
#else
        #warning "Mapnik's sqlite plugin is compiling against a version of sqlite older than 3.5.x which may make rendering slow..."
        const int rc = sqlite3_open (file_.c_str(), &db_);
#endif
        if (rc != SQLITE_OK)
        {
            std::ostringstream s;
            s << "Sqlite Plugin: " << sqlite3_errmsg (db_);

            throw mapnik::datasource_exception (s.str());
        }
        
        sqlite3_busy_timeout(db_,5000);
    }

    sqlite_connection (const std::string& file, int flags)
        : db_(0),
          file_(file)
    {
#if SQLITE_VERSION_NUMBER >= 3005000
        const int rc = sqlite3_open_v2 (file_.c_str(), &db_, flags, 0);
#else
        #warning "Mapnik's sqlite plugin is compiling against a version of sqlite older than 3.5.x which may make rendering slow..."
        const int rc = sqlite3_open (file_.c_str(), &db_);
#endif
        if (rc != SQLITE_OK)
        {
            std::ostringstream s;
            s << "Sqlite Plugin: " << sqlite3_errmsg (db_);

            throw mapnik::datasource_exception (s.str());
        }
    }

    virtual ~sqlite_connection ()
    {
        if (db_)
        {
            sqlite3_close (db_);
        }
    }

    void throw_sqlite_error(const std::string& sql)
    {
        std::ostringstream s;
        s << "Sqlite Plugin: ";
        if (db_)
            s << "'" << sqlite3_errmsg(db_) << "'";
        else
            s << "unknown error, lost connection";
        s << " (" << file_ << ")"
          << "\nFull sql was: '" 
          <<  sql << "'";

        throw mapnik::datasource_exception (s.str());
    }
    
    sqlite_resultset* execute_query(const std::string& sql)
    {
        sqlite3_stmt* stmt = 0;

        const int rc = sqlite3_prepare_v2 (db_, sql.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            throw_sqlite_error(sql);
        }

        return new sqlite_resultset (stmt);
    }
  
    void execute(const std::string& sql)
    {
        const int rc = sqlite3_exec(db_, sql.c_str(), 0, 0, 0);
        if (rc != SQLITE_OK)
        {
            throw_sqlite_error(sql);
        }
    }

    int execute_with_code(const std::string& sql)
    {
        const int rc = sqlite3_exec(db_, sql.c_str(), 0, 0, 0);
        return rc;
    }
   
    sqlite3* operator*()
    {
        return db_;
    }

    bool has_rtree(std::string const& index_table)
    {
        try
        {
            std::ostringstream s;
            s << "SELECT pkid,xmin,xmax,ymin,ymax FROM " << index_table << " LIMIT 1";
            boost::scoped_ptr<sqlite_resultset> rs(execute_query(s.str()));
            if (rs->is_valid() && rs->step_next())
            {
                return true;
            }
        }
        catch (...)
        {
            return false;
        }
        return false;
    }

    bool detect_types_from_subquery(std::string const& query,
                                    std::string & geometry_field,
                                    mapnik::layer_descriptor & desc)
    {
        bool found = false;
        boost::scoped_ptr<sqlite_resultset> rs(execute_query(query));
        if (rs->is_valid() && rs->step_next())
        {
            for (int i = 0; i < rs->column_count(); ++i)
            {
                found = true;
                const int type_oid = rs->column_type(i);
                const char* fld_name = rs->column_name(i);
                switch (type_oid)
                {
                case SQLITE_INTEGER:
                    desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::Integer));
                    break;
                     
                case SQLITE_FLOAT:
                    desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::Double));
                    break;
                     
                case SQLITE_TEXT:
                    desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::String));
                    break;
                     
                case SQLITE_NULL:
                    // sqlite reports based on value, not actual column type unless
                    // PRAGMA table_info is used so here we assume the column is a string
                    // which is a lesser evil than altogether dropping the column
                    desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::String));

                case SQLITE_BLOB:
                    if (geometry_field.empty()
                        && (boost::algorithm::icontains(fld_name, "geom") ||
                            boost::algorithm::icontains(fld_name, "point") ||
                            boost::algorithm::icontains(fld_name, "linestring") ||
                            boost::algorithm::icontains(fld_name, "polygon")))
                    {
                        geometry_field = std::string(fld_name);
                    }
                    break;
                     
                default:
#ifdef MAPNIK_DEBUG
                    std::clog << "Sqlite Plugin: unknown type_oid=" << type_oid << std::endl;
#endif
                    break;
                }
            }
        }
        
        return found;
    }

    bool table_info(std::string & key_field,
                    bool detected_types,
                    std::string & field,
                    std::string & table,
                    mapnik::layer_descriptor & desc)
    {

        // http://www.sqlite.org/pragma.html#pragma_table_info
        // use pragma table_info to detect primary key
        // and to detect types if no subquery is used or 
        // if the subquery-based type detection failed
        std::ostringstream s;
        s << "PRAGMA table_info(" << table << ")";
        boost::scoped_ptr<sqlite_resultset> rs(execute_query(s.str()));
        bool found_table = false;
        bool found_pk = false;
        while (rs->is_valid() && rs->step_next())
        {
            found_table = true;
            const char* fld_name = rs->column_text(1);
            std::string fld_type(rs->column_text(2));
            int fld_pk = rs->column_integer(5);
            boost::algorithm::to_lower(fld_type);
    
            // TODO - how to handle primary keys on multiple columns ?
            if (key_field.empty() && ! found_pk && fld_pk != 0)
            {
                key_field = fld_name;
                found_pk = true;
            }
            if (! detected_types)
            {
                // see 2.1 "Column Affinity" at http://www.sqlite.org/datatype3.html
                // TODO - refactor this somehow ?
                if (field.empty()
                    && ((boost::algorithm::contains(fld_type, "geom") ||
                         boost::algorithm::contains(fld_type, "point") ||
                         boost::algorithm::contains(fld_type, "linestring") ||
                         boost::algorithm::contains(fld_type, "polygon"))
                    ||
                        (boost::algorithm::icontains(fld_name, "geom") ||
                         boost::algorithm::icontains(fld_name, "point") ||
                         boost::algorithm::icontains(fld_name, "linestring") ||
                         boost::algorithm::icontains(fld_name, "polygon")))
                   )
                {
                    field = std::string(fld_name);
                }
                else if (boost::algorithm::contains(fld_type, "int"))
                {
                    desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::Integer));
                }
                else if (boost::algorithm::contains(fld_type, "text") ||
                         boost::algorithm::contains(fld_type, "char") ||
                         boost::algorithm::contains(fld_type, "clob"))
                {
                    desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::String));
                }
                else if (boost::algorithm::contains(fld_type, "real") ||
                         boost::algorithm::contains(fld_type, "float") ||
                         boost::algorithm::contains(fld_type, "double"))
                {
                    desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::Double));
                }
                else if (boost::algorithm::contains(fld_type, "blob"))
                {
                    if (! field.empty())
                    {
                        desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::String));
                    }
                }
        #ifdef MAPNIK_DEBUG
                else
                {
                    // "Column Affinity" says default to "Numeric" but for now we pass..
                    //desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Double));
        
                    // TODO - this should not fail when we specify geometry_field in XML file
        
                    std::clog << "Sqlite Plugin: column '"
                              << std::string(fld_name)
                              << "' unhandled due to unknown type: "
                              << fld_type << std::endl;
                }
        #endif
            }
        }
        
        return found_table;
    }
    
private:

    sqlite3* db_;
    std::string file_;
};


//==============================================================================

class sqlite_utils
{
public:

    static void dequote(std::string & z)
    {
        boost::algorithm::trim_if(z,boost::algorithm::is_any_of("[]'\"`"));
    }
    
    static std::string index_for_table(std::string const& table,std::string const& field)
    {
        return "\"idx_" + mapnik::unquote_sql2(table) + "_" + field + "\"";
    }

    static std::string get_index_db_name(std::string const& file)
    {
        //std::size_t idx = file.find_last_of(".");
        //if(idx != std::string::npos) {
        //    return file.substr(0,idx) + ".index" + file.substr(idx);
        //}
        //else
        //{
            return file + ".index";
        //}
    }    
    
    static void query_extent(boost::shared_ptr<sqlite_resultset> rs,
                           mapnik::box2d<double>& extent)
    {

        bool first = true;
        while (rs->is_valid() && rs->step_next())
        {
            int size;
            const char* data = (const char*) rs->column_blob(0, size);
            if (data)
            {
                // create a feature to parse geometry into
                // see: http://trac.mapnik.org/ticket/745
                mapnik::feature_ptr feature(mapnik::feature_factory::create(0));
                mapnik::geometry_utils::from_wkb(feature->paths(), data, size, false, mapnik::wkbAuto);
                mapnik::box2d<double> const& bbox = feature->envelope();

                if (bbox.valid())
                {
                    if (first)
                    {
                        first = false;
                        extent = bbox;
                    }
                    else
                    {
                        extent.expand_to_include(bbox);
                    }
                }
            }
        }
    }
    
    static void create_spatial_index(std::string const& index_db,
                                     std::string const& table,
                                     std::string const& key_field,
                                     boost::shared_ptr<sqlite_resultset> rs,
                                     mapnik::box2d<double>& extent)
    {
        // TODO - return early/recover from failure?
        // TODO - allow either in db or in separate
        
        //std::clog << "creating rtree: " << index_db << " : " << table << "\n";
#if SQLITE_VERSION_NUMBER >= 3005000
        int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#else
        //int flags;
#endif

        boost::shared_ptr<sqlite_connection> dataset_ = boost::make_shared<sqlite_connection>(index_db,flags);
    
        dataset_->execute("BEGIN TRANSACTION");
        dataset_->execute("PRAGMA synchronous=OFF");
        // first drop the index if it already exists
        std::ostringstream spatial_index_drop_sql;
        spatial_index_drop_sql << "DROP TABLE IF EXISTS " << table;
        dataset_->execute(spatial_index_drop_sql.str());

        // create the spatial index
        std::ostringstream spatial_index_sql;
        spatial_index_sql << "create virtual table " << table
                          << " using rtree(pkid, xmin, xmax, ymin, ymax)";

        std::ostringstream spatial_index_insert_sql;
        spatial_index_insert_sql << "insert into " << table
                                 << " values (?,?,?,?,?)";

        sqlite3_stmt* stmt = 0;
        dataset_->execute(spatial_index_sql.str());

        const int rc = sqlite3_prepare_v2 (*(*dataset_),
                                           spatial_index_insert_sql.str().c_str(),
                                           -1,
                                           &stmt,
                                           0);
        if (rc != SQLITE_OK)
        {
            std::ostringstream index_error;
            index_error << "Sqlite Plugin: auto-index table creation failed: '"
                        << sqlite3_errmsg(*(*dataset_)) << "' query was: "
                        << spatial_index_insert_sql;

            throw mapnik::datasource_exception(index_error.str());
        }

        bool first = true;
        while (rs->is_valid() && rs->step_next())
        {
            int size;
            const char* data = (const char*) rs->column_blob(0, size);
            if (data)
            {
                // create a tmp feature to be able to parse geometry
                // ideally we would not have to do this.
                // see: http://trac.mapnik.org/ticket/745
                mapnik::feature_ptr feature(mapnik::feature_factory::create(0));
                mapnik::geometry_utils::from_wkb(feature->paths(), data, size, false, mapnik::wkbAuto);
                mapnik::box2d<double> bbox = feature->envelope();

                if (bbox.valid())
                {
                    if (first)
                    {
                        first = false;
                        extent = bbox;
                    }
                    else
                    {
                        extent.expand_to_include(bbox);
                    }

                    const int type_oid = rs->column_type(1);
                    if (type_oid != SQLITE_INTEGER)
                    {
                        std::ostringstream type_error;
                        type_error << "Sqlite Plugin: invalid type for key field '"
                                   << key_field << "' when creating index '" << table
                                   << "' type was: " << type_oid << "";

                        throw mapnik::datasource_exception(type_error.str());
                    }

                    const sqlite_int64 pkid = rs->column_integer64(1);
                    if (sqlite3_bind_int64(stmt, 1, pkid) != SQLITE_OK)
                    {
                        throw mapnik::datasource_exception("invalid value for for key field while generating index");
                    }

                    if ((sqlite3_bind_double(stmt, 2, bbox.minx()) != SQLITE_OK) ||
                        (sqlite3_bind_double(stmt, 3, bbox.maxx()) != SQLITE_OK) ||
                        (sqlite3_bind_double(stmt, 4, bbox.miny()) != SQLITE_OK) ||
                        (sqlite3_bind_double(stmt, 5, bbox.maxy()) != SQLITE_OK))
                    {
                        throw mapnik::datasource_exception("invalid value for for extent while generating index");
                    }

                    const int res = sqlite3_step(stmt);
                    if (res != SQLITE_ROW && res != SQLITE_DONE)
                    {
                        std::ostringstream s;
                        s << "SQLite Plugin: inserting bbox into rtree index failed: "
                             << "error code " << sqlite3_errcode(*(*dataset_)) << ": '"
                             << sqlite3_errmsg(*(*dataset_)) << "' query was: "
                             << spatial_index_insert_sql;

                        throw mapnik::datasource_exception(s.str());
                    }

                    sqlite3_reset(stmt);
                }
                else
                {
                    std::ostringstream index_error;
                    index_error << "SQLite Plugin: encountered invalid bbox at '"
                                << key_field << "' == " << rs->column_integer64(1);

                    throw mapnik::datasource_exception(index_error.str());
                }
            }
        }
        
        const int res = sqlite3_finalize(stmt);
        if (res != SQLITE_OK)
        {
            throw mapnik::datasource_exception("auto-indexing failed: set use_spatial_index=false to disable auto-indexing and avoid this error");
        }
        
        dataset_->execute("COMMIT");
    }

    static bool detect_extent(boost::shared_ptr<sqlite_connection> dataset_,
                              bool has_spatial_index,
                              mapnik::box2d<double> & extent,
                              std::string const& index_table,
                              std::string const& metadata,
                              std::string const& geometry_field,
                              std::string const& geometry_table,
                              std::string const& key_field,
                              std::string const& table
                              )
    {
        if (has_spatial_index)
        {
            std::ostringstream s;
            s << "SELECT MIN(xmin), MIN(ymin), MAX(xmax), MAX(ymax) FROM " 
            << index_table;
    
            boost::scoped_ptr<sqlite_resultset> rs(dataset_->execute_query(s.str()));
            if (rs->is_valid() && rs->step_next())
            {
                if (! rs->column_isnull(0))
                {
                    try 
                    {
                        double xmin = boost::lexical_cast<double>(rs->column_double(0));
                        double ymin = boost::lexical_cast<double>(rs->column_double(1));
                        double xmax = boost::lexical_cast<double>(rs->column_double(2));
                        double ymax = boost::lexical_cast<double>(rs->column_double(3));
                        extent.init(xmin, ymin, xmax, ymax);
                        return true;
                    }
                    catch (boost::bad_lexical_cast& ex)
                    {
                        std::ostringstream ss;
                        ss << "SQLite Plugin: warning: could not determine extent from query: "
                           << "'" << s.str() << "' \n problem was: " << ex.what() << std::endl;
                        std::clog << ss.str();
                    }
                }
            }
        }
        else if (! metadata.empty())
        {
            std::ostringstream s;
            s << "SELECT xmin, ymin, xmax, ymax FROM " << metadata;
            s << " WHERE LOWER(f_table_name) = LOWER('" << geometry_table << "')";
            boost::scoped_ptr<sqlite_resultset> rs(dataset_->execute_query(s.str()));
            if (rs->is_valid() && rs->step_next())
            {
                double xmin = rs->column_double (0);
                double ymin = rs->column_double (1);
                double xmax = rs->column_double (2);
                double ymax = rs->column_double (3);
                extent.init (xmin, ymin, xmax, ymax);
                return true;
            }
        }
        else if (! key_field.empty())
        {
            std::ostringstream s;
            s << "SELECT " << geometry_field << "," << key_field
              << " FROM (" << table << ")";
            boost::shared_ptr<sqlite_resultset> rs(dataset_->execute_query(s.str()));
            sqlite_utils::query_extent(rs,extent);
            return true;
        }
        return false;
    }

};

#endif //SQLITE_TYPES_HPP
