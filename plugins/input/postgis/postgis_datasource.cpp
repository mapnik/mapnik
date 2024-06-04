/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#include "connection_manager.hpp"
#include "postgis_datasource.hpp"
#include "postgis_featureset.hpp"
#include "asyncresultset.hpp"

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/global.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/sql_utils.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/timer.hpp>
#include <mapnik/value/types.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <cfloat> // FLT_MAX
#include <memory>
#include <string>
#include <algorithm>
#include <set>
#include <sstream>

DATASOURCE_PLUGIN_IMPL(postgis_datasource_plugin, postgis_datasource);
DATASOURCE_PLUGIN_EXPORT(postgis_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_AFTER_LOAD(postgis_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_BEFORE_UNLOAD(postgis_datasource_plugin);

const std::string postgis_datasource::GEOMETRY_COLUMNS = "geometry_columns";
const std::string postgis_datasource::SPATIAL_REF_SYS = "spatial_ref_system";

using mapnik::attribute_descriptor;
using mapnik::sql_utils::identifier;
using mapnik::sql_utils::literal;
using std::shared_ptr;

postgis_datasource::postgis_datasource(parameters const& params)
    : datasource(params)
    , table_(*params.get<std::string>("table", ""))
    , geometry_table_(*params.get<std::string>("geometry_table", ""))
    , geometry_field_(*params.get<std::string>("geometry_field", ""))
    , key_field_(*params.get<std::string>("key_field", ""))
    , cursor_fetch_size_(*params.get<mapnik::value_integer>("cursor_size", 0))
    , row_limit_(*params.get<mapnik::value_integer>("row_limit", 0))
    , type_(datasource::Vector)
    , srid_(*params.get<mapnik::value_integer>("srid", 0))
    , extent_initialized_(false)
    , simplify_geometries_(false)
    , desc_(postgis_datasource::name(), "utf-8")
    , creator_(params)
    , pool_max_size_(*params_.get<mapnik::value_integer>("max_size", 10))
    , persist_connection_(*params.get<mapnik::boolean_type>("persist_connection", true))
    , extent_from_subquery_(*params.get<mapnik::boolean_type>("extent_from_subquery", false))
    , max_async_connections_(*params_.get<mapnik::value_integer>("max_async_connection", 1))
    , asynchronous_request_(false)
    , twkb_encoding_(false)
    , twkb_rounding_adjustment_(*params_.get<mapnik::value_double>("twkb_rounding_adjustment", 0.0))
    , simplify_snap_ratio_(*params_.get<mapnik::value_double>("simplify_snap_ratio", 1.0 / 40.0))
    ,
    // 1/20 of pixel seems to be a good compromise to avoid
    // drop of collapsed polygons.
    // See https://github.com/mapnik/mapnik/issues/1639
    // See http://trac.osgeo.org/postgis/ticket/2093
    simplify_dp_ratio_(*params_.get<mapnik::value_double>("simplify_dp_ratio", 1.0 / 20.0))
    , simplify_dp_preserve_(false)
    , simplify_clip_resolution_(*params_.get<mapnik::value_double>("simplify_clip_resolution", 0.0))
    , re_tokens_("!(@?\\w+)!")
    , // matches  !mapnik_var!  or  !@user_var!
    // params below are for testing purposes only and may be removed at any time
    intersect_min_scale_(*params.get<mapnik::value_integer>("intersect_min_scale", 0))
    , intersect_max_scale_(*params.get<mapnik::value_integer>("intersect_max_scale", 0))
    , key_field_as_attribute_(*params.get<mapnik::boolean_type>("key_field_as_attribute", true))
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "postgis_datasource::init");
#endif
    if (table_.empty())
    {
        throw mapnik::datasource_exception("Postgis Plugin: missing <table> parameter");
    }

    const auto ext = params.get<std::string>("extent");
    if (ext.has_value() && !ext->empty())
    {
        extent_initialized_ = extent_.from_string(*ext);
    }

    // NOTE: In multithread environment, pool_max_size_ should be
    // max_async_connections_ * num_threads
    if (max_async_connections_ > 1)
    {
        if (max_async_connections_ > pool_max_size_)
        {
            std::ostringstream err;
            err << "PostGIS Plugin: Error: 'max_async_connections (" << max_async_connections_
                << ") must be <= max_size(" << pool_max_size_ << ")";
            throw mapnik::datasource_exception(err.str());
        }
        asynchronous_request_ = true;
    }

    const auto initial_size = params.get<mapnik::value_integer>("initial_size", 1);
    const auto autodetect_key_field = params.get<mapnik::boolean_type>("autodetect_key_field", false);
    const auto estimate_extent = params.get<mapnik::boolean_type>("estimate_extent", false);
    estimate_extent_ = estimate_extent && *estimate_extent;
    const auto simplify_opt = params.get<mapnik::boolean_type>("simplify_geometries", false);
    simplify_geometries_ = simplify_opt && *simplify_opt;

    const auto twkb_opt = params.get<mapnik::boolean_type>("twkb_encoding", false);
    twkb_encoding_ = twkb_opt && *twkb_opt;

    const auto simplify_preserve_opt = params.get<mapnik::boolean_type>("simplify_dp_preserve", false);
    simplify_dp_preserve_ = simplify_preserve_opt && *simplify_preserve_opt;

    ConnectionManager::instance().registerPool(creator_, *initial_size, pool_max_size_);
    CnxPool_ptr pool = ConnectionManager::instance().getPool(creator_.id());
    if (pool)
    {
        shared_ptr<Connection> conn = pool->borrowObject();
        if (!conn)
            return;

        if (conn->isOK())
        {
            desc_.set_encoding(conn->client_encoding());

            mapnik::sql_utils::table_from_sql(geometry_table_.empty() ? table_ : geometry_table_,
                                              parsed_schema_,
                                              parsed_table_);

            // NOTE: parsed_table_ now should ideally be a table name, but
            // there are known edge cases where this will break down and
            // it may even be empty: https://github.com/mapnik/mapnik/issues/2718

            // If we do not know both the geometry_field and the srid
            // then first attempt to fetch the geometry name from a geometry_columns entry.
            // This will return no records if we are querying a bogus table returned
            // from the simplistic table parsing in table_from_sql() or if
            // the table parameter references a table, view, or subselect not
            // registered in the geometry columns.
            geometryColumn_ = mapnik::sql_utils::unquote_copy('"', geometry_field_);
            if (!parsed_table_.empty() && (geometryColumn_.empty() || srid_ == 0))
            {
#ifdef MAPNIK_STATS
                mapnik::progress_timer __stats2__(std::clog, "postgis_datasource::init(get_srid_and_geometry_column)");
#endif
                std::ostringstream s;

                try
                {
                    s << "SELECT f_geometry_column, srid FROM " << GEOMETRY_COLUMNS
                      << " WHERE f_table_name=" << literal(parsed_table_);
                    if (!parsed_schema_.empty())
                    {
                        s << " AND f_table_schema=" << literal(parsed_schema_);
                    }
                    if (!geometryColumn_.empty())
                    {
                        s << " AND f_geometry_column=" << literal(geometryColumn_);
                    }
                    shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
                    if (rs->next())
                    {
                        geometryColumn_ = rs->getValue("f_geometry_column");
                        // only accept srid from geometry_tables if
                        // user has not provided as option
                        if (srid_ == 0)
                        {
                            const char* srid_c = rs->getValue("srid");
                            if (srid_c != nullptr)
                            {
                                int result = 0;
                                const char* end = srid_c + std::strlen(srid_c);
                                if (mapnik::util::string2int(srid_c, end, result))
                                {
                                    srid_ = result;
                                }
                            }
                        }
                    }
                    rs->close();
                }
                catch (mapnik::datasource_exception const& ex)
                {
                    // let this pass on query error and use the fallback below
                    MAPNIK_LOG_WARN(postgis) << "postgis_datasource: metadata query failed: " << ex.what();
                }
            }

            // If we still do not know the srid then we can try to fetch
            // it from the 'parsed_table_' parameter, which should work even if it is
            // a subselect as long as we know the geometry_field to query
            if (!geometryColumn_.empty() && srid_ <= 0)
            {
                std::ostringstream s;

                s << "SELECT ST_SRID(" << identifier(geometryColumn_) << ") AS srid FROM ";
                if (!parsed_table_.empty())
                {
                    append_geometry_table(s);
                }
                else
                {
                    s << populate_tokens(table_);
                }
                s << " WHERE " << identifier(geometryColumn_) << " IS NOT NULL LIMIT 1";

                shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
                if (rs->next())
                {
                    const char* srid_c = rs->getValue("srid");
                    if (srid_c != nullptr)
                    {
                        int result = 0;
                        const char* end = srid_c + std::strlen(srid_c);
                        if (mapnik::util::string2int(srid_c, end, result))
                        {
                            srid_ = result;
                        }
                    }
                }
                rs->close();
            }

            // detect primary key
            if (*autodetect_key_field && key_field_.empty())
            {
#ifdef MAPNIK_STATS
                mapnik::progress_timer __stats2__(std::clog, "postgis_datasource::bind(get_primary_key)");
#endif

                std::ostringstream s;
                s << "SELECT a.attname, a.attnum, t.typname, t.typname in ('int2','int4','int8') "
                     "AS is_int FROM pg_class c, pg_attribute a, pg_type t, pg_namespace n, pg_index i "
                     "WHERE a.attnum > 0 AND a.attrelid = c.oid "
                     "AND a.atttypid = t.oid AND c.relnamespace = n.oid "
                     "AND c.oid = i.indrelid AND i.indisprimary = 't' "
                     "AND t.typname !~ '^geom' AND c.relname = "
                  << literal(parsed_table_)
                  << " "
                  //"AND a.attnum = ANY (i.indkey) " // postgres >= 8.1
                  << "AND (i.indkey[0]=a.attnum OR i.indkey[1]=a.attnum OR i.indkey[2]=a.attnum "
                     "OR i.indkey[3]=a.attnum OR i.indkey[4]=a.attnum OR i.indkey[5]=a.attnum "
                     "OR i.indkey[6]=a.attnum OR i.indkey[7]=a.attnum OR i.indkey[8]=a.attnum "
                     "OR i.indkey[9]=a.attnum) ";
                if (!parsed_schema_.empty())
                {
                    s << "AND n.nspname=" << literal(parsed_schema_) << ' ';
                }
                s << "ORDER BY a.attnum";

                shared_ptr<ResultSet> rs_key = conn->executeQuery(s.str());
                if (rs_key->next())
                {
                    unsigned int result_rows = rs_key->size();
                    if (result_rows == 1)
                    {
                        bool is_int = (std::string(rs_key->getValue(3)) == "t");
                        if (is_int)
                        {
                            const char* key_field_string = rs_key->getValue(0);
                            if (key_field_string)
                            {
                                key_field_ = std::string(key_field_string);

                                MAPNIK_LOG_DEBUG(postgis) << "postgis_datasource: auto-detected key field of '"
                                                          << key_field_ << "' on table '" << parsed_table_ << "'";
                            }
                        }
                        else
                        {
                            // throw for cases like a numeric primary key, which is invalid
                            // as it should be floating point (int numerics are useless)
                            std::ostringstream err;
                            err << "PostGIS Plugin: Error: '" << rs_key->getValue(0) << "' on table '" << parsed_table_
                                << "' is not a valid integer primary key field\n";
                            throw mapnik::datasource_exception(err.str());
                        }
                    }
                    else if (result_rows > 1)
                    {
                        std::ostringstream err;
                        err << "PostGIS Plugin: Error: '" << "multi column primary key detected but is not supported";
                        throw mapnik::datasource_exception(err.str());
                    }
                }
                rs_key->close();
            }

            // if a globally unique key field/primary key is required
            // but still not known at this point, then throw
            if (*autodetect_key_field && key_field_.empty())
            {
                throw mapnik::datasource_exception("PostGIS Plugin: Error: primary key required"
                                                   " but could not be detected for table '" +
                                                   parsed_table_ +
                                                   "', please supply 'key_field'"
                                                   " option to specify field to use for primary key");
            }

            if (srid_ == 0)
            {
                srid_ = -1;

                MAPNIK_LOG_DEBUG(postgis) << "postgis_datasource: Table " << table_ << " is using SRID=" << srid_;
            }

            // At this point the geometry_field may still not be known
            // but we'll catch that where more useful...
            MAPNIK_LOG_DEBUG(postgis) << "postgis_datasource: Using SRID=" << srid_;
            MAPNIK_LOG_DEBUG(postgis) << "postgis_datasource: Using geometry_column=" << geometryColumn_;

            // collect attribute desc
#ifdef MAPNIK_STATS
            mapnik::progress_timer __stats2__(std::clog, "postgis_datasource::bind(get_column_description)");
#endif

            std::ostringstream s;
            s << "SELECT * FROM " << populate_tokens(table_) << " LIMIT 0";

            shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
            int count = rs->getNumFields();
            bool found_key_field = false;
            for (int i = 0; i < count; ++i)
            {
                std::string fld_name = rs->getFieldName(i);
                int type_oid = rs->getTypeOID(i);

                // validate type of key_field
                if (!found_key_field && !key_field_.empty() && fld_name == key_field_)
                {
                    if (type_oid == 20 || type_oid == 21 || type_oid == 23)
                    {
                        found_key_field = true;
                        if (key_field_as_attribute_)
                        {
                            desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Integer));
                        }
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
                            error_s << rs_oid->getValue("typname") << "' (oid:" << rs_oid->getValue("oid") << ")";
                        }
                        else
                        {
                            error_s << "oid:" << type_oid << "'";
                        }

                        rs_oid->close();
                        error_s << " for key_field '" << fld_name << "' - " << "must be an integer primary key";

                        rs->close();
                        throw mapnik::datasource_exception(error_s.str());
                    }
                }
                else
                {
                    switch (type_oid)
                    {
                        case 16: // bool
                            desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Boolean));
                            break;
                        case 20: // int8
                        case 21: // int2
                        case 23: // int4
                            desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Integer));
                            break;
                        case 700:  // float4
                        case 701:  // float8
                        case 1700: // numeric
                            desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::Double));
                            break;
                        case 1042: // bpchar
                        case 1043: // varchar
                        case 25:   // text
                        case 705:  // literal
                            desc_.add_descriptor(attribute_descriptor(fld_name, mapnik::String));
                            break;
                        default: // should not get here
#ifdef MAPNIK_LOG
                            s.str("");
                            s << "SELECT oid, typname FROM pg_type WHERE oid = " << type_oid;

                            shared_ptr<ResultSet> rs_oid = conn->executeQuery(s.str());
                            if (rs_oid->next())
                            {
                                std::string typname(rs_oid->getValue("typname"));
                                if (typname != "geometry")
                                {
                                    MAPNIK_LOG_WARN(postgis) << "postgis_datasource: Unknown type=" << typname
                                                             << " (oid:" << rs_oid->getValue("oid") << ")";
                                }
                            }
                            else
                            {
                                MAPNIK_LOG_WARN(postgis) << "postgis_datasource: Unknown type_oid=" << type_oid;
                            }
                            rs_oid->close();
#endif
                            break;
                    }
                }
            }

            rs->close();
        }

        // Close explicitly the connection so we can 'fork()' without sharing open connections
        conn->close();

        // Finally, add unique metadata to layer descriptor
        mapnik::parameters& extra_params = desc_.get_extra_parameters();
        // explicitly make copies of values due to https://github.com/mapnik/mapnik/issues/2651
        extra_params["srid"] = mapnik::value_integer(srid_);
        if (!key_field_.empty())
        {
            extra_params["key_field"] = key_field_;
        }
    }
}

postgis_datasource::~postgis_datasource()
{
    if (!persist_connection_)
    {
        CnxPool_ptr pool = ConnectionManager::instance().getPool(creator_.id());
        if (pool)
        {
            try
            {
                shared_ptr<Connection> conn = pool->borrowObject();
                if (conn)
                {
                    conn->close();
                }
            }
            catch (mapnik::datasource_exception const& ex)
            {
                // happens when borrowObject tries to
                // create a new connection and fails.
                // In turn, new connection would be needed
                // when our broke and was thus no good to
                // be borrowed
                // See https://github.com/mapnik/mapnik/issues/2191
            }
        }
    }
}

const char* postgis_datasource::name()
{
    return "postgis";
}

mapnik::datasource::datasource_t postgis_datasource::type() const
{
    return type_;
}

layer_descriptor postgis_datasource::get_descriptor() const
{
    return desc_;
}

std::string postgis_datasource::sql_bbox(box2d<double> const& env) const
{
    std::ostringstream b;
    b.precision(16);
    b << "ST_MakeEnvelope(";
    b << env.minx() << "," << env.miny() << ",";
    b << env.maxx() << "," << env.maxy() << ",";
    b << std::max(srid_, 0) << ")";
    return b.str();
}

std::string postgis_datasource::populate_tokens(std::string const& sql) const
{
    return populate_tokens(sql,
                           FLT_MAX,
                           box2d<double>(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX),
                           0,
                           0,
                           mapnik::attributes{},
                           false);
}

std::string postgis_datasource::populate_tokens(std::string const& sql,
                                                double scale_denom,
                                                box2d<double> const& env,
                                                double pixel_width,
                                                double pixel_height,
                                                mapnik::attributes const& vars,
                                                bool intersect) const
{
    std::ostringstream populated_sql;
    std::cmatch m;
    char const* start = sql.data();
    char const* end = start + sql.size();

    populated_sql.precision(16);
    populated_sql << std::showpoint;

    while (std::regex_search(start, end, m, re_tokens_))
    {
        populated_sql.write(start, m[0].first - start);
        start = m[0].second;

        auto m1 = boost::make_iterator_range(m[1].first, m[1].second);
        if (m1.front() == '@')
        {
            std::string var_name(m1.begin() + 1, m1.end());
            auto itr = vars.find(var_name);
            if (itr != vars.end())
            {
                auto var_value = itr->second.to_string();
                populated_sql << literal(var_value);
            }
            else
            {
                populated_sql << "NULL"; // undefined @variable
            }
        }
        else if (boost::algorithm::equals(m1, "bbox"))
        {
            populated_sql << sql_bbox(env);
            intersect = false;
        }
        else if (boost::algorithm::equals(m1, "pixel_height"))
        {
            populated_sql << pixel_height;
        }
        else if (boost::algorithm::equals(m1, "pixel_width"))
        {
            populated_sql << pixel_width;
        }
        else if (boost::algorithm::equals(m1, "scale_denominator"))
        {
            populated_sql << scale_denom;
        }
        else
        {
            populated_sql << "NULL"; // unrecognized !token!
        }
    }

    populated_sql.write(start, end - start);

    if (intersect)
    {
        if (intersect_min_scale_ > 0 && (scale_denom <= intersect_min_scale_))
        {
            populated_sql << " WHERE ST_Intersects(" << identifier(geometryColumn_) << ", " << sql_bbox(env) << ")";
        }
        else if (intersect_max_scale_ > 0 && (scale_denom >= intersect_max_scale_))
        {
            // do no bbox restriction
        }
        else
        {
            populated_sql << " WHERE " << identifier(geometryColumn_) << " && " << sql_bbox(env);
        }
    }

    return populated_sql.str();
}

void postgis_datasource::append_geometry_table(std::ostream& os) const
{
    if (!geometry_table_.empty())
    {
        os << geometry_table_; // assume datasource parameter is valid SQL
    }
    else if (!parsed_schema_.empty())
    {
        os << identifier(parsed_schema_) << '.' << identifier(parsed_table_);
    }
    else if (!parsed_table_.empty())
    {
        os << identifier(parsed_table_);
    }
    else
    {
        os << table_; // assume datasource parameter is valid SQL
    }
}

std::shared_ptr<IResultSet> postgis_datasource::get_resultset(std::shared_ptr<Connection>& conn,
                                                              std::string const& sql,
                                                              CnxPool_ptr const& pool,
                                                              processor_context_ptr ctx) const
{
    if (!ctx)
    {
        // ! asynchronous_request_
        if (cursor_fetch_size_ > 0)
        {
            // cursor
            std::ostringstream csql;
            std::string cursor_name = conn->new_cursor_name();

            csql << "DECLARE " << cursor_name << " BINARY INSENSITIVE NO SCROLL CURSOR WITH HOLD FOR " << sql
                 << " FOR READ ONLY";

            if (!conn->execute(csql.str()))
            {
                // TODO - better error
                throw mapnik::datasource_exception("Postgis Plugin: error creating cursor for data select.");
            }

            return std::make_shared<CursorResultSet>(conn, cursor_name, cursor_fetch_size_);
        }
        else
        {
            // no cursor
            return conn->executeQuery(sql, 1);
        }
    }
    else
    { // asynchronous requests

        std::shared_ptr<postgis_processor_context> pgis_ctxt = std::static_pointer_cast<postgis_processor_context>(ctx);
        if (conn)
        {
            // lauch async req & create asyncresult with conn
            conn->executeAsyncQuery(sql, 1);
            return std::make_shared<AsyncResultSet>(pgis_ctxt, pool, conn, sql);
        }
        else
        {
            // create asyncresult  with  null connection
            std::shared_ptr<AsyncResultSet> res = std::make_shared<AsyncResultSet>(pgis_ctxt, pool, conn, sql);
            pgis_ctxt->add_request(res);
            return res;
        }
    }
}

processor_context_ptr postgis_datasource::get_context(feature_style_context_map& ctx) const
{
    if (!asynchronous_request_)
    {
        return processor_context_ptr();
    }

    std::string ds_name(name());
    feature_style_context_map::const_iterator itr = ctx.find(ds_name);
    if (itr != ctx.end())
    {
        return itr->second;
    }
    else
    {
        return ctx.emplace(ds_name, std::make_shared<postgis_processor_context>()).first->second;
    }
}

featureset_ptr postgis_datasource::features(query const& q) const
{
    // if the driver is in asynchronous mode, return the appropriate fetaures
    if (asynchronous_request_)
    {
        return features_with_context(q, std::make_shared<postgis_processor_context>());
    }
    else
    {
        return features_with_context(q, processor_context_ptr());
    }
}

featureset_ptr postgis_datasource::features_with_context(query const& q, processor_context_ptr proc_ctx) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "postgis_datasource::features_with_context");
#endif

    box2d<double> const& box = q.get_bbox();
    double scale_denom = q.scale_denominator();

    CnxPool_ptr pool = ConnectionManager::instance().getPool(creator_.id());

    if (pool)
    {
        shared_ptr<Connection> conn;

        if (asynchronous_request_)
        {
            // limit use to num_async_request_ => if reached don't borrow the last connexion object
            std::shared_ptr<postgis_processor_context> pgis_ctxt =
              std::static_pointer_cast<postgis_processor_context>(proc_ctx);
            if (pgis_ctxt->num_async_requests_ < max_async_connections_)
            {
                conn = pool->borrowObject();
                pgis_ctxt->num_async_requests_++;
            }
        }
        else
        {
            // Always get a connection in synchronous mode
            conn = pool->borrowObject();
            if (!conn)
            {
                throw mapnik::datasource_exception("Postgis Plugin: Null connection");
            }
        }

        if (geometryColumn_.empty())
        {
            std::ostringstream s_error;
            s_error << "PostGIS: geometry name lookup failed for table '";
            append_geometry_table(s_error);
            s_error << "'. Please manually provide the 'geometry_field' parameter or add an entry "
                    << "in the geometry_columns for '";
            append_geometry_table(s_error);
            s_error << "'.";

            throw mapnik::datasource_exception(s_error.str());
        }

        std::ostringstream s;

        const double px_gw = 1.0 / std::get<0>(q.resolution());
        const double px_gh = 1.0 / std::get<1>(q.resolution());
        const double px_sz = std::min(px_gw, px_gh);

        if (twkb_encoding_)
        {
            // This will only work against PostGIS 2.2, or a back-patched version
            // that has (a) a ST_Simplify with a "preserve collapsed" flag and
            // (b) a ST_RemoveRepeatedPoints with a tolerance parameter and
            // (c) a ST_AsTWKB implementation

            // What number of decimals of rounding does the pixel size imply?
            const int twkb_rounding = -1 * std::lround(log10(px_sz) + twkb_rounding_adjustment_) + 1;
            // And what's that in map units?
            const double twkb_tolerance = pow(10.0, -1.0 * twkb_rounding);

            s << "SELECT ST_AsTWKB(";
            s << "ST_Simplify(";
            s << "ST_RemoveRepeatedPoints(";

            if (simplify_clip_resolution_ > 0.0 && simplify_clip_resolution_ > px_sz)
            {
                s << "ST_ClipByBox2D(";
            }
            s << identifier(geometryColumn_);

            // ! ST_ClipByBox2D()
            if (simplify_clip_resolution_ > 0.0 && simplify_clip_resolution_ > px_sz)
            {
                s << "," << sql_bbox(box) << ")";
            }

            // ! ST_RemoveRepeatedPoints()
            s << "," << twkb_tolerance << ")";
            // ! ST_Simplify(), with parameter to keep collapsed geometries
            s << "," << twkb_tolerance << ",true)";
            // ! ST_TWKB()
            s << "," << twkb_rounding << ") AS geom";
        }
        else
        {
            s << "SELECT ST_AsBinary(";
            if (simplify_geometries_)
            {
                s << "ST_Simplify(";
            }
            if (simplify_clip_resolution_ > 0.0 && simplify_clip_resolution_ > px_sz)
            {
                s << "ST_ClipByBox2D(";
            }
            if (simplify_geometries_ && simplify_snap_ratio_ > 0.0)
            {
                s << "ST_SnapToGrid(";
            }

            // Geometry column!
            s << identifier(geometryColumn_);

            // ! ST_SnapToGrid()
            if (simplify_geometries_ && simplify_snap_ratio_ > 0.0)
            {
                const double tolerance = px_sz * simplify_snap_ratio_;
                s << "," << tolerance << ")";
            }

            // ! ST_ClipByBox2D()
            if (simplify_clip_resolution_ > 0.0 && simplify_clip_resolution_ > px_sz)
            {
                s << "," << sql_bbox(box) << ")";
            }

            // ! ST_Simplify()
            if (simplify_geometries_)
            {
                const double tolerance = px_sz * simplify_dp_ratio_;
                s << ", " << tolerance;
                // Add parameter to ST_Simplify to keep collapsed geometries
                if (simplify_dp_preserve_)
                {
                    s << ", true";
                }
                s << ")";
            }

            // ! ST_AsBinary()
            s << ") AS geom";
        }
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
        std::set<std::string> const& props = q.property_names();
        std::set<std::string>::const_iterator pos = props.begin();
        std::set<std::string>::const_iterator end = props.end();

        if (!key_field_.empty())
        {
            s << ',' << identifier(key_field_);
            if (key_field_as_attribute_)
            {
                ctx->push(key_field_);
            }

            for (; pos != end; ++pos)
            {
                if (*pos != key_field_)
                {
                    s << ',' << identifier(*pos);
                    ctx->push(*pos);
                }
            }
        }
        else
        {
            for (; pos != end; ++pos)
            {
                s << ',' << identifier(*pos);
                ctx->push(*pos);
            }
        }

        std::string table_with_bbox = populate_tokens(table_, scale_denom, box, px_gw, px_gh, q.variables());

        s << " FROM " << table_with_bbox;

        if (row_limit_ > 0)
        {
            s << " LIMIT " << row_limit_;
        }

        std::shared_ptr<IResultSet> rs = get_resultset(conn, s.str(), pool, proc_ctx);
        return std::make_shared<postgis_featureset>(rs,
                                                    ctx,
                                                    desc_.get_encoding(),
                                                    !key_field_.empty(),
                                                    key_field_as_attribute_,
                                                    twkb_encoding_);
    }

    return mapnik::make_invalid_featureset();
}

featureset_ptr postgis_datasource::features_at_point(coord2d const& pt, double tol) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "postgis_datasource::features_at_point");
#endif
    CnxPool_ptr pool = ConnectionManager::instance().getPool(creator_.id());
    if (pool)
    {
        shared_ptr<Connection> conn = pool->borrowObject();
        if (!conn)
            return mapnik::make_invalid_featureset();

        if (conn->isOK())
        {
            if (geometryColumn_.empty())
            {
                std::ostringstream s_error;
                s_error << "PostGIS: geometry name lookup failed for table '";
                append_geometry_table(s_error);
                s_error << "'. Please manually provide the 'geometry_field' parameter or add an entry "
                        << "in the geometry_columns for '";
                append_geometry_table(s_error);
                s_error << "'.";

                throw mapnik::datasource_exception(s_error.str());
            }

            std::ostringstream s;
            s << "SELECT ST_AsBinary(" << identifier(geometryColumn_) << ") AS geom";

            mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
            auto const& desc = desc_.get_descriptors();

            if (!key_field_.empty())
            {
                s << ',' << identifier(key_field_);
                if (key_field_as_attribute_)
                {
                    ctx->push(key_field_);
                }
                for (auto const& attr_info : desc)
                {
                    std::string const& name = attr_info.get_name();
                    if (name != key_field_)
                    {
                        s << ',' << identifier(name);
                        ctx->push(name);
                    }
                }
            }
            else
            {
                for (auto const& attr_info : desc)
                {
                    std::string const& name = attr_info.get_name();
                    s << ',' << identifier(name);
                    ctx->push(name);
                }
            }

            box2d<double> box(pt.x - tol, pt.y - tol, pt.x + tol, pt.y + tol);
            std::string table_with_bbox = populate_tokens(table_, FLT_MAX, box, 0, 0, mapnik::attributes{});

            s << " FROM " << table_with_bbox;

            if (row_limit_ > 0)
            {
                s << " LIMIT " << row_limit_;
            }

            std::shared_ptr<IResultSet> rs = get_resultset(conn, s.str(), pool);
            return std::make_shared<postgis_featureset>(rs,
                                                        ctx,
                                                        desc_.get_encoding(),
                                                        !key_field_.empty(),
                                                        key_field_as_attribute_,
                                                        twkb_encoding_);
        }
    }

    return mapnik::make_invalid_featureset();
}

box2d<double> postgis_datasource::envelope() const
{
    if (extent_initialized_)
    {
        return extent_;
    }

    CnxPool_ptr pool = ConnectionManager::instance().getPool(creator_.id());
    if (pool)
    {
        shared_ptr<Connection> conn = pool->borrowObject();
        if (!conn)
            return extent_;
        if (conn->isOK())
        {
            std::ostringstream s;

            if (geometryColumn_.empty())
            {
                std::ostringstream s_error;
                s_error << "PostGIS: unable to query the layer extent of table '";
                append_geometry_table(s_error);
                s_error << "' because we cannot determine the geometry field name."
                        << "\nPlease provide either an 'extent' parameter to skip this query, "
                        << "a 'geometry_field' and/or 'geometry_table' parameter, or add a "
                        << "record to the 'geometry_columns' for your table.";

                throw mapnik::datasource_exception("Postgis Plugin: " + s_error.str());
            }

            if (estimate_extent_)
            {
                s << "SELECT ST_XMin(ext),ST_YMin(ext),ST_XMax(ext),ST_YMax(ext)"
                  << " FROM (SELECT ST_EstimatedExtent(";
                if (!parsed_schema_.empty())
                {
                    s << literal(parsed_schema_) << ',';
                }

                s << literal(parsed_table_) << ',' << literal(geometryColumn_) << ") as ext) as tmp";
            }
            else
            {
                s << "SELECT ST_XMin(ext),ST_YMin(ext),ST_XMax(ext),ST_YMax(ext)" << " FROM (SELECT ST_Extent("
                  << identifier(geometryColumn_) << ") as ext from ";

                if (extent_from_subquery_)
                {
                    // if a subselect limits records then calculating the extent upon the
                    // subquery will be faster and the bounds will be more accurate
                    s << populate_tokens(table_) << ") as tmp";
                }
                else
                {
                    // but if the subquery does not limit records then querying the
                    // actual table will be faster as indexes can be used
                    append_geometry_table(s);
                    s << ") as tmp";
                }
            }

            shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
            if (rs->next() && !rs->isNull(0))
            {
                double lox, loy, hix, hiy;
                if (mapnik::util::string2double(rs->getValue(0), lox) &&
                    mapnik::util::string2double(rs->getValue(1), loy) &&
                    mapnik::util::string2double(rs->getValue(2), hix) &&
                    mapnik::util::string2double(rs->getValue(3), hiy))
                {
                    extent_.init(lox, loy, hix, hiy);
                    extent_initialized_ = true;
                }
                else
                {
                    MAPNIK_LOG_DEBUG(postgis)
                      << "postgis_datasource: Could not determine extent from query: " << s.str();
                }
            }
            rs->close();
        }
    }

    return extent_;
}

std::optional<mapnik::datasource_geometry_t> postgis_datasource::get_geometry_type() const
{
    std::optional<mapnik::datasource_geometry_t> result;

    CnxPool_ptr pool = ConnectionManager::instance().getPool(creator_.id());
    if (pool)
    {
        shared_ptr<Connection> conn = pool->borrowObject();
        if (!conn)
            return result;
        if (conn->isOK())
        {
            std::ostringstream s;
            std::string g_type;
            try
            {
                s << "SELECT lower(type) as type FROM " << GEOMETRY_COLUMNS
                  << " WHERE f_table_name=" << literal(parsed_table_);
                if (!parsed_schema_.empty())
                {
                    s << " AND f_table_schema=" << literal(parsed_schema_);
                }
                if (!geometryColumn_.empty())
                {
                    s << " AND f_geometry_column=" << literal(geometryColumn_);
                }
                shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
                if (rs->next())
                {
                    g_type = rs->getValue("type");
                    if (boost::algorithm::contains(g_type, "line"))
                    {
                        result = mapnik::datasource_geometry_t::LineString;
                        return result;
                    }
                    else if (boost::algorithm::contains(g_type, "point"))
                    {
                        result = mapnik::datasource_geometry_t::Point;
                        return result;
                    }
                    else if (boost::algorithm::contains(g_type, "polygon"))
                    {
                        result = mapnik::datasource_geometry_t::Polygon;
                        return result;
                    }
                    else // geometry
                    {
                        g_type = "";
                    }
                }
            }
            catch (mapnik::datasource_exception const& ex)
            {
                // let this pass on query error and use the fallback below
                MAPNIK_LOG_WARN(postgis) << "postgis_datasource: metadata query failed: " << ex.what();
            }

            // fallback to querying first several features
            if (g_type.empty() && !geometryColumn_.empty())
            {
                s.str("");

                std::string prev_type("");

                s << "SELECT ST_GeometryType(" << identifier(geometryColumn_) << ") AS geom" << " FROM "
                  << populate_tokens(table_);

                if (row_limit_ > 0 && row_limit_ < 5)
                {
                    s << " LIMIT " << row_limit_;
                }
                else
                {
                    s << " LIMIT 5";
                }

                shared_ptr<ResultSet> rs = conn->executeQuery(s.str());
                while (rs->next() && !rs->isNull(0))
                {
                    const char* data = rs->getValue(0);
                    if (boost::algorithm::icontains(data, "line"))
                    {
                        g_type = "linestring";
                        result = mapnik::datasource_geometry_t::LineString;
                    }
                    else if (boost::algorithm::icontains(data, "point"))
                    {
                        g_type = "point";
                        result = mapnik::datasource_geometry_t::Point;
                    }
                    else if (boost::algorithm::icontains(data, "polygon"))
                    {
                        g_type = "polygon";
                        result = mapnik::datasource_geometry_t::Polygon;
                    }
                    else // geometry
                    {
                        result = mapnik::datasource_geometry_t::Collection;
                        return result;
                    }
                    if (!prev_type.empty() && g_type != prev_type)
                    {
                        result = mapnik::datasource_geometry_t::Collection;
                        return result;
                    }
                    prev_type = g_type;
                }
            }
        }
    }

    return result;
}
