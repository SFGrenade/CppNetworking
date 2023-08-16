#include "wrapper/pubSub.hpp"

#include "main.pb.h"

namespace SFG {
namespace Networking {

PubSub::PubSub( std::string const& host, uint16_t port, bool isServer )
    : ZmqWrap( host, port, isServer ? zmq::socket_type::pub : zmq::socket_type::sub ),
      logger_( spdlog::get( "PubSub" ) ),
      isServer_( isServer ),
      status_( isServer ? PubSub::Status::Sending : PubSub::Status::Receiving ) {
  logger_->trace( "PubSub( host: \"{}\", port: {}, isServer: {} )", host, port, isServer );

  if( isServer_ ) {
    try {
      zmqSocket_.bind( fmt::format( "{}:{}", host_, port_ ) );
    } catch( const std::exception& e ) {
      logger_->error( "Error during bind: {}", e.what() );
    }
  } else {
    zmqSocket_.set( zmq::sockopt::subscribe, "" );  // subscribe to all incoming messages
    try {
      zmqSocket_.connect( fmt::format( "{}:{}", host_, port_ ) );
    } catch( const std::exception& e ) {
      logger_->error( "Error during bind: {}", e.what() );
    }
  }

  logger_->trace( "PubSub()~" );
}

PubSub::~PubSub() {
  logger_->trace( "~PubSub()" );

  logger_->trace( "~PubSub()~" );
}

bool PubSub::canSend() const {
  // logger_->trace( "canSend()" );

  // logger_->trace( "canSend()~" );
  return status_ == PubSub::Status::Sending;
}

void PubSub::didSend() {
  logger_->trace( "didSend()" );

  logger_->trace( "didSend()~" );
}

bool PubSub::canRecv() const {
  // logger_->trace( "canRecv()" );

  // logger_->trace( "canRecv()~" );
  return status_ == PubSub::Status::Receiving;
}

void PubSub::didRecv() {
  logger_->trace( "didRecv()" );

  logger_->trace( "didRecv()~" );
}

}  // namespace Networking
}  // namespace SFG
