#include "wrapper/zmqWrap.hpp"

#include "main.pb.h"

namespace SFG {
namespace Networking {

ZmqWrap::ZmqWrap( std::string const& host, uint16_t port, ZmqWrap::Status startingStatus, zmq::socket_type socketType )
    : logger_( spdlog::get( "ZmqWrap" ) ),
      host_( host ),
      port_( port ),
      status_( startingStatus ),
      zmqContext_( 1, 1 ),
      zmqSocket_( zmqContext_, socketType ),
      queueToSend_() {
  logger_->trace( "ZmqWrap( host: \"{}\", port: {}, startingStatus: {}, socketType: {} )",
                  host,
                  port,
                  static_cast< int >( startingStatus ),
                  static_cast< int >( socketType ) );

  zmqSocket_.set( zmq::sockopt::linger, 0 );  // don't wait after destructor is called

  logger_->trace( "ZmqWrap()~" );
}

ZmqWrap::~ZmqWrap() {
  logger_->trace( "~ZmqWrap()" );
  while( !queueToSend_.empty() ) {
    google::protobuf::Message* tmp = queueToSend_.front();
    if( tmp ) {
      delete tmp;
    }
    queueToSend_.pop();
  }
  for( int i = 0; i < subscribedMessages_.size(); i++ ) {
    delete subscribedMessages_[i];
  }
  zmqSocket_.close();
  zmqContext_.shutdown();
  logger_->trace( "~ZmqWrap()~" );
}

void ZmqWrap::subscribe( google::protobuf::Message* message, std::function< void( google::protobuf::Message const& ) > callback ) {
  logger_->trace( "subscribe( message: {} (\"{}\"), callback )", static_cast< void* >( message ), message->GetTypeName() );
  bool contains = false;
  for( int i = 0; i < subscribedMessages_.size(); i++ ) {
    if( subscribedMessages_[i]->GetTypeName() == message->GetTypeName() ) {
      contains = true;
      break;
    }
  }
  if( !contains ) {
    logger_->trace( "subscribe - adding message to subscribed list" );
    subscribedMessages_.push_back( message );
    subscribedCallbacks_.push_back( callback );
  }
  logger_->trace( "subscribe()~" );
}

void ZmqWrap::sendMessage( google::protobuf::Message* message ) {
  logger_->trace( "sendMessage( message: {} (\"{}\") )", static_cast< void* >( message ), message->GetTypeName() );

  mutexForSendQueue_.lock();
  queueToSend_.push( message );
  mutexForSendQueue_.unlock();

  logger_->trace( "sendMessage()~" );
}

ZmqWrap::Status ZmqWrap::status() const {
  // logger_->trace( "status()" );

  // logger_->trace( "status()~" );
  return status_;
}

void ZmqWrap::setStatus( ZmqWrap::Status newStatus ) {
  logger_->trace( "setStatus( newStatus: {} )", static_cast< int >( newStatus ) );

  status_ = newStatus;

  logger_->trace( "setStatus()~" );
}

}  // namespace Networking
}  // namespace SFG
