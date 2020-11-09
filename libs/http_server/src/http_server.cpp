#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <string>

#include <meta_log.hpp>


namespace http::server {
    
namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

void fail(beast::error_code ec, const std::string& what)
{
    DEBUG_COUT(what + ": " + ec.message());
}

void do_session(beast::tcp_stream& stream, std::function<std::string(const std::string&, const std::string&)> processor, net::yield_context yield)
{
    bool keep_alive = true;
    beast::error_code ec;
    beast::flat_buffer buffer;

    while (keep_alive) {
        stream.expires_after(std::chrono::seconds(60));

        http::request<http::string_body> req;
        http::async_read(stream, buffer, req, yield[ec]);
        if (ec == http::error::end_of_stream) {
            break;
        }
        if (ec) {
            return fail(ec, "read");
        }

        keep_alive = req.keep_alive();
        auto& request_body = req.body();
        std::string request_ur(req.target().data(), req.target().size());

        std::string response_body = processor(request_body, request_ur);

        http::response<http::string_body> resp;
        resp.result(http::status::ok);
        resp.body() = response_body;
        resp.keep_alive(keep_alive);
        resp.content_length(resp.body().size());

        http::async_write(stream, resp, yield[ec]);

        if (ec) {
            return fail(ec, "write");
        }
    }

    stream.socket().shutdown(tcp::socket::shutdown_send, ec);
}

void do_listen(net::io_context& io_context, unsigned short port, std::function<std::string(const std::string&, const std::string&)> processor, net::yield_context yield)
{
    beast::error_code ec;

    boost::asio::ip::tcp::acceptor acceptor(io_context);
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

    acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        fail(ec, "open");
        do_listen(io_context, port, processor, yield);
        return;
    }
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
    if (ec) {
        fail(ec, "set_option");
        do_listen(io_context, port, processor, yield);
        return;
    }
    acceptor.set_option(boost::asio::ip::tcp::acceptor::enable_connection_aborted(true), ec);
    if (ec) {
        fail(ec, "set_option");
        do_listen(io_context, port, processor, yield);
        return;
    }
    acceptor.bind(endpoint, ec);
    if (ec) {
        fail(ec, "open");
        do_listen(io_context, port, processor, yield);
        return;
    }
    acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        fail(ec, "listen");
        do_listen(io_context, port, processor, yield);
        return;
    }

    for (;;) {
        tcp::socket socket(io_context);
        acceptor.async_accept(socket, yield[ec]);
        if (ec) {
            fail(ec, "async_accept");
        } else {
            boost::asio::spawn(acceptor.get_executor(), std::bind(&do_session, beast::tcp_stream(std::move(socket)), processor, std::placeholders::_1));
        }
    }
}

void http_server(boost::asio::io_context& io_context, int http_port, std::function<std::string(const std::string&, const std::string&)> processor)
{
    auto const port = static_cast<unsigned short>(http_port);

    boost::asio::spawn(io_context, std::bind(&do_listen, std::ref(io_context), port, processor, std::placeholders::_1));
}

}