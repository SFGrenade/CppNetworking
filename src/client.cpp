#include "client.hpp"

namespace SFG {

Client::Client( std::string const& host, uint16_t port )
    : logger_( spdlog::get( "Client" ) ), client_( host, port ), thread_( nullptr ), loop_( false ), waitingForReply_( false ) {
  logger_->trace( "Client()" );
  client_.subscribe( new SFG::Proto::SimpleResponse(), [this]( google::protobuf::Message const& message ) {
    this->onSimpleResponse( static_cast< SFG::Proto::SimpleResponse const& >( message ) );
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
      this->client_.run();
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

  SFG::Proto::SimpleRequest* msg = new SFG::Proto::SimpleRequest();
  msg->set_message( message );
  client_.sendMessage( msg );

  waitingForReply_ = true;

  logger_->trace( "sendMessage()~" );
}

bool Client::isWaitingForReply() const {
  // logger_->trace( "isWaitingForReply()" );

  // logger_->trace( "isWaitingForReply()~" );
  return waitingForReply_;
}

void Client::onSimpleResponse( SFG::Proto::SimpleResponse const& repMsg ) {
  logger_->trace( "onSimpleResponse( repMsg: \"{}\" )", repMsg.message() );

  std::cout << "[Server] " << repMsg.message() << std::endl;

  waitingForReply_ = false;

  logger_->trace( "onSimpleResponse()~" );
}

}  // namespace SFG
