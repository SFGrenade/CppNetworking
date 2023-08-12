#include "wrapper/reqRepClient.hpp"

namespace SFG {
namespace Networking {

ReqRepClient::ReqRepClient( std::string const& host, uint16_t port )
    : logger_( spdlog::get( "ReqRepClient" ) ),
      host_( host ),
      port_( port ),
      zmqContext_( 1, 1 ),
      zmqSocket_( zmqContext_, zmq::socket_type::req ),
      queueToSend_() {
  logger_->trace( "ReqRepClient( host: \"{}\", port: {} )", host_, port_ );

  zmqSocket_.connect( fmt::format( "{}:{}", host_, port_ ) );

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

  if( queueToSend_.empty() ) {
    // logger_->trace( "run()~" );
    return;
  }

  mutexForSendQueue_.lock();
  google::protobuf::Message* msgToSend = queueToSend_.front();
  zmq::send_result_t sendResult = zmqSocket_.send( zmq::buffer( msgToSend->SerializeAsString() ), zmq::send_flags::none );
  queueToSend_.pop();
  delete msgToSend;
  mutexForSendQueue_.unlock();

  zmq::message_t receivedReply;
  zmq::recv_result_t recvResult = zmqSocket_.recv( receivedReply, zmq::recv_flags::none );
  if( recvResult ) {
    for( int i = 0; i < subscribedMessages_.size(); i++ ) {
      if( subscribedMessages_[i]->ParseFromString( receivedReply.to_string() ) ) {
        subscribedCallbacks_[i]( *( subscribedMessages_[i] ) );
      }
    }
  }

  // logger_->trace( "run()~" );
}

}  // namespace Networking
}  // namespace SFG
