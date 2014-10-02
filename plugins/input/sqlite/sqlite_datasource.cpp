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
#include <mapnik/debug.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/sql_utils.hpp>
#include <mapnik/util/geometry_to_ds_type.hpp>
#include <mapnik/timer.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/util/fs.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

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

sqlite_datasource::sqlite_datasource(parameters const& params)
    : datasource(params),
      extent_(),
      extent_initialized_(false),
      type_(datasource::Vector),
      table_(*params.get<std::string>("table", "")),
      fields_(*params.get<std::string>("fields", "*")),
      metadata_(*params.get<std::string>("metadata", "")),
      geometry_table_(*params.get<std::string>("geometry_table", "")),
      geometry_field_(*params.get<std::string>("geometry_field", "")),
      index_table_(*params.get<std::string>("index_table", "")),
      key_field_(*params.get<std::string>("key_field", "")),
      row_offset_(*params.get<mapnik::value_integer>("row_offset", 0)),
      row_limit_(*params.get<mapnik::value_integer>("row_limit", 0)),
      intersects_token_("!intersects!"),
      desc_(sqlite_datasource::name(), *params.get<std::string>("encoding", "utf-8")),
      format_(mapnik::wkbAuto)
{
    /* TODO
       - throw if no primary key but spatial index is present?
       - remove auto-indexing
       - if spatialite - leverage more of the metadata for geometry type detection
    */

#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "sqlite_datasource::init");
#endif

    boost::optional<std::string> file = params.get<std::string>("file");
    if (! file) throw datasource_exception("Sqlite Plugin: missing <file> parameter");

    boost::optional<std::string> base = params.get<std::string>("base");
    if (base)
        dataset_name_ = *base + "/" + *file;
    else
        dataset_name_ = *file;

    if ((dataset_name_.compare(":memory:") != 0) && (!mapnik::util::exists(dataset_name_)))
    {
        throw datasource_exception("Sqlite Plugin: " + dataset_name_ + " does not exist");
    }

    use_spatial_index_ = *params.get<mapnik::boolean_type>("use_spatial_index", true);

    // TODO - remove this option once all datasources have an indexing api
    bool auto_index = *params.get<mapnik::boolean_type>("auto_index", true);

    boost::optional<std::string> ext  = params.get<std::string>("extent");
    if (ext) extent_initialized_ = extent_.from_string(*ext);

    boost::optional<std::string> wkb = params.get<std::string>("wkb_format");
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
    boost::optional<std::string> attachdb = params.get<std::string>("attachdb");
    if (attachdb)
    {
        parse_attachdb(*attachdb);
    }

    boost::optional<std::string> initdb = params.get<std::string>("initdb");
    if (initdb)
    {
        init_statements_.push_back(*initdb);
    }

    // now actually create the connection and start executing setup sql
    dataset_ = std::make_shared<sqlite_connection>(dataset_name_);

    boost::optional<mapnik::value_integer> table_by_index = params.get<mapnik::value_integer>("table_by_index");

    int passed_parameters = 0;
    passed_parameters += params.get<std::string>("table") ? 1 : 0;
    passed_parameters += table_by_index ? 1 : 0;

    if (passed_parameters > 1)
    {
        throw datasource_exception("SQLite Plugin: you can only select an by name "
                                   "('table' parameter), by number ('table_by_index' parameter), "
                                   "do not supply 2 or more of them at the same time" );
    }

    if (table_by_index)
    {
        std::vector<std::string> tables;
        sqlite_utils::get_tables(dataset_,tables);
        if (*table_by_index < 0 || *table_by_index >= static_cast<int>(tables.size()))
        {
            std::ostringstream s;
            s << "SQLite Plugin: only "
              << tables.size()
              << " table(s) exist, cannot find table by index '" << *table_by_index << "'";

            throw datasource_exception(s.str());
        }
        table_ = tables[*table_by_index];

    }

    if (table_.empty())
    {
        throw mapnik::datasource_exception("Sqlite Plugin: missing <table> parameter");
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
    else
    {
        // attempt to auto-quote table if needed
        if (sqlite_utils::needs_quoting(table_))
        {
            table_ = std::string("[") + table_ + "]";
            geometry_table_ = table_;
        }
    }

    // Execute init_statements_
    for (std::vector<std::string>::const_iterator iter = init_statements_.begin();
         iter != init_statements_.end(); ++iter)
    {
        MAPNIK_LOG_DEBUG(sqlite) << "sqlite_datasource: Execute init sql=" << *iter;

        dataset_->execute(*iter);
    }

    bool found_types_via_subquery = false;
    if (using_subquery_)
    {
        std::ostringstream s;
        std::string query = populate_tokens(table_);
        s << "SELECT " << fields_ << " FROM (" << query << ") LIMIT 1";
        found_types_via_subquery = sqlite_utils::detect_types_from_subquery(
            s.str(),
            geometry_field_,
            desc_,
            dataset_);
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
        s << "Sqlite Plugin: could not query table '" << geometry_table_ << "'";
        if (using_subquery_)
        {
            s << " from subquery '" << table_ << "'";
        }

        // report get available tables
        std::vector<std::string> tables;
        sqlite_utils::get_tables(dataset_,tables);
        if (tables.size() > 0)
        {
            s << " (available tables for " << dataset_name_ << " are: '" << boost::algorithm::join(tables, ", ") << "')";
        }

        throw datasource_exception(s.str());
    }

    if (geometry_field_.empty())
    {
        std::ostringstream s;
        s << "Sqlite Plugin: unable to detect the column "
          << "containing a valid geometry on table '" << geometry_table_ << "'. "
          << "Please provide a column name by passing the 'geometry_field' option "
          << "or indicate a different spatial table to use by passing the 'geometry_table' option";
        throw datasource_exception(s.str());
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
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats2__(std::clog, "sqlite_datasource::init(use_spatial_index)");
#endif

        bool index_db_attached = false;
        if (mapnik::util::exists(index_db))
        {
            dataset_->execute("attach database '" + index_db + "' as " + index_table_);
            index_db_attached = true;
        }
        has_spatial_index_ = sqlite_utils::has_rtree(index_table_,dataset_);

        if (!has_spatial_index_ && auto_index)
        {
            if (! key_field_.empty())
            {
                std::ostringstream query;
                query << "SELECT "
                      << geometry_field_
                      << "," << key_field_
                      << " FROM ("
                      << geometry_table_ << ")";

#ifdef MAPNIK_STATS
                mapnik::progress_timer __stats2__(std::clog, "sqlite_datasource::init(create_spatial_index)");
#endif

                std::shared_ptr<sqlite_resultset> rs = dataset_->execute_query(query.str());
                if (sqlite_utils::create_spatial_index(index_db,index_table_,rs))
                {
                    //extent_initialized_ = true;
                    has_spatial_index_ = true;
                    if (!index_db_attached && mapnik::util::exists(index_db))
                    {
                        dataset_->execute("attach database '" + index_db + "' as " + index_table_);
                    }
                }
            }
            else
            {
                std::ostringstream s;
                s << "Sqlite Plugin: could not generate spatial index"
                  << " for table '" << geometry_table_ << "'"
                  << " as no primary key can be detected."
                  << " You should either declare an INTEGER PRIMARY KEY"
                  << " or set the 'key_field' option to force a"
                  << " given field to be used as the primary key";
                throw datasource_exception(s.str());
            }
        }
    }

    if (! extent_initialized_)
    {
#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats2__(std::clog, "sqlite_datasource::init(detect_extent)");
#endif
        // TODO - clean this up - reducing arguments
        std::string query = populate_tokens(table_);
        if (!sqlite_utils::detect_extent(dataset_,
                                         has_spatial_index_,
                                         extent_,
                                         index_table_,
                                         metadata_,
                                         geometry_field_,
                                         geometry_table_,
                                         key_field_,
                                         query))
        {
            std::ostringstream s;
            s << "Sqlite Plugin: extent could not be determined for table '"
              << geometry_table_ << "' and geometry field '" << geometry_field_ << "'"
              << " because an rtree spatial index is missing or empty."
              << " - either set the table 'extent' or create an rtree spatial index";

            throw datasource_exception(s.str());
        }
    }

}

std::string sqlite_datasource::populate_tokens(std::string const& sql) const
{
    std::string populated_sql = sql;
    if (boost::algorithm::ifind_first(populated_sql, intersects_token_))
    {
        // replace with dummy comparison that is true
        boost::algorithm::ireplace_first(populated_sql, intersects_token_, "1=1");
    }
    return populated_sql;
}

sqlite_datasource::~sqlite_datasource()
{
}

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
        std::string dbname = mapnik::util::trim_copy(spec.substr(0, atpos));
        std::string filename = mapnik::util::trim_copy(spec.substr(atpos + 1));
        // Normalize the filename and make it relative to dataset_name_
        if (filename.compare(":memory:") != 0 && mapnik::util::is_relative(filename))
        {
            filename = mapnik::util::make_relative(filename,dataset_name_);
        }

        // And add an init_statement_
        init_statements_.push_back("attach database '" + filename + "' as " + dbname);
    }
}

const char * sqlite_datasource::name()
{
    return "sqlite";
}

mapnik::datasource::datasource_t sqlite_datasource::type() const
{
    return type_;
}

box2d<double> sqlite_datasource::envelope() const
{
    return extent_;
}

boost::optional<mapnik::datasource::geometry_t> sqlite_datasource::get_geometry_type() const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "sqlite_datasource::get_geometry_type");
#endif

    boost::optional<mapnik::datasource::geometry_t> result;
    if (dataset_)
    {
        // finally, get geometry type by querying first feature
        std::ostringstream s;
        s << "SELECT " << geometry_field_
          << " FROM " << geometry_table_;
        if (row_limit_ > 0 && row_limit_ < 5)
        {
            s << " LIMIT " << row_limit_;
        }
        else
        {
            s << " LIMIT 5";
        }
        std::shared_ptr<sqlite_resultset> rs = dataset_->execute_query(s.str());
        int multi_type = 0;
        while (rs->is_valid() && rs->step_next())
        {
            int size;
            const char* data = (const char*) rs->column_blob(0, size);
            if (data)
            {
                mapnik::geometry_container paths;
                if (mapnik::geometry_utils::from_wkb(paths, data, size, format_))
                {
                    mapnik::util::to_ds_type(paths,result);
                    if (result)
                    {
                        int type = static_cast<int>(*result);
                        if (multi_type > 0 && multi_type != type)
                        {
                            result.reset(mapnik::datasource::Collection);
                            return result;
                        }
                        multi_type = type;
                    }
                }
            }
        }
    }

    return result;
}

layer_descriptor sqlite_datasource::get_descriptor() const
{
    return desc_;
}

featureset_ptr sqlite_datasource::features(query const& q) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "sqlite_datasource::features");
#endif

    if (dataset_)
    {
        mapnik::box2d<double> const& e = q.get_bbox();

        std::ostringstream s;
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();

        s << "SELECT " << geometry_field_;
        if (!key_field_.empty())
        {
            s << "," << key_field_;
            ctx->push(key_field_);
        }
        std::set<std::string> const& props = q.property_names();
        std::set<std::string>::const_iterator pos = props.begin();
        std::set<std::string>::const_iterator end = props.end();

        for ( ;pos != end;++pos)
        {
            // TODO - should we restrict duplicate key query?
            //if (*pos != key_field_)
            s << ",[" << *pos << "]";
            ctx->push(*pos);
        }
        s << " FROM ";

        std::string query(table_);

        if (! key_field_.empty() && has_spatial_index_)
        {
            // TODO - debug warn if fails
            sqlite_utils::apply_spatial_filter(query,
                                               e,
                                               table_,
                                               key_field_,
                                               index_table_,
                                               geometry_table_,
                                               intersects_token_);
        }
        else
        {
            query = populate_tokens(table_);
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

        MAPNIK_LOG_DEBUG(sqlite) << "sqlite_datasource: " << s.str();

        std::shared_ptr<sqlite_resultset> rs(dataset_->execute_query(s.str()));

        return std::make_shared<sqlite_featureset>(rs,
                                                     ctx,
                                                     desc_.get_encoding(),
                                                     e,
                                                     format_,
                                                     has_spatial_index_,
                                                     using_subquery_);
    }

    return featureset_ptr();
}

featureset_ptr sqlite_datasource::features_at_point(coord2d const& pt, double tol) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "sqlite_datasource::features_at_point");
#endif

    if (dataset_)
    {
        mapnik::box2d<double> e(pt.x, pt.y, pt.x, pt.y);
        e.pad(tol);
        std::ostringstream s;
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();

        s << "SELECT " << geometry_field_;
        if (!key_field_.empty())
        {
            s << "," << key_field_;
            ctx->push(key_field_);
        }

        std::vector<attribute_descriptor>::const_iterator itr = desc_.get_descriptors().begin();
        std::vector<attribute_descriptor>::const_iterator end = desc_.get_descriptors().end();

        for ( ; itr != end; ++itr)
        {
            std::string fld_name = itr->get_name();
            if (fld_name != key_field_)
            {
                s << ",[" << itr->get_name() << "]";
                ctx->push(itr->get_name());
            }
        }

        s << " FROM ";

        std::string query(table_);

        if (! key_field_.empty() && has_spatial_index_)
        {
            // TODO - debug warn if fails
            sqlite_utils::apply_spatial_filter(query,
                                               e,
                                               table_,
                                               key_field_,
                                               index_table_,
                                               geometry_table_,
                                               intersects_token_);
        }
        else
        {
            query = populate_tokens(table_);
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

        MAPNIK_LOG_DEBUG(sqlite) << "sqlite_datasource: " << s.str();

        std::shared_ptr<sqlite_resultset> rs(dataset_->execute_query(s.str()));

        return std::make_shared<sqlite_featureset>(rs,
                                                     ctx,
                                                     desc_.get_encoding(),
                                                     e,
                                                     format_,
                                                     has_spatial_index_,
                                                     using_subquery_);
    }

    return featureset_ptr();
}
