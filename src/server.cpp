#include "server.hpp"

namespace SFG {

Server::Server( uint16_t port ) : logger_( spdlog::get( "Server" ) ), server_( "*", port ), thread_( nullptr ), loop_( false ) {
  logger_->trace( "Server()" );
  server_.subscribe( new SFG::Proto::SimpleRequest(), [this]( google::protobuf::Message const& message ) {
    this->onSimpleRequest( static_cast< SFG::Proto::SimpleRequest const& >( message ) );
  } );
  logger_->trace( "Server()~" );
}

Server::~Server() {
  logger_->trace( "~Server()" );
  if( thread_ ) {
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
      this->server_.run();
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

void Server::onSimpleRequest( SFG::Proto::SimpleRequest const& reqMsg ) {
  logger_->trace( "onSimpleRequest( reqMsg: \"{}\" )", reqMsg.message() );

  SFG::Proto::SimpleResponse* repMsg = new SFG::Proto::SimpleResponse();
  repMsg->set_message( reqMsg.message() + "World" );
  server_.sendMessage( repMsg );

  logger_->trace( "onSimpleRequest()~" );
}

}  // namespace SFG
