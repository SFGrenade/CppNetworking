#include "wrapper/reqRep.hpp"

#include "main.pb.h"

namespace SFG {
namespace Networking {

ReqRep::ReqRep( std::string const& host, uint16_t port, bool isServer )
    : ZmqWrap( host, port, isServer ? zmq::socket_type::rep : zmq::socket_type::req ),
      logger_( spdlog::get( "ReqRep" ) ),
      isServer_( isServer ),
      status_( isServer ? ReqRep::Status::Receiving : ReqRep::Status::Sending ) {
  logger_->trace( "ReqRep( host: \"{}\", port: {}, isServer: {} )", host_, port_, isServer_ );

  if( isServer_ ) {
    logger_->trace( "ReqRep - 1" );
    try {
      zmqSocket_.bind( fmt::format( "{}:{}", host_, port_ ) );
    } catch( const std::exception& e ) {
      logger_->error( "Error during bind: {}", e.what() );
    }
  } else {
    logger_->trace( "ReqRep - 2" );
    try {
      zmqSocket_.connect( fmt::format( "{}:{}", host_, port_ ) );
    } catch( const std::exception& e ) {
      logger_->error( "Error during bind: {}", e.what() );
    }
  }
  logger_->trace( "ReqRep - 3" );

  logger_->trace( "ReqRep()~" );
}

ReqRep::~ReqRep() {
  logger_->trace( "~ReqRep()" );

  logger_->trace( "~ReqRep()~" );
}

bool ReqRep::canSend() const {
  // logger_->trace( "canSend()" );

  // logger_->trace( "canSend()~" );
  return status_ == ReqRep::Status::Sending;
}

void ReqRep::didSend() {
  logger_->trace( "didSend()" );

  status_ = ReqRep::Status::Receiving;

  logger_->trace( "didSend()~" );
}

bool ReqRep::canRecv() const {
  // logger_->trace( "canRecv()" );

  // logger_->trace( "canRecv()~" );
  return status_ == ReqRep::Status::Receiving;
}

void ReqRep::didRecv() {
  logger_->trace( "didRecv()" );

  status_ = ReqRep::Status::Sending;

  logger_->trace( "didRecv()~" );
}

}  // namespace Networking
}  // namespace SFG
