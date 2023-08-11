#include "main.hpp"

#include <csignal>
#include <fmt/ranges.h>
#include <iostream>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <vector>

#include "client.hpp"
#include "server.hpp"

namespace SFG {

std::function< void( int32_t ) > signalCallback = nullptr;
void signalHandler( int signal ) {
  if( signalCallback ) {
    signalCallback( signal );
  }
}

[[nodiscard]] int better_main( [[maybe_unused]] std::span< std::string_view const > args ) noexcept {
  if( ( args.size() != 2 ) || ( ( args[1] != "client" ) && ( args[1] != "server" ) ) ) {
    // only allow for `program client` or `program server`
    std::cerr << "call program like \"" << args[0] << " client\" or \"" << args[0] << " server\"" << std::endl;
    return EXIT_FAILURE;
  }
  InitializeLoggers( std::string( args[1] ) );
  spdlog::trace( "better_main( args: \"{}\" )", fmt::join( args, "\", \"" ) );

  signal( SIGINT, signalHandler );
  signal( SIGILL, signalHandler );
  signal( SIGFPE, signalHandler );
  signal( SIGSEGV, signalHandler );
  signal( SIGTERM, signalHandler );
  signal( SIGBREAK, signalHandler );
  signal( SIGABRT, signalHandler );

  bool isClient = args[1] == "client";

  if( isClient ) {
    spdlog::trace( "Constructing Client" );
    Client *myClient = new Client( "127.0.0.1", 13337 );

    signalCallback = [myClient]( int32_t signal ) {
      spdlog::trace( "signalCallback( signal: {} )", signal );
      myClient->stopClient();
      myClient->waitForClient();
      delete myClient;
      spdlog::trace( "signalCallback()~" );
    };

    spdlog::trace( "Calling Client::startClient" );
    myClient->startClient();
    spdlog::trace( "Called Client::startClient" );

    std::string readLine;
    while( 1 ) {
      while( myClient->isWaitingForReply() ) {
      }
      std::cout << "Write message: ";
      std::getline( std::cin, readLine, '\n' );
      if( readLine == "" ) {
        myClient->stopClient();
        break;
      }
      myClient->sendMessage( readLine );
    }

    spdlog::trace( "Waiting for Client to stop" );
    myClient->waitForClient();

    spdlog::trace( "Cleaning up Client" );
    delete myClient;
  } else {
    spdlog::trace( "Constructing Server" );
    Server *myServer = new Server( 13337 );

    signalCallback = [myServer]( int32_t signal ) {
      spdlog::trace( "signalCallback( signal: {} )", signal );
      myServer->stopServer();
      myServer->waitForServer();
      delete myServer;
      spdlog::trace( "signalCallback()~" );
    };

    spdlog::trace( "Calling Server::startServer" );
    myServer->startServer();
    spdlog::trace( "Called Server::startServer" );

    spdlog::trace( "Waiting for Server to stop" );
    myServer->waitForServer();

    spdlog::trace( "Cleaning up Server" );
    delete myServer;
  }

  spdlog::trace( "better_main()~" );
  return EXIT_SUCCESS;
}

void InitializeLoggers( std::string filePostfix ) noexcept {
  std::vector< std::string > allLoggerNames = { "ReqRepClient", "ReqRepServer", "Client", "Server" };

  auto consoleSink = std::make_shared< spdlog::sinks::stdout_color_sink_mt >();
  consoleSink->set_level( spdlog::level::level_enum::debug );

  auto truncatedFileSink = std::make_shared< spdlog::sinks::basic_file_sink_mt >( fmt::format( "log_{}.log", filePostfix ), true );
  truncatedFileSink->set_level( spdlog::level::level_enum::trace );
  spdlog::sinks_init_list truncatedSinkList = { truncatedFileSink, consoleSink };

  std::shared_ptr< spdlog::logger > mainLogger = std::make_shared< spdlog::logger >( "main", truncatedSinkList.begin(), truncatedSinkList.end() );
  mainLogger->set_level( spdlog::level::level_enum::trace );
  mainLogger->flush_on( spdlog::level::level_enum::trace );
  spdlog::register_logger( mainLogger );
  spdlog::set_default_logger( mainLogger );
  mainLogger->trace( "Contructed main logger" );

  mainLogger->trace( "Contructing other loggers" );
  auto normalFileSink = std::make_shared< spdlog::sinks::basic_file_sink_mt >( fmt::format( "log_{}.log", filePostfix ), false );
  normalFileSink->set_level( spdlog::level::level_enum::trace );
  spdlog::sinks_init_list normalSinkList = { normalFileSink, consoleSink };

  for( auto name : allLoggerNames ) {
    std::shared_ptr< spdlog::logger > logger = std::make_shared< spdlog::logger >( name, normalSinkList.begin(), normalSinkList.end() );
    logger->set_level( spdlog::level::level_enum::trace );
    logger->flush_on( spdlog::level::level_enum::trace );
    spdlog::register_logger( logger );
  }
  mainLogger->trace( "All loggers constructed" );
}

}  // namespace SFG

int32_t main( int32_t const argc, char const *const *argv ) {
  std::vector< std::string_view > args( argv, std::next( argv, static_cast< std::ptrdiff_t >( argc ) ) );
  return SFG::better_main( args );
}
