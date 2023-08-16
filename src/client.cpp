#include "client.hpp"

#include <fmt/ranges.h>

namespace SFG {

Client::Client( std::string const& hostReqRep, uint16_t portReqRep, std::string const& hostPubSub, uint16_t portPubSub )
    : logger_( spdlog::get( "Client" ) ),
      rrNetwork_( hostReqRep, portReqRep, false ),
      psNetwork_( hostPubSub, portPubSub, false ),
      thread_( nullptr ),
      loop_( false ),
      waitingForMessageResponse_( false ) {
  logger_->trace( "Client()" );
  rrNetwork_.subscribe( new SFG::Proto::MessageResponse(), [this]( google::protobuf::Message const& message ) {
    this->onMessageResponse( static_cast< SFG::Proto::MessageResponse const& >( message ) );
  } );
  psNetwork_.subscribe( new SFG::Proto::AllMessages(), [this]( google::protobuf::Message const& message ) {
    this->onAllMessages( static_cast< SFG::Proto::AllMessages const& >( message ) );
  } );
  logger_->trace( "Client()~" );
}

Client::~Client() {
  logger_->trace( "~Client()" );
  if( thread_ ) {
    if( thread_->joinable() ) {
      thread_->join();
    }
    delete thread_;
    thread_ = nullptr;
  }
  logger_->trace( "~Client()~" );
}

void Client::startClient() {
  logger_->trace( "startClient()" );

  loop_ = true;

  thread_ = new std::thread( [this]() {
    this->logger_->trace( "thread_()" );
    while( this->loop_ ) {
      this->rrNetwork_.run();
      this->psNetwork_.run();
      std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    }
    this->logger_->trace( "thread_()~" );
  } );

  logger_->trace( "startClient()~" );
}

void Client::waitForClient() {
  logger_->trace( "waitForClient()" );

  if( thread_ ) {
    if( thread_->joinable() ) {
      thread_->join();
    }
  }

  logger_->trace( "waitForClient()~" );
}

void Client::stopClient() {
  logger_->trace( "stopClient()" );

  loop_ = false;

  logger_->trace( "stopClient()~" );
}

void Client::sendMessage( std::string const& message ) {
  logger_->trace( "sendMessage( message: \"{}\" )", message );

  SFG::Proto::MessageRequest* msg = new SFG::Proto::MessageRequest();
  msg->set_message( message );
  rrNetwork_.sendMessage( msg );
  waitingForMessageResponse_ = true;

  logger_->trace( "sendMessage()~" );
}

bool Client::isWaitingForReply() const {
  // logger_->trace( "isWaitingForReply()" );

  // logger_->trace( "isWaitingForReply()~" );
  return waitingForMessageResponse_ && isRunning();
}

bool Client::isRunning() const {
  // logger_->trace( "isRunning()" );

  // logger_->trace( "isRunning()~" );
  return thread_ != nullptr && loop_;
}

void Client::onMessageResponse( SFG::Proto::MessageResponse const& repMsg ) {
  logger_->trace( "onMessageResponse( success: \"{}\" )", repMsg.success() );

  waitingForMessageResponse_ = false;

  logger_->trace( "onMessageResponse()~" );
}

void Client::onAllMessages( SFG::Proto::AllMessages const& repMsg ) {
  logger_->trace( "onAllMessages( [{} messages] )", repMsg.messages_size() );

  logger_->debug( "Messages: [\"{}\"]", fmt::join( repMsg.messages(), "\", \"" ) );

  logger_->trace( "onAllMessages()~" );
}

}  // namespace SFG
