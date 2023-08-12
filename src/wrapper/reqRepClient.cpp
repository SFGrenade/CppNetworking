#include "wrapper/reqRepClient.hpp"

#include "main.pb.h"

namespace SFG {
namespace Networking {

ReqRepClient::ReqRepClient( std::string const& host, uint16_t port, bool isServer )
    : logger_( spdlog::get( "ReqRepClient" ) ),
      host_( host ),
      port_( port ),
      isServer_( isServer ),
      zmqContext_( 1, 1 ),
      zmqSocket_( zmqContext_, isServer_ ? zmq::socket_type::rep : zmq::socket_type::req ),
      queueToSend_(),
      sending_( !isServer_ ) {
  logger_->trace( "ReqRepClient( host: \"{}\", port: {}, isServer: {} )", host_, port_, isServer_ );

  if( isServer_ ) {
    zmqSocket_.bind( fmt::format( "{}:{}", host_, port_ ) );
  } else {
    zmqSocket_.connect( fmt::format( "{}:{}", host_, port_ ) );
  }

  logger_->trace( "ReqRepClient()~" );
}

ReqRepClient::~ReqRepClient() {
  logger_->trace( "~ReqRepClient()" );
  for( int i = 0; i < subscribedMessages_.size(); i++ ) {
    delete subscribedMessages_[i];
  }
  zmqSocket_.close();
  zmqContext_.shutdown();
  logger_->trace( "~ReqRepClient()~" );
}

void ReqRepClient::subscribe( google::protobuf::Message* message, std::function< void( google::protobuf::Message const& ) > callback ) {
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

void ReqRepClient::sendMessage( google::protobuf::Message* message ) {
  logger_->trace( "sendMessage( message: {} (\"{}\") )", static_cast< void* >( message ), message->GetTypeName() );

  mutexForSendQueue_.lock();
  queueToSend_.push( message );
  mutexForSendQueue_.unlock();

  logger_->trace( "sendMessage()~" );
}

void ReqRepClient::run() {
  // logger_->trace( "run()" );

  if( sending_ ) {
    if( queueToSend_.empty() ) {
      // logger_->trace( "run()~" );
      return;
    }
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
      sending_ = false;
    } else {
      // logger_->warn( "No message sent!" );
    }
    mutexForSendQueue_.unlock();
  }

  if( !sending_ ) {
    zmq::message_t receivedReply;
    SFG::Proto::Wrapper receivedWrapper;
    zmq::recv_result_t recvResult = zmqSocket_.recv( receivedReply, zmq::recv_flags::dontwait );
    if( recvResult ) {
      receivedWrapper.ParseFromString( receivedReply.to_string() );
      for( int i = 0; i < subscribedMessages_.size(); i++ ) {
        if( subscribedMessages_[i]->GetTypeName() != receivedWrapper.protoname() ) {
          continue;
        }
        subscribedMessages_[i]->ParseFromString( receivedWrapper.protocontent() );
        subscribedCallbacks_[i]( *( subscribedMessages_[i] ) );
        sending_ = true;
        break;
      }
    } else {
      // logger_->warn( "No message received!" );
    }
  }

  // logger_->trace( "run()~" );
}

}  // namespace Networking
}  // namespace SFG
