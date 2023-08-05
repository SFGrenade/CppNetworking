#pragma once

#include <asio.hpp>
#include <functional>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>


namespace SFG {

class Server {
  public:
  Server( uint16_t port );
  ~Server();

  void waitForServer();
  void prepareServer( std::function< std::string( uint16_t ) > new_callback );
  void runServer();
  void stopServer();

  private:
  static std::string toString( asio::ip::tcp::endpoint const& endpoint );

  private:
  std::shared_ptr< spdlog::logger > logger_;

  uint16_t port_;
  std::function< std::string( uint16_t ) > callback_;
  std::thread* asio_v4_thread_;
  std::thread* asio_v6_thread_;
  bool loop_;

  asio::error_code asio_error_code_;
  asio::io_context asio_io_context_;
  asio::ip::tcp::acceptor asio_ip_tcp_v4_acceptor_;
  asio::ip::tcp::acceptor asio_ip_tcp_v6_acceptor_;
  asio::ip::tcp::socket asio_ip_tcp_socket_;
};

}  // namespace SFG
