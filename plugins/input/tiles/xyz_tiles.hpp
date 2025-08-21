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

#ifndef XYZ_TILES_HPP
#define XYZ_TILES_HPP

#include <boost/url.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>

#include <boost/lockfree/spsc_queue.hpp>
#include <boost/algorithm/string.hpp>
#include <chrono>
#include <thread>
#include <tuple>


namespace beast = boost::beast;
namespace http  = beast::http;
using tcp = boost::asio::ip::tcp;

using zxy = std::tuple<std::size_t, std::size_t, std::size_t>;

struct tile_data
{
    std::size_t zoom;
    std::size_t x;
    std::size_t y;
    std::string data;

    tile_data() = default;
    tile_data(tile_data const&) = default;
    tile_data(std::size_t zoom_, std::size_t x_, std::size_t y_, std::string && data_)
        : zoom(zoom_), x(x_), y(y_), data(std::move(data_))
    {}
};

using queue_type = boost::lockfree::spsc_queue<tile_data>;

class tiles_stash
{
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    std::atomic<std::size_t> index_;
    std::vector<zxy> & targets_;
    queue_type & queue_;
public:
    tiles_stash(boost::asio::io_context& ioc, std::vector<zxy> & targets, queue_type & queue)
        : strand_(ioc.get_executor()),
          index_(0),
          targets_(targets),
          queue_(queue)
    {
    }

    std::vector<zxy> & targets()
    {
        return targets_;
    }

    void push_async(tile_data const& data)
    {
        boost::asio::post(strand_, [&, data] {
            while (!queue_.push(data))
                ;
        });
    }

    void push(tile_data const& data)
    {
        queue_.push(data);
    }

    std::optional<zxy> get_zxy()
    {
        auto const current = index_++;
        if (current >= targets_.size())
            return {};
        return targets_[current];
    }
};


// Report a failure
inline void fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << " FIXME\n";
}

namespace  xyz_tiles {

inline std::string metadata_http(std::string const& host, std::string const& port, std::string const& target, beast::error_code& ec)
{
    ec = {};
    boost::asio::io_context ioc;
    std::string result;
    boost::asio::spawn(ioc, [&](boost::asio::yield_context yield)
    {
        boost::asio::ip::tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);
        auto const endpoints = resolver.async_resolve(host, port, yield[ec]);
        if (ec) return;
        stream.expires_after(std::chrono::seconds(30));
        stream.async_connect(endpoints, yield[ec]);
        if (ec) return;
        //HTTP GET
        http::request<http::string_body> req{http::verb::get, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        http::async_write(stream, req, yield[ec]);
        if (ec) return;
        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::async_read(stream, buffer, res, yield[ec]);
        if (ec) return;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);
        if (ec) return;
        result = std::move(res.body());
    }, [] (std::exception_ptr ex)
    {
        if (ex) std::rethrow_exception(ex);
    });
    ioc.run();
    return result;
}

inline boost::json::value metadata(std::string const& url_str)
{
    auto result = boost::urls::parse_uri_reference(url_str);
    if (!result)
    {
        throw mapnik::datasource_exception(result.error().to_string());
    }
    boost::json::value json_value;

    std::string scheme = result->scheme();
    std::string host = result->host();
    std::string port = result->port();
    std::string target = result->path();
    beast::error_code ec;
    std::string str = metadata_http(host, port, target, ec);
    if (ec)
    {
        mapnik::datasource_exception("Tiles datasource: Error fetching metatada" + ec.to_string());
    }
    boost::json::value json;
    try
    {
        json = boost::json::parse(str);
    }
    catch (std::exception e)
    {
        mapnik::datasource_exception("Tiles datasource: Failed to parse JSON");
    }
    return json;
}

}

class worker : public std::enable_shared_from_this<worker>
{
    tiles_stash& stash_;
    std::string url_template_;
    boost::asio::strand<boost::asio::io_context::executor_type> ex_;
    tcp::resolver resolver_;
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::empty_body> req_;
    http::response<http::string_body> res_;
    zxy tile_;
public:
    worker(worker&&) = default;

    explicit worker(boost::asio::io_context& ioc, std::string const& url_template, tiles_stash & stash)
        : stash_(stash),
          url_template_(url_template),
          ex_(boost::asio::make_strand(ioc.get_executor())),
          resolver_(boost::asio::make_strand(ioc)),
          stream_(boost::asio::make_strand(ioc))
    {
        req_.version(11); // HTTP 1.1
        req_.method(http::verb::get);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    }

    // Start the asynchronous operation
    void run(std::string const& host, std::string const& port)
    {
        req_.set(http::field::host, host);
        resolver_.async_resolve(
            host,
            port,
            beast::bind_front_handler(
                &worker::on_resolve,
                shared_from_this()));
    }

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results)
    {
        if (ec) return fail(ec, "resolve");
        // Set a timeout on the operation
        stream_.expires_after(std::chrono::seconds(10));
        // Make the connection on the IP address we get from a lookup
        stream_.async_connect(
            results,
            beast::bind_front_handler(
                &worker::on_connect,
                shared_from_this()));
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
    {
        if (ec) return fail(ec, "connect");
        auto zxy = stash_.get_zxy();
        if (!zxy)
        {
            // Work is done, gracefully close the socket
            stream_.socket().shutdown(tcp::socket::shutdown_both, ec);
            // not_connected happens sometimes so don't bother reporting it.
            if (ec && ec != beast::errc::not_connected)
            {
                return fail(ec, "shutdown");
                // If we get here then the connection is closed gracefully
            }
            return;
        }
        tile_ = *zxy;
        auto url = boost::urls::format(url_template_,
                                       {{"z", std::get<0>(tile_)},
                                        {"x", std::get<1>(tile_)},
                                        {"y", std::get<2>(tile_)}});

        std::cerr << "\e[46m target:" << url.path() << " thread:" << std::this_thread::get_id() <<"\e[0m" << std::endl;
        req_.target(url.path());
        // Set a timeout on the operation
        stream_.expires_after(std::chrono::seconds(10));
        // Send the HTTP request to the remote host
        http::async_write(stream_, req_,
            beast::bind_front_handler(
                &worker::on_write,
                shared_from_this()));
    }
    void on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec) return fail(ec, "write");
        // Receive the HTTP response
        res_ = {};
        // Set a timeout on the operation
        stream_.expires_after(std::chrono::seconds(10));
        http::async_read(stream_, buffer_, res_,
            beast::bind_front_handler(
                &worker::on_read,
                shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        //using namespace std::chrono_literals;
        //std::this_thread::sleep_for(50ms);

        if(ec) return fail(ec, "read");
        std::cerr << req_.target() << std::endl;
        //std::string data = res_.body();
        std::cerr << "response result:" << res_.result() << std::endl;
        //std::cerr << "response size:" << data.size() << std::endl;
        stash_.push_async(tile_data(std::get<0>(tile_), std::get<1>(tile_), std::get<2>(tile_), std::move(res_.body())));
        auto zxy = stash_.get_zxy();
        if (!zxy)
        {
            // Work is done, gracefully close the socket
            stream_.socket().shutdown(tcp::socket::shutdown_both, ec);
            // not_connected happens sometimes so don't bother reporting it.
            if (ec && ec != beast::errc::not_connected)
            {
                return fail(ec, "shutdown");
                // If we get here then the connection is closed gracefully
            }
            return;
        }
        tile_ = *zxy;
        auto url = boost::urls::format(url_template_,
                                       {{"z", std::get<0>(tile_)},
                                        {"x", std::get<1>(tile_)},
                                        {"y", std::get<2>(tile_)}});
        std::cerr << "\e[1;46m target:" << url.path() << " thread:" << std::this_thread::get_id() <<"\e[0m" << std::endl;
        req_.target(url.path());
        http::async_write(stream_, req_,
            beast::bind_front_handler(
                &worker::on_write,
                shared_from_this()));
    }
};


#endif // XYZ_FEATURESET_HPP
