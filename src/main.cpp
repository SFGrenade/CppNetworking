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

namespace {
// trim from start (in place)
static inline void ltrim( std::string &s ) {
  s.erase( s.begin(), std::find_if( s.begin(), s.end(), []( unsigned char ch ) { return !std::isspace( ch ); } ) );
}

// trim from end (in place)
static inline void rtrim( std::string &s ) {
  s.erase( std::find_if( s.rbegin(), s.rend(), []( unsigned char ch ) { return !std::isspace( ch ); } ).base(), s.end() );
}

// trim from both ends (in place)
static inline void trim( std::string &s ) {
  rtrim( s );
  ltrim( s );
}
}  // namespace

namespace SFG {

std::function< void( int32_t ) > signalCallback = nullptr;
void signalHandler( int sigNum ) {
  signal( sigNum, signalHandler );
  if( signalCallback ) {
    signalCallback( sigNum );
  }
}

#if __cplusplus >= 202002L
#define STRING_CONTAINS( str1, str2 ) ( str1 ).contains( ( str2 ) )
#else
#define STRING_CONTAINS( str1, str2 ) ( str1 ).find( ( str2 ) ) != std::string::npos
#endif

#if __cplusplus >= 202002L
[[nodiscard]] int better_main( [[maybe_unused]] std::span< std::string_view > args ) noexcept {
#else
[[nodiscard]] int better_main( [[maybe_unused]] std::vector< std::string_view > args ) noexcept {
#endif
  if( ( args.size() != 2 ) || ( ( STRING_CONTAINS( args[1], "client" ) ) && ( STRING_CONTAINS( args[1], "server" ) ) ) ) {
    // only allow for `program client` or `program server`
    std::cerr << "call program like \"" << args[0] << " client\" or \"" << args[0] << " server\"" << std::endl;
    return EXIT_FAILURE;
  }
  InitializeLoggers( std::string( args[1] ) );
  spdlog::trace( "better_main( args: \"{}\" )", fmt::join( args, "\", \"" ) );

  InitializeSignalHandler();

  bool isClient = STRING_CONTAINS( args[1], "client" );

  if( isClient ) {
    spdlog::trace( "Constructing Client" );
    Client *myClient = new Client( "tcp://127.0.0.1", 13337, "tcp://127.0.0.1", 13338 );

    signalCallback = [myClient]( int32_t signal ) {
      spdlog::trace( "signalCallback( signal: {} )", signal );
      myClient->stopClient();
      // delete myClient;
      spdlog::trace( "signalCallback()~" );
    };

    spdlog::trace( "Calling Client::startClient" );
    myClient->startClient();
    spdlog::trace( "Called Client::startClient" );

    std::string readLine;
    while( myClient->isRunning() ) {
      std::cout << "Write message: ";
      std::getline( std::cin, readLine, '\n' );
      trim( readLine );
      if( readLine == "" ) {
        continue;
      }
      if( readLine == "quit" || readLine == "stop" ) {
        myClient->stopClient();
        break;
      }
      myClient->sendMessage( readLine );
      while( myClient->isWaitingForReply() ) {
      }
    }

    spdlog::trace( "Waiting for Client to stop" );
    myClient->waitForClient();

    spdlog::trace( "Cleaning up Client" );
    delete myClient;
  } else {
    spdlog::trace( "Constructing Server" );
    Server *myServer = new Server( 13337, 13338 );

    signalCallback = [myServer]( int32_t signal ) {
      spdlog::trace( "signalCallback( signal: {} )", signal );
      myServer->stopServer();
      // delete myServer;
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
  std::vector< std::string > allLoggerNames = { "PubSub", "ReqRep", "ZmqWrap", "Client", "Server" };

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

void InitializeSignalHandler() noexcept {
  spdlog::trace( "InitializeSignalHandler()" );

  std::set_terminate( []() {
    spdlog::trace( "terminateCallback()" );
    spdlog::trace( "terminateCallback()~" );
  } );
  for( int i = 0; i <= NSIG; i++ ) {
    auto retCode = signal( i, signalHandler );
    spdlog::debug( "Installing handler for {}: {}", i, SIG_ERR != retCode );
  }

  spdlog::trace( "InitializeSignalHandler()~" );
}

}  // namespace SFG

int32_t main( int32_t const argc, char const *const *argv ) {
  std::vector< std::string_view > args( argv, std::next( argv, static_cast< std::ptrdiff_t >( argc ) ) );
  return SFG::better_main( args );
}
