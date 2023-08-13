#include "client.hpp"

namespace SFG {

Client::Client( std::string const& host, uint16_t port )
    : logger_( spdlog::get( "Client" ) ), network_( host, port, false ), thread_( nullptr ), loop_( false ) {
  logger_->trace( "Client()" );
  network_.subscribe( new SFG::Proto::MessageResponse(), [this]( google::protobuf::Message const& message ) {
    this->onMessageResponse( static_cast< SFG::Proto::MessageResponse const& >( message ) );
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
      this->network_.run();
      std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
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
  network_.sendMessage( msg );

  logger_->trace( "sendMessage()~" );
}

bool Client::isWaitingForReply() const {
  // logger_->trace( "isWaitingForReply()" );

  // logger_->trace( "isWaitingForReply()~" );
  return ( network_.status() == SFG::Networking::ReqRep::Status::Receiving ) && isRunning();
}

bool Client::isRunning() const {
  // logger_->trace( "isRunning()" );

  // logger_->trace( "isRunning()~" );
  return thread_ != nullptr && loop_;
}

void Client::onMessageResponse( SFG::Proto::MessageResponse const& repMsg ) {
  logger_->trace( "onMessageResponse( success: \"{}\" )", repMsg.success() );

  logger_->trace( "onMessageResponse()~" );
}

}  // namespace SFG
