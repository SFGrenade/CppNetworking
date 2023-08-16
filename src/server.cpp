#include "server.hpp"

namespace SFG {

Server::Server( uint16_t portReqRep, uint16_t portPubSub )
    : logger_( spdlog::get( "Server" ) ),
      rrNetwork_( "tcp://*", portReqRep, true ),
      psNetwork_( "tcp://*", portPubSub, true ),
      thread_( nullptr ),
      loop_( false ) {
  logger_->trace( "Server()" );
  rrNetwork_.subscribe( new SFG::Proto::MessageRequest(), [this]( google::protobuf::Message const& message ) {
    this->onMessageRequest( static_cast< SFG::Proto::MessageRequest const& >( message ) );
  } );
  logger_->trace( "Server()~" );
}

Server::~Server() {
  logger_->trace( "~Server()" );
  if( thread_ ) {
    logger_->trace( "~Server - deleting thread" );
    if( thread_->joinable() ) {
      thread_->join();
    }
    delete thread_;
    thread_ = nullptr;
  }
  logger_->trace( "~Server()~" );
}

void Server::startServer() {
  logger_->trace( "startServer()" );

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

  logger_->trace( "startServer()~" );
}

void Server::waitForServer() {
  logger_->trace( "waitForServer()" );

  if( thread_ ) {
    if( thread_->joinable() ) {
      thread_->join();
    }
  }

  logger_->trace( "waitForServer()~" );
}

void Server::stopServer() {
  logger_->trace( "stopServer()" );

  loop_ = false;

  logger_->trace( "stopServer()~" );
}

void Server::onMessageRequest( SFG::Proto::MessageRequest const& reqMsg ) {
  logger_->trace( "onMessageRequest( reqMsg: \"{}\" )", reqMsg.message() );

  SFG::Proto::MessageResponse* repMsg = new SFG::Proto::MessageResponse();
  if( reqMsg.message().empty() ) {
    repMsg->set_success( false );
  } else {
    messages_.push_back( reqMsg.message() );
    repMsg->set_success( true );
  }
  rrNetwork_.sendMessage( repMsg );

  SFG::Proto::AllMessages* pubMsg = new SFG::Proto::AllMessages();
  for( auto message : messages_ ) {
    pubMsg->add_messages( message );
  }
  psNetwork_.sendMessage( pubMsg );

  logger_->trace( "onMessageRequest()~" );
}

}  // namespace SFG
