/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef POSTGIS_ASYNCRESULTSET_HPP
#define POSTGIS_ASYNCRESULTSET_HPP

#include <mapnik/debug.hpp>
#include <mapnik/datasource.hpp>

#include "connection_manager.hpp"
#include "resultset.hpp"
#include <queue>
#include <memory>

class postgis_processor_context;
using postgis_processor_context_ptr = std::shared_ptr<postgis_processor_context>;

class AsyncResultSet : public IResultSet,
                       private mapnik::util::noncopyable
{
  public:
    AsyncResultSet(postgis_processor_context_ptr const& ctx,
                   std::shared_ptr<Pool<Connection, ConnectionCreator>> const& pool,
                   std::shared_ptr<Connection> const& conn,
                   std::string const& sql)
        : ctx_(ctx),
          pool_(pool),
          conn_(conn),
          sql_(sql),
          is_closed_(false)
    {}

    virtual bool use_connection() { return true; }

    virtual ~AsyncResultSet() { close(); }

    void abort()
    {
        if (conn_ && conn_->isPending())
        {
            MAPNIK_LOG_DEBUG(postgis) << "AsyncResultSet: aborting pending connection - " << conn_.get();
            // there is no easy way to abort a pending connection, so we close it : this will ensure that
            // the connection will be recycled in the pool
            conn_->close();
        }
    }

    virtual void close()
    {
        if (!is_closed_)
        {
            rs_.reset();
            is_closed_ = true;
            if (conn_)
            {
                if (conn_->isPending())
                {
                    abort();
                }
                conn_.reset();
            }
        }
    }

    virtual int getNumFields() const { return rs_->getNumFields(); }

    virtual bool next()
    {
        bool next_res = false;
        if (!rs_)
        {
            // Ensure connection is valid
            if (conn_ && conn_->isOK())
            {
                rs_ = conn_->getAsyncResult();
            }
            else
            {
                throw mapnik::datasource_exception("invalid connection in AsyncResultSet::next");
            }
        }

        next_res = rs_->next();
        if (!next_res)
        {
            rs_.reset();
            rs_ = conn_->getNextAsyncResult();
            if (rs_ && rs_->next())
            {
                return true;
            }
            close();
            prepare_next();
        }
        return next_res;
    }

    virtual char const* getFieldName(int index) const { return rs_->getFieldName(index); }

    virtual int getFieldLength(int index) const { return rs_->getFieldLength(index); }

    virtual int getFieldLength(char const* name) const { return rs_->getFieldLength(name); }

    virtual int getTypeOID(int index) const { return rs_->getTypeOID(index); }

    virtual int getTypeOID(char const* name) const { return rs_->getTypeOID(name); }

    virtual bool isNull(int index) const { return rs_->isNull(index); }

    virtual char const* getValue(int index) const { return rs_->getValue(index); }

    virtual char const* getValue(char const* name) const { return rs_->getValue(name); }

  private:
    postgis_processor_context_ptr ctx_;
    std::shared_ptr<Pool<Connection, ConnectionCreator>> pool_;
    std::shared_ptr<Connection> conn_;
    std::string sql_;
    std::shared_ptr<ResultSet> rs_;
    bool is_closed_;

    void prepare()
    {
        conn_ = pool_->borrowObject();
        if (conn_ && conn_->isOK())
        {
            conn_->executeAsyncQuery(sql_, 1);
        }
        else
        {
            throw mapnik::datasource_exception("Postgis Plugin: bad connection");
        }
    }

    void prepare_next();
};

class postgis_processor_context : public mapnik::IProcessorContext
{
  public:
    postgis_processor_context()
        : num_async_requests_(0)
    {}
    ~postgis_processor_context() {}

    void add_request(std::shared_ptr<AsyncResultSet> const& req) { q_.push(req); }

    std::shared_ptr<AsyncResultSet> pop_next_request()
    {
        std::shared_ptr<AsyncResultSet> r;
        if (!q_.empty())
        {
            r = q_.front();
            q_.pop();
        }
        return r;
    }

    int num_async_requests_;

  private:
    using async_queue = std::queue<std::shared_ptr<AsyncResultSet>>;
    async_queue q_;
};

inline void AsyncResultSet::prepare_next()
{
    // ensure cnx pool has unused cnx
    std::shared_ptr<AsyncResultSet> next = ctx_->pop_next_request();
    if (next)
    {
        next->prepare();
    }
}

#endif // POSTGIS_ASYNCRESULTSET_HPP
