/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_SQLITE_UTILS_HPP
#define MAPNIK_SQLITE_UTILS_HPP

// stl
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/sql_utils.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/geometry/is_empty.hpp>
#include <mapnik/geometry/envelope.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
MAPNIK_DISABLE_WARNING_POP

// sqlite
extern "C" {
#include <sqlite3.h>
}

#include "sqlite_resultset.hpp"
#include "sqlite_prepared.hpp"
#include "sqlite_connection.hpp"

//==============================================================================

class sqlite_utils
{
  public:

    static bool needs_quoting(std::string const& name)
    {
        if (name.size() > 0)
        {
            const char first = name[0];
            if (is_quote_char(first))
            {
                return false;
            }
            if ((first >= '0' && first <= '9') || (name.find("-") != std::string::npos))
            {
                return true;
            }
        }
        return false;
    }

    static bool is_quote_char(const char z)
    {
        if (z == '"' || z == '\'' || z == '[' || z == '`')
        {
            return true;
        }
        return false;
    }

    static void dequote(std::string& z) { boost::algorithm::trim_if(z, boost::algorithm::is_any_of("[]'\"`")); }

    static std::string index_for_table(std::string const& table, std::string const& field)
    {
        std::string table_trimmed = table;
        dequote(table_trimmed);
        return "\"idx_" + table_trimmed + "_" + field + "\"";
    }

    static std::string index_for_db(std::string const& file)
    {
        // std::size_t idx = file.find_last_of(".");
        // if(idx != std::string::npos) {
        //     return file.substr(0,idx) + ".index" + file.substr(idx);
        // }
        // else
        //{
        return file + ".index";
        //}
    }

    static bool apply_spatial_filter(std::string& query,
                                     mapnik::box2d<double> const& e,
                                     std::string const& table,
                                     std::string const& key_field,
                                     std::string const& index_table,
                                     std::string const& geometry_table,
                                     std::string const& intersects_token)
    {
        std::ostringstream spatial_sql;
        spatial_sql << std::setprecision(16);
        spatial_sql << key_field << " IN (SELECT pkid FROM " << index_table;
        spatial_sql << " WHERE xmax>=" << e.minx() << " AND xmin<=" << e.maxx();
        spatial_sql << " AND ymax>=" << e.miny() << " AND ymin<=" << e.maxy() << ")";
        if (boost::algorithm::ifind_first(query, intersects_token))
        {
            boost::algorithm::ireplace_all(query, intersects_token, spatial_sql.str());
            return true;
        }
        // substitute first WHERE found if not using JOIN
        // (because we can't know the WHERE is on the right table)
        else if (boost::algorithm::ifind_first(query, "WHERE") && !boost::algorithm::ifind_first(query, "JOIN"))
        {
            std::string replace(" WHERE " + spatial_sql.str() + " AND ");
            boost::algorithm::ireplace_first(query, "WHERE", replace);
            return true;
        }
        // fallback to appending spatial filter at end of query
        else if (boost::algorithm::ifind_first(query, geometry_table))
        {
            query = table + " WHERE " + spatial_sql.str();
            return true;
        }
        return false;
    }

    static void get_tables(std::shared_ptr<sqlite_connection> ds, std::vector<std::string>& tables)
    {
        std::ostringstream sql;
        // clang-format off
        // todo handle finding tables from attached db's
        sql << " SELECT name FROM sqlite_master"
            << " WHERE type IN ('table','view')"
            << " AND name NOT LIKE 'sqlite_%'"
            << " AND name NOT LIKE 'idx_%'"
            << " AND name NOT LIKE '%geometry_columns%'"
            << " AND name NOT LIKE '%ref_sys%'"
            << " UNION ALL"
            << " SELECT name FROM sqlite_temp_master"
            << " WHERE type IN ('table','view')"
            << " ORDER BY 1";
        // clang-format on
        sqlite3_stmt* stmt = 0;
        const int rc = sqlite3_prepare_v2(*(*ds), sql.str().c_str(), -1, &stmt, 0);
        if (rc == SQLITE_OK)
        {
            std::shared_ptr<sqlite_resultset> rs = std::make_shared<sqlite_resultset>(stmt);
            while (rs->is_valid() && rs->step_next())
            {
                const int type_oid = rs->column_type(0);
                if (type_oid == SQLITE_TEXT)
                {
                    const char* data = rs->column_text(0);
                    if (data)
                    {
                        tables.push_back(std::string(data));
                    }
                }
            }
        }
    }

    static void query_extent(std::shared_ptr<sqlite_resultset> rs, mapnik::box2d<double>& extent)
    {
        bool first = true;
        while (rs->is_valid() && rs->step_next())
        {
            int size;
            const char* data = static_cast<const char*>(rs->column_blob(0, size));
            if (data)
            {
                mapnik::geometry::geometry<double> geom = mapnik::geometry_utils::from_wkb(data, size, mapnik::wkbAuto);
                if (!mapnik::geometry::is_empty(geom))
                {
                    mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
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
    }

    static bool create_spatial_index(std::string const& index_db,
                                     std::string const& index_table,
                                     std::shared_ptr<sqlite_resultset> rs)
    {
        /* TODO
           - speedups
           - return early/recover from failure
           - allow either in db or in separate
        */

        if (!rs->is_valid())
            return false;

#if SQLITE_VERSION_NUMBER >= 3005000
        int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#else
        int flags;
#endif

        bool existed = mapnik::util::exists(index_db);
        std::shared_ptr<sqlite_connection> ds = std::make_shared<sqlite_connection>(index_db, flags);

        bool one_success = false;
        try
        {
            ds->execute("PRAGMA synchronous=OFF");
            ds->execute("BEGIN IMMEDIATE TRANSACTION");

            /*
              ds->execute("PRAGMA journal_mode=WAL");
              ds->execute("PRAGMA fullfsync=1");
              ds->execute("PRAGMA locking_mode = EXCLUSIVE");
              ds->execute("BEGIN EXCLUSIVE TRANSACTION");
            */

            // first drop the index if it already exists
            std::ostringstream spatial_index_drop_sql;
            spatial_index_drop_sql << "DROP TABLE IF EXISTS " << index_table;
            ds->execute(spatial_index_drop_sql.str());

            // create the spatial index
            std::ostringstream create_idx;
            create_idx << "create virtual table " << index_table << " using rtree(pkid, xmin, xmax, ymin, ymax)";

            // insert for prepared statement
            std::ostringstream insert_idx;
            insert_idx << "insert into " << index_table << " values (?,?,?,?,?)";

            ds->execute(create_idx.str());

            prepared_index_statement ps(ds, insert_idx.str());

            while (rs->is_valid() && rs->step_next())
            {
                int size;
                const char* data = (const char*)rs->column_blob(0, size);
                if (data)
                {
                    mapnik::geometry::geometry<double> geom =
                      mapnik::geometry_utils::from_wkb(data, size, mapnik::wkbAuto);
                    if (!mapnik::geometry::is_empty(geom))
                    {
                        mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
                        if (bbox.valid())
                        {
                            ps.bind(bbox);
                            const int type_oid = rs->column_type(1);
                            if (type_oid != SQLITE_INTEGER)
                            {
                                std::ostringstream error_msg;
                                error_msg << "Sqlite Plugin: invalid type for key field '" << rs->column_name(1)
                                          << "' when creating index '" << index_table << "' type was: " << type_oid
                                          << "";
                                throw mapnik::datasource_exception(error_msg.str());
                            }
                            const sqlite_int64 pkid = rs->column_integer64(1);
                            ps.bind(pkid);
                        }
                        else
                        {
                            std::ostringstream error_msg;
                            error_msg << "SQLite Plugin: encountered invalid bbox at '" << rs->column_name(1)
                                      << "' == " << rs->column_integer64(1);
                            throw mapnik::datasource_exception(error_msg.str());
                        }
                    }
                    ps.step_next();
                    one_success = true;
                }
            }
        }
        catch (mapnik::datasource_exception const& ex)
        {
            if (!existed)
            {
                try
                {
                    mapnik::util::remove(index_db);
                }
                catch (...)
                {};
            }
            throw mapnik::datasource_exception(ex.what());
        }

        if (one_success)
        {
            ds->execute("COMMIT");
            return true;
        }
        else if (!existed)
        {
            try
            {
                mapnik::util::remove(index_db);
            }
            catch (...)
            {};
        }
        return false;
    }

    typedef struct
    {
        sqlite_int64 pkid;
        mapnik::box2d<double> bbox;
    } rtree_type;

    static void build_tree(std::shared_ptr<sqlite_resultset> rs, std::vector<sqlite_utils::rtree_type>& rtree_list)
    {
        while (rs->is_valid() && rs->step_next())
        {
            int size;
            const char* data = static_cast<const char*>(rs->column_blob(0, size));
            if (data)
            {
                mapnik::geometry::geometry<double> geom = mapnik::geometry_utils::from_wkb(data, size, mapnik::wkbAuto);
                if (!mapnik::geometry::is_empty(geom))
                {
                    mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
                    if (bbox.valid())
                    {
                        const int type_oid = rs->column_type(1);
                        if (type_oid != SQLITE_INTEGER)
                        {
                            std::ostringstream error_msg;
                            error_msg << "Sqlite Plugin: invalid type for key field '" << rs->column_name(1)
                                      << "' when creating index " << "type was: " << type_oid << "";
                            throw mapnik::datasource_exception(error_msg.str());
                        }
                        const sqlite_int64 pkid = rs->column_integer64(1);
                        rtree_type entry = rtree_type();
                        entry.pkid = pkid;
                        entry.bbox = bbox;
                        rtree_list.push_back(entry);
                    }
                    else
                    {
                        std::ostringstream error_msg;
                        error_msg << "SQLite Plugin: encountered invalid bbox at '" << rs->column_name(1)
                                  << "' == " << rs->column_integer64(1);
                        throw mapnik::datasource_exception(error_msg.str());
                    }
                }
            }
        }
    }

    static bool create_spatial_index2(std::string const& index_db,
                                      std::string const& index_table,
                                      std::vector<rtree_type> const& rtree_list)
    {
#if SQLITE_VERSION_NUMBER >= 3005000
        int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#else
        int flags;
#endif

        bool existed = mapnik::util::exists(index_db);
        ;

        std::shared_ptr<sqlite_connection> ds = std::make_shared<sqlite_connection>(index_db, flags);

        bool one_success = false;
        try
        {
            ds->execute("PRAGMA synchronous=OFF");
            ds->execute("BEGIN IMMEDIATE TRANSACTION");

            // first drop the index if it already exists
            std::ostringstream spatial_index_drop_sql;
            spatial_index_drop_sql << "DROP TABLE IF EXISTS " << index_table;
            ds->execute(spatial_index_drop_sql.str());

            // create the spatial index
            std::ostringstream create_idx;
            create_idx << "create virtual table " << index_table << " using rtree(pkid, xmin, xmax, ymin, ymax)";

            // insert for prepared statement
            std::ostringstream insert_idx;
            insert_idx << "insert into " << index_table << " values (?,?,?,?,?)";

            ds->execute(create_idx.str());

            prepared_index_statement ps(ds, insert_idx.str());

            std::vector<rtree_type>::const_iterator it = rtree_list.begin();
            std::vector<rtree_type>::const_iterator end = rtree_list.end();
            for (; it != end; ++it)
            {
                ps.bind(it->bbox);
                ps.bind(it->pkid);
                ps.step_next();
                one_success = true;
            }
        }
        catch (mapnik::datasource_exception const& ex)
        {
            if (!existed)
            {
                try
                {
                    mapnik::util::remove(index_db);
                }
                catch (...)
                {};
            }
            throw mapnik::datasource_exception(ex.what());
        }

        if (one_success)
        {
            ds->execute("COMMIT");
            return true;
        }
        else if (!existed)
        {
            try
            {
                mapnik::util::remove(index_db);
            }
            catch (...)
            {};
        }
        return false;
    }

    static bool detect_extent(std::shared_ptr<sqlite_connection> ds,
                              bool has_spatial_index,
                              mapnik::box2d<double>& extent,
                              std::string const& index_table,
                              std::string const& metadata,
                              std::string const& geometry_field,
                              std::string const& geometry_table,
                              std::string const& key_field,
                              std::string const& table)
    {
        if (!metadata.empty())
        {
            std::ostringstream s;
            s << "SELECT xmin, ymin, xmax, ymax FROM " << metadata;
            s << " WHERE LOWER(f_table_name) = LOWER('" << geometry_table << "')";
            MAPNIK_LOG_DEBUG(sqlite) << "sqlite_datasource: executing: '" << s.str() << "'";
            std::shared_ptr<sqlite_resultset> rs(ds->execute_query(s.str()));
            if (rs->is_valid() && rs->step_next())
            {
                double xmin = rs->column_double(0);
                double ymin = rs->column_double(1);
                double xmax = rs->column_double(2);
                double ymax = rs->column_double(3);
                extent.init(xmin, ymin, xmax, ymax);
                return true;
            }
        }
        else if (has_spatial_index)
        {
            std::ostringstream s;
            s << "SELECT MIN(xmin), MIN(ymin), MAX(xmax), MAX(ymax) FROM " << index_table;
            MAPNIK_LOG_DEBUG(sqlite) << "sqlite_datasource: executing: '" << s.str() << "'";
            std::shared_ptr<sqlite_resultset> rs(ds->execute_query(s.str()));
            if (rs->is_valid() && rs->step_next())
            {
                if (!rs->column_isnull(0))
                {
                    double xmin = rs->column_double(0);
                    double ymin = rs->column_double(1);
                    double xmax = rs->column_double(2);
                    double ymax = rs->column_double(3);
                    extent.init(xmin, ymin, xmax, ymax);
                    return true;
                }
            }
        }

        else if (!key_field.empty())
        {
            std::ostringstream s;
            s << "SELECT " << geometry_field << "," << key_field << " FROM (" << table << ")";
            MAPNIK_LOG_DEBUG(sqlite) << "sqlite_datasource: executing: '" << s.str() << "'";
            std::shared_ptr<sqlite_resultset> rs(ds->execute_query(s.str()));
            sqlite_utils::query_extent(rs, extent);
            return true;
        }
        return false;
    }

    static bool has_rtree(std::string const& index_table, std::shared_ptr<sqlite_connection> ds)
    {
        try
        {
            std::ostringstream s;
            s << "SELECT pkid,xmin,xmax,ymin,ymax FROM " << index_table << " LIMIT 1";
            std::shared_ptr<sqlite_resultset> rs = ds->execute_query(s.str());
            if (rs->is_valid() && rs->step_next())
            {
                return true;
            }
        }
        catch (std::exception const& ex)
        {
            MAPNIK_LOG_DEBUG(sqlite) << "has_rtree returned:" << ex.what();
            return false;
        }
        return false;
    }

    static bool detect_types_from_subquery(std::string const& query,
                                           std::string& geometry_field,
                                           mapnik::layer_descriptor& desc,
                                           std::shared_ptr<sqlite_connection> ds)
    {
        bool found = false;
        std::shared_ptr<sqlite_resultset> rs(ds->execute_query(query));
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
                        break;

                    case SQLITE_BLOB:
                        if (geometry_field.empty() && (boost::algorithm::icontains(fld_name, "geom") ||
                                                       boost::algorithm::icontains(fld_name, "point") ||
                                                       boost::algorithm::icontains(fld_name, "linestring") ||
                                                       boost::algorithm::icontains(fld_name, "polygon")))
                        {
                            geometry_field = std::string(fld_name);
                        }
                        break;

                    default:
                        MAPNIK_LOG_DEBUG(sqlite) << "detect_types_from_subquery: unknown type_oid=" << type_oid;
                        break;
                }
            }
        }

        return found;
    }

    static bool table_info(std::string& key_field,
                           bool detected_types,
                           std::string& field,
                           std::string& table,
                           mapnik::layer_descriptor& desc,
                           std::shared_ptr<sqlite_connection> ds)
    {
        // http://www.sqlite.org/pragma.html#pragma_table_info
        // use pragma table_info to detect primary key
        // and to detect types if no subquery is used or
        // if the subquery-based type detection failed
        std::ostringstream s;
        s << "PRAGMA table_info(" << table << ")";
        std::shared_ptr<sqlite_resultset> rs(ds->execute_query(s.str()));
        bool found_table = false;
        bool found_pk = false;
        while (rs->is_valid() && rs->step_next())
        {
            found_table = true;
            const char* fld_name = rs->column_text(1);
            std::string fld_type(rs->column_text(2));
            sqlite_int64 fld_pk = rs->column_integer64(5);
            std::transform(fld_type.begin(), fld_type.end(), fld_type.begin(), ::tolower);
            // TODO - how to handle primary keys on multiple columns ?
            if (key_field.empty() && !found_pk && fld_pk != 0)
            {
                key_field = fld_name;
                found_pk = true;
            }
            if (!detected_types)
            {
                // see 2.1 "Column Affinity" at http://www.sqlite.org/datatype3.html
                // TODO - refactor this somehow ?
                if (field.empty() &&
                    ((boost::algorithm::contains(fld_type, "geom") || boost::algorithm::contains(fld_type, "point") ||
                      boost::algorithm::contains(fld_type, "linestring") ||
                      boost::algorithm::contains(fld_type, "polygon")) ||
                     (boost::algorithm::icontains(fld_name, "geom") || boost::algorithm::icontains(fld_name, "point") ||
                      boost::algorithm::icontains(fld_name, "linestring") ||
                      boost::algorithm::icontains(fld_name, "polygon"))))
                {
                    field = std::string(fld_name);
                }
                else if (boost::algorithm::contains(fld_type, "int"))
                {
                    desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::Integer));
                }
                else if (boost::algorithm::contains(fld_type, "text") || boost::algorithm::contains(fld_type, "char") ||
                         boost::algorithm::contains(fld_type, "clob"))
                {
                    desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::String));
                }
                else if (boost::algorithm::contains(fld_type, "real") || boost::algorithm::contains(fld_type, "floa") ||
                         boost::algorithm::contains(fld_type, "doub"))
                {
                    desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::Double));
                }
                else if (boost::algorithm::contains(fld_type, "blob"))
                {
                    if (!field.empty())
                    {
                        desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::String));
                    }
                }
                else
                {
                    // "Column Affinity" says default to "Numeric" but for now we pass..
                    // desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Double));

                    desc.add_descriptor(mapnik::attribute_descriptor(fld_name, mapnik::String));

#ifdef MAPNIK_LOG
                    // Do not fail when we specify geometry_field in XML file
                    if (field.empty())
                    {
                        MAPNIK_LOG_DEBUG(sqlite)
                          << "Column '" << std::string(fld_name) << "' unhandled due to unknown type: '" << fld_type
                          << "', using String";
                    }
#endif
                }
            }
        }

        return found_table;
    }
};

#endif // MAPNIK_SQLITE_UTILS_HPP
