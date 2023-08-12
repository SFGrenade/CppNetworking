#include "wrapper/reqRepServer.hpp"

namespace SFG {
namespace Networking {

ReqRepServer::ReqRepServer( std::string const& host, uint16_t port )
    : logger_( spdlog::get( "ReqRepServer" ) ),
      host_( host ),
      port_( port ),
      zmqContext_( 1, 1 ),
      zmqSocket_( zmqContext_, zmq::socket_type::rep ),
      queueToSend_() {
  logger_->trace( "ReqRepServer( host: \"{}\", port: {} )", host_, port_ );

  zmqSocket_.bind( fmt::format( "{}:{}", host_, port_ ) );

  logger_->trace( "ReqRepServer()~" );
}

ReqRepServer::~ReqRepServer() {
  logger_->trace( "~ReqRepServer()" );

  for( int i = 0; i < subscribedMessages_.size(); i++ ) {
    delete subscribedMessages_[i];
  }
  zmqSocket_.close();
  zmqContext_.shutdown();

  logger_->trace( "~ReqRepServer()~" );
}

void ReqRepServer::subscribe( google::protobuf::Message* message, std::function< void( google::protobuf::Message const& ) > callback ) {
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

void ReqRepServer::sendMessage( google::protobuf::Message* message ) {
  logger_->trace( "sendMessage( message: {} (\"{}\") )", static_cast< void* >( message ), message->GetTypeName() );

  mutexForSendQueue_.lock();
  queueToSend_.push( message );
  mutexForSendQueue_.unlock();

  logger_->trace( "sendMessage()~" );
}

void ReqRepServer::run() {
  // logger_->trace( "run()" );

  zmq::message_t receivedReply;
  zmq::recv_result_t recvResult = zmqSocket_.recv( receivedReply, zmq::recv_flags::none );
  if( recvResult ) {
    for( int i = 0; i < subscribedMessages_.size(); i++ ) {
      if( subscribedMessages_[i]->ParseFromString( receivedReply.to_string() ) ) {
        subscribedCallbacks_[i]( *( subscribedMessages_[i] ) );
      }
    }
  }

  while( queueToSend_.empty() ) {
    // wait until queue has content
  }
  mutexForSendQueue_.lock();
  google::protobuf::Message* msgToSend = queueToSend_.front();
  zmq::send_result_t sendResult = zmqSocket_.send( zmq::buffer( msgToSend->SerializeAsString() ), zmq::send_flags::none );
  queueToSend_.pop();
  delete msgToSend;
  mutexForSendQueue_.unlock();

  // logger_->trace( "run()~" );
}

}  // namespace Networking
}  // namespace SFG
