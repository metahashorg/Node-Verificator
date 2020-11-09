#ifndef MHCURL_HPP
#define MHCURL_HPP

#include <string>
#include <functional>
#include <boost/asio.hpp>

namespace http::server {

void http_server(boost::asio::io_context& io_context, int http_port, std::function<std::string(const std::string&, const std::string&)> processor);

}

#endif // MHCURL_HPP
