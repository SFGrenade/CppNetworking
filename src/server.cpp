#include "server.hpp"

namespace SFG {

std::string Server::toString( asio::ip::tcp::endpoint const& endpoint ) {
  asio::ip::address address = endpoint.address();
  asio::ip::tcp protocol = endpoint.protocol();
  asio::ip::tcp protocolV4 = protocol.v4();
  asio::ip::tcp protocolV6 = protocol.v6();

  std::string addressString = address.to_string();
  // if( address.is_unspecified() ) {
  //   addressString = "unspecified";
  // } else if( address.is_multicast() ) {
  //   addressString = address.to_string();
  // } else if( address.is_loopback() ) {
  //   addressString = "loopback";
  // } else if( address.is_v4() ) {
  //   addressString = address.to_v4().to_string();
  // } else if( address.is_v6() ) {
  //   addressString = address.to_v6().to_string();
  // }
  asio::ip::port_type port = endpoint.port();
  std::size_t capacity = endpoint.capacity();
  uint32_t protocolFamily = protocol.family();
  uint32_t protocolProtocol = protocol.protocol();
  uint32_t protocolType = protocol.type();
  uint32_t protocolV4Family = protocolV4.family();
  uint32_t protocolV4Protocol = protocolV4.protocol();
  uint32_t protocolV4Type = protocolV4.type();
  uint32_t protocolV6Family = protocolV6.family();
  uint32_t protocolV6Protocol = protocolV6.protocol();
  uint32_t protocolV6Type = protocolV6.type();

  return fmt::format( "[ {}:{}, {}, [ {}, {}, {}, [ {}, {}, {} ], [ {}, {}, {} ] ] ]",
                      addressString,
                      port,
                      capacity,
                      protocolFamily,
                      protocolProtocol,
                      protocolType,
                      protocolV4Family,
                      protocolV4Protocol,
                      protocolV4Type,
                      protocolV6Family,
                      protocolV6Protocol,
                      protocolV6Type );
}

Server::Server( uint16_t port )
    : logger_( spdlog::get( "Server" ) ),
      port_( port ),
      callback_( nullptr ),
      asio_v4_thread_( nullptr ),
      asio_v6_thread_( nullptr ),
      loop_( false ),
      asio_error_code_(),
      asio_io_context_(),
      asio_ip_tcp_v4_acceptor_( asio_io_context_, asio::ip::tcp::endpoint( asio::ip::tcp::v4(), port_ ) ),
      asio_ip_tcp_v6_acceptor_( asio_io_context_, asio::ip::tcp::endpoint( asio::ip::tcp::v6(), port_ ) ),
      asio_ip_tcp_socket_( asio_io_context_ ) {
  logger_->trace( "Server()" );
  logger_->trace( "Server()~" );
}

Server::~Server() {
  logger_->trace( "~Server()" );
  if( asio_v4_thread_ ) {
    if( asio_v4_thread_->joinable() ) {
      asio_v4_thread_->join();
    }
    delete asio_v4_thread_;
    asio_v4_thread_ = nullptr;
  }
  if( asio_v6_thread_ ) {
    if( asio_v6_thread_->joinable() ) {
      asio_v6_thread_->join();
    }
    delete asio_v6_thread_;
    asio_v6_thread_ = nullptr;
  }
  logger_->trace( "~Server()~" );
}

void Server::waitForServer() {
  logger_->trace( "waitForServer()" );

  if( asio_v4_thread_ ) {
    if( asio_v4_thread_->joinable() ) {
      asio_v4_thread_->join();
    }
  }
  if( asio_v6_thread_ ) {
    if( asio_v6_thread_->joinable() ) {
      asio_v6_thread_->join();
    }
  }

  logger_->trace( "waitForServer()~" );
}

void Server::prepareServer( std::function< std::string( uint16_t ) > new_callback ) {
  logger_->trace( "prepareServer()" );

  callback_ = new_callback;

  logger_->trace( "prepareServer()~" );
}

void Server::runServer() {
  logger_->trace( "runServer()" );

  loop_ = true;

  asio_v4_thread_ = new std::thread( [this]() {
    this->logger_->trace( "asio_v4_thread_()" );
    while( this->loop_ ) {
      try {
        asio::ip::tcp::socket socket( asio_io_context_ );
        asio_ip_tcp_v4_acceptor_.accept( socket );
        std::string message = this->callback_( 4 );
        asio::write( socket, asio::buffer( message ), asio_error_code_ );
      } catch( const std::exception& e ) {
        this->logger_->error( "Error {} while serving message: {}", this->asio_error_code_.value(), e.what() );
      }
    }
    this->logger_->trace( "asio_v4_thread_()~" );
  } );
  asio_v6_thread_ = new std::thread( [this]() {
    this->logger_->trace( "asio_v6_thread_()" );
    while( this->loop_ ) {
      try {
        asio::ip::tcp::socket socket( asio_io_context_ );
        asio_ip_tcp_v6_acceptor_.accept( socket );
        std::string message = this->callback_( 6 );
        asio::write( socket, asio::buffer( message ), asio_error_code_ );
      } catch( const std::exception& e ) {
        this->logger_->error( "Error {} while serving message: {}", this->asio_error_code_.value(), e.what() );
      }
    }
    this->logger_->trace( "asio_v6_thread_()~" );
  } );

  logger_->trace( "runServer()~" );
}

void Server::stopServer() {
  logger_->trace( "stopServer()" );

  loop_ = false;

  logger_->trace( "stopServer()~" );
}

}  // namespace SFG
