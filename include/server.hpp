#pragma once

#include <functional>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>

#include "main.pb.h"
#include "reqRepServer.hpp"

namespace SFG {

class Server {
  public:
  Server( uint16_t port );
  ~Server();

  void startServer();
  void waitForServer();
  void stopServer();

  private:
  void onSimpleRequest( SFG::Proto::SimpleRequest const& reqMsg );

  private:
  std::shared_ptr< spdlog::logger > logger_;

  ZmqPbWrap::ReqRepServer server_;
  std::thread* thread_;
  bool loop_;
};

}  // namespace SFG
