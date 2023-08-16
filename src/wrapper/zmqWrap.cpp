#include "wrapper/zmqWrap.hpp"

#include "main.pb.h"

namespace SFG {
namespace Networking {

Subscription::Subscription() : message( nullptr ), callback( nullptr ) {}
Subscription::Subscription( google::protobuf::Message* message, std::function< void( google::protobuf::Message const& ) > callback )
    : message( message ), callback( callback ) {}

ZmqWrap::ZmqWrap( std::string const& host, uint16_t port, zmq::socket_type socketType )
    : logger_( spdlog::get( "ZmqWrap" ) ), host_( host ), port_( port ), zmqContext_( 1 ), zmqSocket_( zmqContext_, socketType ), queueToSend_() {
  logger_->trace( "ZmqWrap( host: \"{}\", port: {}, socketType: {} )", host, port, static_cast< int >( socketType ) );

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
  for( auto pair : subscribedMessages_ ) {
    delete pair.second.message;
  }
  subscribedMessages_.clear();
  zmqSocket_.close();
  zmqContext_.shutdown();
  logger_->trace( "~ZmqWrap()~" );
}

void ZmqWrap::subscribe( google::protobuf::Message* message, std::function< void( google::protobuf::Message const& ) > callback ) {
  logger_->trace( "subscribe( message: {} (\"{}\"), callback )", static_cast< void* >( message ), message->GetTypeName() );
  std::string messageType = message->GetTypeName();
  auto found = subscribedMessages_.find( messageType );
  if( found != subscribedMessages_.end() ) {
    logger_->trace( "subscribe - changing callback" );
    subscribedMessages_[messageType].callback = callback;
  } else {
    logger_->trace( "subscribe - adding message to subscribed list" );
    subscribedMessages_[messageType] = Subscription{ message, callback };
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

void ZmqWrap::run() {
  // logger_->trace( "run()" );

  if( canSend() && !queueToSend_.empty() ) {
    mutexForSendQueue_.lock();
    google::protobuf::Message* msgToSend = queueToSend_.front();
    SFG::Proto::Wrapper* actualMessage = new SFG::Proto::Wrapper();
    actualMessage->set_protoname( msgToSend->GetTypeName() );
    actualMessage->set_protocontent( msgToSend->SerializeAsString() );
    zmq::send_result_t sendResult = zmqSocket_.send( zmq::buffer( actualMessage->SerializeAsString() ), zmq::send_flags::dontwait );
    delete actualMessage;
    if( sendResult ) {
      queueToSend_.pop();
      delete msgToSend;
      didSend();
    } else {
      // logger_->warn( "No message sent!" );
    }
    mutexForSendQueue_.unlock();
  } else if( canRecv() ) {
    zmq::message_t receivedReply;
    SFG::Proto::Wrapper receivedWrapper;
    zmq::recv_result_t recvResult = zmqSocket_.recv( receivedReply, zmq::recv_flags::dontwait );
    if( recvResult ) {
      receivedWrapper.ParseFromString( receivedReply.to_string() );
      auto found = subscribedMessages_.find( receivedWrapper.protoname() );
      if( found != subscribedMessages_.end() ) {
        found->second.message->ParseFromString( receivedWrapper.protocontent() );
        found->second.callback( *( found->second.message ) );
        didRecv();
      } else {
        throw std::runtime_error( fmt::format( "Topic '{}' not subscribed!", receivedWrapper.protoname() ) );
      }
    } else {
      // logger_->warn( "No message received!" );
    }
  }

  // logger_->trace( "run()~" );
}

}  // namespace Networking
}  // namespace SFG
