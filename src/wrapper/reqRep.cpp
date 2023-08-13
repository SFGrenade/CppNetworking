#include "wrapper/reqRep.hpp"

#include "main.pb.h"

namespace SFG {
namespace Networking {

ReqRep::ReqRep( std::string const& host, uint16_t port, bool isServer )
    : ZmqWrap( host, port, isServer ? ZmqWrap::Status::Receiving : ZmqWrap::Status::Sending, isServer ? zmq::socket_type::rep : zmq::socket_type::req ),
      logger_( spdlog::get( "ReqRep" ) ),
      isServer_( isServer ) {
  logger_->trace( "ReqRep( host: \"{}\", port: {}, isServer: {} )", host_, port_, isServer_ );

  if( isServer_ ) {
    zmqSocket_.bind( fmt::format( "{}:{}", host_, port_ ) );
  } else {
    zmqSocket_.connect( fmt::format( "{}:{}", host_, port_ ) );
  }

  logger_->trace( "ReqRep()~" );
}

ReqRep::~ReqRep() {
  logger_->trace( "~ReqRep()" );

  logger_->trace( "~ReqRep()~" );
}

void ReqRep::run() {
  // logger_->trace( "run()" );

  if( status() == ZmqWrap::Status::Sending ) {
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
      setStatus( ZmqWrap::Status::Receiving );
    } else {
      // logger_->warn( "No message sent!" );
    }
    mutexForSendQueue_.unlock();
  }

  if( status() == ZmqWrap::Status::Receiving ) {
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
        setStatus( ZmqWrap::Status::Sending );
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
