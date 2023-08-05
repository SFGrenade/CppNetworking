#include "client.hpp"

namespace SFG {

std::string Client::toString( asio::ip::tcp::endpoint const& endpoint ) {
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

Client::Client( std::string const& host, std::string const& port )
    : logger_( spdlog::get( "Client" ) ),
      host_( host ),
      port_( port ),
      callback_( nullptr ),
      asio_thread_( nullptr ),
      loop_( false ),
      asio_error_code_(),
      asio_io_context_(),
      asio_ip_tcp_resolver_( asio_io_context_ ),
      asio_ip_tcp_socket_( asio_io_context_ ),
      readingBuffer_() {
  logger_->trace( "Client()" );
  logger_->trace( "Client()~" );
}

Client::~Client() {
  logger_->trace( "~Client()" );
  if( asio_thread_ ) {
    if( asio_thread_->joinable() ) {
      asio_thread_->join();
    }
    delete asio_thread_;
    asio_thread_ = nullptr;
  }
  logger_->trace( "~Client()~" );
}

void Client::waitForClient() {
  logger_->trace( "waitForClient()" );

  if( asio_thread_ ) {
    if( asio_thread_->joinable() ) {
      asio_thread_->join();
    }
  }

  logger_->trace( "waitForClient()~" );
}

void Client::prepareClient( std::function< void( std::string const& ) > new_callback ) {
  logger_->trace( "prepareClient()" );

  callback_ = new_callback;

  logger_->trace( "prepareClient()~" );
}

void Client::runClient() {
  logger_->trace( "runClient()" );

  asio_ip_tcp_resolver_.async_resolve( host_, port_, [this]( asio::error_code const& ec, asio::ip::tcp::resolver::iterator const& it ) {
    this->logger_->trace( "async_resolve( ec: {}, it: [\"{}\", \"{}\", {}] )", ec.value(), it->host_name(), it->service_name(), toString( it->endpoint() ) );
    this->resolve_callback( ec, it );
  } );

  loop_ = true;

  asio_thread_ = new std::thread( [this]() {
    this->logger_->trace( "asio_thread_()" );
    while( this->loop_ ) {
      try {
        this->asio_io_context_.run( this->asio_error_code_ );
      } catch( const std::exception& e ) {
        this->logger_->error( "Error {} while context.run: {}", this->asio_error_code_.value(), e.what() );
      }
    }
    this->logger_->trace( "asio_thread_()~" );
  } );

  logger_->trace( "runClient()~" );
}

void Client::stopClient() {
  logger_->trace( "stopClient()" );

  loop_ = false;

  logger_->trace( "stopClient()~" );
}

void Client::resolve_callback( asio::error_code const& ec, asio::ip::tcp::resolver::iterator const& it ) {
  logger_->trace( "resolve_callback( ec: {}, it: [\"{}\", \"{}\", {}] )", ec.value(), it->host_name(), it->service_name(), toString( it->endpoint() ) );

  if( !ec ) {
    asio_ip_tcp_socket_.async_connect( *it, [this]( asio::error_code const& ec ) {
      this->logger_->trace( "async_connect( ec: {} )", ec.value() );
      this->connect_callback( ec );
    } );
  }

  logger_->trace( "resolve_callback()~" );
}

void Client::connect_callback( asio::error_code const& ec ) {
  logger_->trace( "connect_callback( ec: {} )", ec.value() );

  if( !ec ) {
    // std::string r = fmt::format( "GET / HTTP/1.1\r\nHost: {}\r\n\r\n", host_ );
    // asio::write( asio_ip_tcp_socket_, asio::buffer( r ) );
    readingBuffer_.fill( '\0' );
    asio_ip_tcp_socket_.async_read_some( asio::buffer( readingBuffer_ ), [this]( asio::error_code const& ec, std::size_t transferredBytes ) {
      this->logger_->trace( "async_read_some( ec: {}, transferredBytes: {} )", ec.value(), static_cast< uint64_t >( transferredBytes ) );
      this->read_callback( ec, transferredBytes );
    } );
  }

  logger_->trace( "connect_callback()~" );
}

void Client::read_callback( asio::error_code const& ec, std::size_t transferredBytes ) {
  logger_->trace( "read_callback( ec: {}, transferredBytes: {} )", ec.value(), static_cast< uint64_t >( transferredBytes ) );
  callback_( std::string( readingBuffer_.data(), transferredBytes ) );

  stopClient();
  logger_->trace( "read_callback()~" );
}

}  // namespace SFG
