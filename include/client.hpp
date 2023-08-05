#pragma once

#include <asio.hpp>
#include <functional>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>


namespace SFG {

class Client {
  public:
  Client( std::string const& host, std::string const& port );
  ~Client();

  void waitForClient();
  void prepareClient( std::function< void( std::string const& ) > new_callback );
  void runClient();
  void stopClient();

  private:
  static std::string toString( asio::ip::tcp::endpoint const& endpoint );

  void resolve_callback( asio::error_code const& ec, asio::ip::tcp::resolver::iterator const& it );
  void connect_callback( asio::error_code const& ec );
  void read_callback( asio::error_code const& ec, std::size_t transferredBytes );

  private:
  std::shared_ptr< spdlog::logger > logger_;

  std::string host_;
  std::string port_;
  std::function< void( std::string const& ) > callback_;
  std::thread* asio_thread_;
  bool loop_;

  asio::error_code asio_error_code_;
  asio::io_context asio_io_context_;
  asio::ip::tcp::resolver asio_ip_tcp_resolver_;
  asio::ip::tcp::socket asio_ip_tcp_socket_;
  std::array< char, 4096 > readingBuffer_;
};

}  // namespace SFG
