#include "server.hpp"

namespace SFG {

Server::Server( uint16_t port )
    : logger_( spdlog::get( "Server" ) ), network_( "tcp://*", port, true ), thread_( nullptr ), loop_( false ), stopThread_( nullptr ) {
  logger_->trace( "Server()" );
  network_.subscribe( new SFG::Proto::MessageRequest(), [this]( google::protobuf::Message const& message ) {
    this->onMessageRequest( static_cast< SFG::Proto::MessageRequest const& >( message ) );
  } );
  network_.subscribe( new SFG::Proto::StopRequest(),
                      [this]( google::protobuf::Message const& message ) { this->onStopRequest( static_cast< SFG::Proto::StopRequest const& >( message ) ); } );
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
  if( stopThread_ ) {
    logger_->trace( "~Server - deleting stopThread" );
    if( stopThread_->joinable() ) {
      stopThread_->join();
    }
    delete stopThread_;
    stopThread_ = nullptr;
  }
  logger_->trace( "~Server()~" );
}

void Server::startServer() {
  logger_->trace( "startServer()" );

  loop_ = true;

  thread_ = new std::thread( [this]() {
    this->logger_->trace( "thread_()" );
    while( this->loop_ ) {
      std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
      this->network_.run();
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
  repMsg->set_success( true );
  network_.sendMessage( repMsg );

  logger_->trace( "onMessageRequest()~" );
}

void Server::onStopRequest( sfpb::StopRequest const& reqMsg ) {
  logger_->trace( "onStopRequest()" );

  SFG::Proto::StopResponse* repMsg = new SFG::Proto::StopResponse();
  network_.sendMessage( repMsg );
  stopThread_ = new std::thread( [this]() {
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    this->stopServer();
  } );

  logger_->trace( "onStopRequest()~" );
}

}  // namespace SFG
